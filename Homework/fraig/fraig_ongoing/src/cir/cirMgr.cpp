/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;
CirGate* CirMgr::_const0 = new CirConstGate(0);

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine constant (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

static void
getErrorToken(char *tmp)
{
   assert(!isspace(*tmp));
   char *end = tmp;
   while ((*(++end) != 0) && !isspace(*end));
   *end = 0;
   errMsg = tmp;
}

// Read the number until space character is encountered.
// If any non-digit or non-space char is encountered, return false.
// In the end, colNo will point to the non-digit character.
static bool
getUint(unsigned& num, const string& numType)
{
   if (buf[colNo] == 0) {
      errMsg = numType; return parseError(MISSING_NUM); }
   assert(!isspace(buf[colNo]));
   char *tmp = buf + colNo;
   num = 0;
   do {
      if (!isdigit(buf[colNo])) {
         getErrorToken(tmp);
         errMsg = numType + "(" + errMsg + ")";
         return parseError(ILLEGAL_NUM);
      }
      num *= 10;
      num += unsigned(buf[colNo] - '0');
      if (buf[++colNo] == 0 || isspace(buf[colNo])) break;
   } while (!isspace(buf[colNo]));
   return true;
}

// colNo will be updated!!
// Check if tmp contains identifier.
// However, does not check if tmp contains extra char.
static bool checkIdentifier(const char* identifier)
{
   char *tmp = buf + colNo;
   if (*tmp == 0) {
      errMsg = identifier; return parseError(MISSING_IDENTIFIER);
   }
   for (int i = 0, len = strlen(identifier); i < len; ++i, ++colNo)
      if (buf[colNo] == 0 || buf[colNo] != identifier[i]) {
         getErrorToken(tmp); return parseError(ILLEGAL_IDENTIFIER); }
   return true;
}

// If leadSpace == true, the first char is a space. <== colNo will ++
// The followed char cannot be any white space char.
// Make sure buf[colNo] != 0 (after the leading WS)
static bool checkWS(bool leadSpace)
{
   if (leadSpace) {
      if (buf[colNo] == 0 || buf[colNo] != ' ')
         return parseError(MISSING_SPACE);
      ++colNo;
   }
   if (buf[colNo] == ' ') return parseError(EXTRA_SPACE);
   if (isspace(buf[colNo])) {
      errInt = int(buf[colNo]); return parseError(ILLEGAL_WSPACE); }
   return true;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/

void CirMgr::selfSim(bool is1){
   // ofstream fout("sim_case03_1.out");
   vector<string> s;
   unordered_map<string, int> m;

   for(size_t j = 0; j < _numDecl[PI]; ++j){
      for(size_t i = 0; i < _numDecl[PI]; i++){
         // _piList[i][0].setPValue(CirPValue(~0ULL - (1ULL << i)));
         if(i == j)
            _piList[i][0].setPValue(CirPValue(~0ULL));
         else
            _piList[i][0].setPValue(CirPValue(1ULL << i));
      }
      for(auto i : _dfsList){
         i->pSim();
      }
      for(size_t i = 0; i < _numDecl[PO]; ++i){
         if(count1(_poList[i][0].getPValue()()) > 5){
            if(is1){
               m1[(_poList[i][0].getPValue()())] = _poList[i][0].getName();
               m11[(_poList[i][0].getName())] = _poList[i][0].getPValue()();
            }
               // m1[(_poList[i][0].getPValue()() ^ 2199023255551ULL)] = _poList[i][0].getName();
            else{
               m2[_poList[i][0].getPValue()()] = _poList[i][0].getName();
               m22[_poList[i][0].getName()] = _poList[i][0].getPValue()();
               // m2[(_poList[i][0].getPValue()() ^ 2199023255551ULL)] = _poList[i][0].getName();
               // m2[(_poList[i][0].getPValue()() ^ 2199023255551ULL)] = _poList[i][0].getName();
            }
            s.push_back(_poList[i][0].getName());
            if(m.find(_poList[i][0].getName()) != m.end()){
               ++m[_poList[i][0].getName()];
            }
            else{
               m.insert(pair<string, int>(_poList[i][0].getName(), 1));
            }
         }
      }
      cout << "input ( "<< _numDecl[PI] << " ) = \n";
      // fout << "input = \n";
      for(size_t i = 0; i < _numDecl[PI]; ++i){
         cout <<  setw(4) << _piList[i][0].getName() << " : " << _piList[i][0].getPValue() << endl;
         // fout <<  setw(4) << _piList[i][0].getName() << " : " << _piList[i][0].getPValue() << endl;
      }
      cout << "output ( "<< _numDecl[PO] << " ) = \n";
      // fout << "output = \n";
      for(size_t i = 0; i < _numDecl[PO]; ++i){
         cout <<  setw(4) << _poList[i][0].getName() << " : " << _poList[i][0].getPValue() << endl;
         // fout <<  setw(4) << _poList[i][0].getName() << " : " << _poList[i][0].getPValue() << endl;
         // fout <<  _poList[i][0].getPValue() << endl;
      }

   }
      return;
   if(!m1.empty() && !m2.empty()){
      for(auto i : m1){
         int curmax = -1;
         bool pos = 1;
         string name2 = "";
         string name1 = i.second;
         for(auto j : m2){
            if(i.first & j.first == i.first)
               comp.push_back(std::tuple<string, string, int>(name1, j.second, 1));
         }
      }
   }
   // if(!m1.empty() && !m2.empty()){
   //    for(auto i : m1){
   //       int curmax = -1;
   //       bool pos = 1;
   //       string name2 = "";
   //       string name1 = i.second;
   //       for(auto j : m2){
   //          if(count1(i.first & j.first) >= curmax )
   //          {
   //             cout << i.second << " ; " << j.second << endl;
   //             curmax = count1(i.first & j.first);
   //             name2 = j.second;
   //             pos = 1;
   //          }
   //          else if(count1(i.first & (j.first ^ 2199023255551ULL)) >= curmax ){
   //             cout << i.second << " ; " << j.second << endl;
   //             curmax = count1(i.first & (j.first ^ 2199023255551ULL));
   //             name2 = j.second;
   //             pos = 0;
   //          }
   //       }
   //       comp.push_back(std::tuple<string, string, int>(name1, name2, pos ? curmax : -curmax));
   //    }
   // }
   cout << "m[104] = \n";
   cout << bitset<64>(m22["n145"]) << endl;
   return;
}
bool
CirMgr::readCircuit(const string& fileName)
{
   ifstream cirin(fileName.c_str());
   if (!cirin) {
      cerr << "Cannot open design \"" << fileName << "\"!!" << endl;
      return false;
   }
   lineNo = 0; colNo = 0;
   if (!parseHeader(cirin)) return false;
   if (!parseInput(cirin)) return false;
   if (!parseLatch(cirin)) return false;
   if (!parseOutput(cirin)) return false;
   if (!parseAig(cirin)) return false;
   if (!parseSymbol(cirin)) return false;
   if (!parseComment(cirin)) return false;
   genConnections();

   genDfsList();
   checkFloatList();
   checkUnusedList();
   // selfSim((fileName[11] == '1'));

   return true;
}

void
CirMgr::deleteCircuit(bool _deleteConst)
{
   if (_totGateList)
      for (unsigned i = 1, n = getNumTots(); i < n; ++i)
         if (_totGateList[i]) delete _totGateList[i];
   if (_piList) { delete []_piList; _piList = 0; }
   if (_poList) { delete []_poList; _poList = 0; }
   if (_totGateList) { delete []_totGateList; _totGateList = 0; }
   if (_fanoutInfo) { delete []_fanoutInfo; _fanoutInfo = 0; }
   clearList<IdList>(_undefList);
   clearList<IdList>(_floatList);
   clearList<IdList>(_unusedList);
   clearList<GateList>(_dfsList);
   if(_deleteConst){
      delete _const0;
      _const0 = new CirConstGate(0);
   }
/*
   if (!_fecGrps.empty())
      for (unsigned i = 0, n = _fecGrps.size(); i < n; ++i)
         delete _fecGrps[i];
   clearList<vector<IdList*> >(_fecGrps);
   for (size_t i = 0, np = _fecVector.size(); i < np; ++i)
      delete []_fecVector[i];
   clearList<SimVector>(_fecVector);
   delete _simLog;
   resetFlag();
*/
}

bool
CirMgr::parseHeader(ifstream& cirin)
{
   assert (lineNo == 0 && colNo == 0);
   cirin.getline(buf, 1024);

   // aag M I L O A
   const int aagLength = 3;
   if (!checkWS(false)) return false;
   if (!checkIdentifier("aag")) return false;

   string portStr[TOT_PARSE_PORTS] = { "number of variables", "number of PIs",
                  "number of latches", "number of POs", "number of AIGs" };
   // Need to check 0; otherwise getErrorToken(buf) will seg fault
   if (buf[aagLength] == 0) {
      errMsg = portStr[VARS]; return parseError(MISSING_NUM); }
   // Take care "aagkk", but not "aag3", nor "aag<tab>"
   if (!isspace(buf[aagLength]) && !isdigit(buf[aagLength])) {
      getErrorToken(buf); return parseError(ILLEGAL_IDENTIFIER); }

   colNo = aagLength;
   for (int i = 0; i < TOT_PARSE_PORTS; ++i) {
      if (buf[colNo] == 0) {
         errMsg = portStr[i]; return parseError(MISSING_NUM); }
      if (!checkWS(true)) return false;
      if (!getUint(_numDecl[i], portStr[i])) return false;
   }
   if (buf[colNo] != 0) return parseError(MISSING_NEWLINE);

   // Check numbers
   // M must >= I + L + A
   if (_numDecl[VARS] < _numDecl[PI] + _numDecl[LATCH] + _numDecl[AIG]) {
      errMsg = "Number of variables"; errInt = _numDecl[VARS];
      return parseError(NUM_TOO_SMALL);
   }
   // L must = 0
   if (_numDecl[LATCH] != 0) {
      errMsg = "latches"; return parseError(ILLEGAL_NUM);
   }

   // Create lists
   _piList = new CirPiGate*[_numDecl[PI]];
   _poList = new CirPoGate*[_numDecl[PO]];
   // +1 for const
   unsigned numTots = getNumTots();
   _totGateList = new CirGate*[numTots];
   for (unsigned i = 0; i < numTots; ++i)
      _totGateList[i] = 0;
   _fanoutInfo = new GateList[numTots];

   _totGateList[0] = _const0;

   return true;
}

bool CirMgr::parseInput(ifstream& cirin)
{
   for (size_t i = 0, n = _numDecl[PI]; i < n; ++i) {
      ++lineNo; colNo = 0;
      cirin.getline(buf, 1024);
      if (cirin.eof()) { errMsg = "PI"; return parseError(MISSING_DEF); }
      if (!checkWS(false)) return false;
      unsigned litId;
      CirGate *gate = checkGate(litId, PI, "PI");
      if (!gate) return false;
      _piList[i] = static_cast<CirPiGate*>(gate);
      _totGateList[litId/2] = gate;
      if (buf[colNo] != 0) return parseError(MISSING_NEWLINE);
   }

   return true;
}

bool CirMgr::parseLatch(ifstream& cirin) { return true; }

bool CirMgr::parseOutput(ifstream& cirin)
{
   size_t poId = _numDecl[VARS] + 1;
   for (size_t i = 0,  nPo = _numDecl[PO]; i < nPo; ++i, ++poId) {
      ++lineNo; colNo = 0;
      cirin.getline(buf, 1024);
      if (cirin.eof()) { errMsg = "PO"; return parseError(MISSING_DEF); }
      if (!checkWS(false)) return false;
      unsigned litId;
      if (!checkId(litId, "PO")) return false;
      CirGate *gate = new CirPoGate(poId, lineNo+1, litId);
      _poList[i] = static_cast<CirPoGate*>(gate);
      _totGateList[poId] = gate;
      if (buf[colNo] != 0) return parseError(MISSING_NEWLINE);
   }

   return true;
}

bool CirMgr::parseAig(ifstream& cirin)
{
   for (size_t i = 0, n = _numDecl[AIG]; i < n; ++i) {
      ++lineNo; colNo = 0;
      cirin.getline(buf, 1024);
      if (cirin.eof()) { errMsg = "AIG"; return parseError(MISSING_DEF); }
      if (!checkWS(false)) return false;
      unsigned litId;
      CirGate *gate = checkGate(litId, AIG, "AIG gate");
      if (!gate) return false;
      _totGateList[litId/2] = gate;
      unsigned in[2];
      for (int j = 0; j < 2; ++j) {
         if (!checkWS(true)) return false;
         if (!checkId(in[j], "AIG input")) return false;
      }
      if (buf[colNo] != 0) return parseError(MISSING_NEWLINE);
      // set _in0, _in1 as litId for now. Convert to gateId when connected
      static_cast<CirAigGate*>(gate)->setIn0(in[0]);
      static_cast<CirAigGate*>(gate)->setIn1(in[1]);
   }
   return true;
}

bool CirMgr::parseSymbol(ifstream& cirin)
{
   while (1) {
      // Read in next line!!
      ++lineNo; colNo = 0;
      cirin.getline(buf, 1024);
      if (cirin.eof()) return true;
      char type = buf[colNo];
      if (type == 'i' || type == 'l' || type == 'o') {
         ++colNo;
         if (!checkWS(false)) return false;
         unsigned portId;
         if (!getUint(portId, "symbol index")) return false;
         if (buf[colNo] == 0) {
            errMsg = "symbolic name"; return parseError(MISSING_IDENTIFIER); }
         if (buf[colNo] == ' ') {
            if (buf[++colNo] == 0) {
               errMsg = "symbolic name";
               return parseError(MISSING_IDENTIFIER);
            }
         }
         else 
            return parseError(MISSING_SPACE);
         char *str = buf + colNo;
         for (; buf[colNo] != 0; ++colNo) {
            if (!isprint(buf[colNo])) {
               errInt = int(buf[colNo]);
               return parseError(ILLEGAL_SYMBOL_NAME);
            }
         }
         if (type == 'i') {
            if (portId >= _numDecl[PI]) {
               errMsg = "PI index"; errInt = portId;
               return parseError(NUM_TOO_BIG);
            }
            CirPiGate *pi = _piList[portId];
            if (pi->getName()) {
               errMsg = "i"; errInt = portId;
               return parseError(REDEF_SYMBOLIC_NAME);
            }
            char *n = new char[strlen(str)+1]; strcpy(n, str);
            pi->setName(n);
         }
         // else if (type == 'l')... NOT supported yet
         else { // if (type == 'o')
            assert(type == 'o');
            if (portId >= _numDecl[PO]) {
               errMsg = "PO index"; errInt = portId;
               return parseError(NUM_TOO_BIG);
            }
            CirPoGate *po = _poList[portId];
            if (po->getName()) {
               errMsg = "o"; errInt = portId;
               return parseError(REDEF_SYMBOLIC_NAME);
            }
            char *n = new char[strlen(str)+1]; strcpy(n, str);
            po->setName(n);
         }
      }
      else if (type == 'c') return true;
      else if (type == ' ') return parseError(EXTRA_SPACE);
      else if (isspace(type)) {
         errInt = int(type); return parseError(ILLEGAL_WSPACE); }
      else {
         errMsg.resize(1); errMsg[0] = type;
         return parseError(ILLEGAL_SYMBOL_TYPE);
      }
   }
   return true;
}

bool CirMgr::parseComment(ifstream& cirin) 
{
   // The first comment line should have been read
   if (cirin.eof()) return true;
   char type = buf[colNo];
   assert(type == 'c');
   if (buf[++colNo] != 0) return parseError(MISSING_NEWLINE);

   return true; 
}

bool
CirMgr::checkId(unsigned& litId, const string& err)
{
   unsigned oldColNo = colNo;
   if (!getUint(litId, err + " literal ID")) return false;
   if (litId/2 > _numDecl[VARS]) {
      errInt = litId; colNo = oldColNo; return parseError(MAX_LIT_ID);
   }
   return true;
}

// For PI and AIG gates only!!
CirGate*
CirMgr::checkGate(unsigned& litId, ParsePorts type, const string& err)
{
   unsigned oldColNo = colNo;
   if (!checkId(litId, err)) return 0;
   if (litId < 2) { errInt = litId; colNo = oldColNo;
      parseError(REDEF_CONST); return 0; }
   if (litId % 2 != 0) {
      errMsg = err; errInt = litId; colNo = oldColNo; 
      parseError(CANNOT_INVERTED); return 0;
   }
   CirGate *gate = _totGateList[litId/2];
   if (gate != 0) {
      errInt = litId; errGate = gate; colNo = oldColNo; 
      parseError(REDEF_GATE); return 0;
   }
   switch (type) {
      case PI: gate = new CirPiGate(litId/2, lineNo+1); break;
      case AIG: gate = new CirAigGate(litId/2, lineNo+1); break;
      default: cerr << "Error: Unknown gate type (" << type << ")!!\n";
               exit(-1);
   }
   return gate;
}

// v is the litId to connect
// Create an CirUndefGate if getGate() return 0
size_t
CirMgr::checkConnectedGate(size_t v)
{
   size_t ret;
   CirGate *g = getGate(v/2);
   if (!g) {
      g = new CirUndefGate(v/2);
      _totGateList[v/2] = g;
      _undefList.push_back(v/2);
   }
   ret = size_t(g);
   if (g->isUndef()) ret += CirGateV::AIG_FLT_FANIN;
   ret += (v & CirGateV::AIG_NEG_FANIN);
   return ret;
}

void
CirMgr::genConnections()
{
   for (unsigned i = 0, n = getNumTots(); i < n; ++i) {
      CirGate *gate = _totGateList[i];
      if (gate) gate->genConnections();
   }
}

void
CirPoGate::genConnections()
{
   _in0 = cirMgr->checkConnectedGate(_in0());
   cirMgr->getFanouts(_in0.gateId()).push_back(this);
}

void
CirAigGate::genConnections()
{
   _in0 = cirMgr->checkConnectedGate(_in0());
   cirMgr->getFanouts(_in0.gateId()).push_back(this);
   _in1 = cirMgr->checkConnectedGate(_in1());
   cirMgr->getFanouts(_in1.gateId()).push_back(this);
}

void
CirMgr::genDfsList()
{
   clearList(_dfsList);
   CirGate::setGlobalRef();
   for (unsigned i = 0, n = getNumPOs(); i < n; ++i)
      getPo(i)->genDfsList(_dfsList);
}

void
CirPiGate::genDfsList(GateList& gateList)
{
   setToGlobalRef();
   gateList.push_back(this);
}

void
CirPoGate::genDfsList(GateList& gateList)
{
   setToGlobalRef();
   CirGate* g = _in0.gate();
   if (!g->isGlobalRef())
      g->genDfsList(gateList);
   gateList.push_back(this);
}

void
CirAigGate::genDfsList(GateList& gateList)
{
   if(getEqGate() != 0){
      getEqGate().gate()->genDfsList(gateList);
      return;
   }
   setToGlobalRef();
   CirGate* g = _in0.gate();
   if (!g->isGlobalRef())
      g->genDfsList(gateList);
   g = _in1.gate();
   if (!g->isGlobalRef())
      g->genDfsList(gateList);
   gateList.push_back(this);
}

void
CirConstGate::genDfsList(GateList& gateList)
{
   setToGlobalRef();
   gateList.push_back(this);
}

/*
// Remove the entries that getGate() = NULL
void
CirMgr::updateUndefList()
{
   size_t i = 0;
   for (size_t j = 0, n = _undefList.size(); j< n; ++j) {
      if (!getGate(_undefList[j])) continue; // has been deleted
      if (_fanoutInfo[_undefList[j]].empty()) {
         deleteUndefGate(getGate(_undefList[j]));
         continue;
      }
      if (i != j) _undefList[i] = _undefList[j];
      ++i;
   }
   if (i == 0) clearList<IdList>(_undefList);
   else _undefList.resize(i);
}
*/

void
CirMgr::checkFloatList()
{
   clearList(_floatList);
   CirGate::setGlobalRef();
   for (size_t i = 0, n = _undefList.size(); i< n; ++i) {
      const GateList& fanouts = _fanoutInfo[_undefList[i]];
      for (size_t j = 0, m = fanouts.size(); j < m; ++j)
         if (!fanouts[j]->isGlobalRef()) {
            fanouts[j]->setToGlobalRef();
            _floatList.push_back(fanouts[j]->getGid());
         }
   }
   sort(_floatList.begin(), _floatList.end());
}

void
CirMgr::checkUnusedList()
{
   clearList(_unusedList);
   for (size_t i = 1, n = getNumTots(); i < n; ++i)
      if (_fanoutInfo[i].empty() && _totGateList[i])
//         if (!_totGateList[i]->isPo() && !_totGateList[i]->isPi())
         if (!_totGateList[i]->isPo())
            _unusedList.push_back(i);
}

CirGateV
CirMgr::litId2GateV(unsigned litId) const 
{
   return size_t(litId2Gate(litId)) + litId%2;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   cout << endl;
   cout << "Circuit Statistics" << endl
        << "==================" << endl;
   unsigned tot = 0;
   tot += _numDecl[PI];
   cout << "  " << setw(7) << left << "PI"
        << setw(7) << right << _numDecl[PI] << endl;
   tot += _numDecl[PO];
   cout << "  " << setw(7) << left << "PO"
        << setw(7) << right << _numDecl[PO] << endl;
   tot += _numDecl[AIG];
   cout << "  " << setw(7) << left << "AIG"
        << setw(7) << right << _numDecl[AIG] << endl;
   cout << "------------------" << endl;
   cout << "  Total  " << setw(7) << right << tot << endl;
}

void
CirMgr::printNetlist() const
{
   cout << endl;
   for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      _dfsList[i]->printGate();
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for (unsigned i = 0, n = getNumPIs(); i < n; ++i)
      cout << " " << getPi(i)->getGid();
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (unsigned i = 0, n = getNumPOs(); i < n; ++i)
      cout << " " << getPo(i)->getGid();
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   size_t fn = _floatList.size();
   if (fn) {
      cout << "Gates with floating fanin(s):";
      for (size_t i = 0; i < fn; ++i)
         cout << " " << _floatList[i];
      cout << endl;
   }
   size_t un = _unusedList.size();
   if (un) {
      cout << "Gates defined but not used  :";
      for (size_t i = 0; i < un; ++i)
         cout << " " << _unusedList[i];
      cout << endl;
   }
}


void
CirMgr::printFECPairs() const
{
   // int i = 0;
   // for(auto grp : _fecGrps){
   //    cout << "[" << i++ << "] ";
   //    for(auto j : *grp){
   //       cout << j << " ";
   //    }
   //    cout << endl;
   // }
   struct ArrIdx {
      unsigned _litId;
      size_t _idx;
      ArrIdx(unsigned l, size_t i): _litId(l), _idx(i) {}
      bool operator < (const ArrIdx& i) const { return (_litId < i._litId); }
   };
   vector<ArrIdx> fecOrder;
   for (size_t i = 0, n = _fecGrps.size(); i < n; ++i)
      fecOrder.push_back(ArrIdx((_fecGrps[i])->operator[](0), i));
   ::sort(fecOrder.begin(), fecOrder.end());
   for (size_t i = 0, n = fecOrder.size(); i < n; ++i) {
      size_t idx = fecOrder[i]._idx;
      IdList *ll = _fecGrps[idx];
      cout << "[" << i << "]";
      for (size_t j = 0, m = ll->size(); j < m; ++j)
         cout << " " << litId2GateV((*ll)[j]);
      cout << endl;
   }
}


void
CirMgr::writeAag(ostream& outfile) const
{
   size_t nAig = 0;
   for (size_t i = 0, n = _dfsList.size(); i < n; ++i)
      if (_dfsList[i]->isAig()) ++nAig;
   outfile << "aag " << _numDecl[VARS] << " " << _numDecl[PI] << " "
           << _numDecl[LATCH] << " " << _numDecl[PO] << " "
           << nAig << endl;
   for (size_t i = 0, n = _numDecl[PI]; i < n; ++i)
      outfile << (getPi(i)->getGid()*2) << endl;
   for (size_t i = 0, n = _numDecl[PO]; i < n; ++i)
      outfile << getPo(i)->getIn0().litId() << endl;
   for (size_t i = 0, n = _dfsList.size(); i < n; ++i) {
      CirGate *g = _dfsList[i];
      if (!g->isAig()) continue;
      outfile << g->getGid()*2 << " " << g->getIn0().litId() << " "
           << g->getIn1().litId() << endl;
   }
   for (size_t i = 0, n = _numDecl[PI]; i < n; ++i)
      if (getPi(i)->getName())
         outfile << "i" << i << " " << getPi(i)->getName() << endl;
   for (size_t i = 0, n = _numDecl[PO]; i < n; ++i)
      if (getPo(i)->getName())
         outfile << "o" << i << " " << getPo(i)->getName() << endl;
   outfile << "c" << endl;
   outfile << "AAG output by Chung-Yang (Ric) Huang" << endl;
}


void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
   cout << "writeGate\n";
   GateList faninCone;
   CirGate::setGlobalRef();
   g->genDfsList(faninCone);

   size_t nAig = 0, npi = 0;
   unsigned maxId = 0;
   for (size_t i = 0, n = faninCone.size(); i < n; ++i) {
      if (faninCone[i]->isAig()) ++nAig;
      else if (faninCone[i]->isPi()) ++npi;
      if (faninCone[i]->getGid() > maxId) maxId = faninCone[i]->getGid();
   }
   outfile << "aag " << maxId << " " << npi << " 0 1 " << nAig << endl;
   for (size_t i = 0, n = _numDecl[PI]; i < n; ++i)
      if (getPi(i)->isGlobalRef())
         outfile << (getPi(i)->getGid()*2) << endl;
   outfile << g->getGid()*2 << endl;
   for (size_t i = 0, n = faninCone.size(); i < n; ++i) {
      CirGate *f = faninCone[i];
      if (!f->isAig()) continue;
      outfile << f->getGid()*2 << " " << f->getIn0().litId() << " "
           << f->getIn1().litId() << endl;
   }
   for (size_t i = 0, n = _numDecl[PI], j = 0; i < n; ++i)
      if (getPi(i)->isGlobalRef() && getPi(i)->getName())
         outfile << "i" << j++ << " " << getPi(i)->getName() << endl;
   outfile << "o0 " << g->getGid() << endl;
   outfile << "c" << endl;
   outfile << "Write gate (" << g->getGid()
           << ") by Chung-Yang (Ric) Huang" << endl;
}


/**********************************************************/
/*   class CirMgr member functions for hidden commands    */
/**********************************************************/
// After this command, cm1 and cm2 will be deleted!!
/*
bool
CirMgr::createMiter(CirMgr *cm1, CirMgr *cm2)
{
   ofstream outfile("/tmp/miter.aag");
   size_t npi   = cm1->_numDecl[PI];
   size_t npo   = cm1->_numDecl[PO];
   size_t nVarO = cm1->_numDecl[VARS] + cm2->_numDecl[VARS] + 1;
   size_t nVar  = nVarO + 3*npo;
   size_t nAig1 = 0, nAig2 = 0;
   for (size_t i = 0, n = cm1->_dfsList.size(); i < n; ++i)
      if (cm1->_dfsList[i]->isAig()) ++nAig1;
   for (size_t i = 0, n = cm2->_dfsList.size(); i < n; ++i)
      if (cm2->_dfsList[i]->isAig()) ++nAig2;
   size_t nAig = nAig1 + nAig2 + 3*npo;
   size_t vOffset = cm1->_numDecl[VARS] + 1;
   outfile << "aag " << nVar << " " << npi << " 0 " << npo << " "
           << nAig << endl;
   for (size_t i = 0; i < npi; ++i)
      outfile << (cm1->getPi(i)->getGid()*2) << endl;
   for (size_t i = 0; i < npo; ++i)
      outfile << (nVarO+3+i*3)*2 << endl;
   for (size_t i = 0, n = cm1->_dfsList.size(); i < n; ++i) {
      CirGate *g = cm1->_dfsList[i];
      if (!g->isAig()) continue;
      outfile << g->getGid()*2 << " " << g->getIn0().litId() << " "
              << g->getIn1().litId() << endl;
   }
   for (size_t i = 0, n = cm2->_dfsList.size(); i < n; ++i) {
      CirGate *g = cm2->_dfsList[i];
      if (!g->isAig()) continue;
      outfile << (g->getGid() + vOffset)*2;
      if (g->getIn0Gate()->isAig())
         outfile << " " << (g->getIn0().litId() + vOffset*2) << " ";
      else
         outfile << " " << g->getIn0().litId() << " ";
      if (g->getIn1Gate()->isAig())
         outfile << (g->getIn1().litId() + vOffset*2) << endl;
      else
         outfile << g->getIn1().litId() << endl;
   }
   for (size_t i = 0, j = nVarO + 1; i < npo; ++i, j += 3) {
      unsigned po1 = cm1->getPo(i)->getIn0().litId();
      unsigned po2 = cm2->getPo(i)->getIn0Gate()->isAig()?
                     cm2->getPo(i)->getIn0().litId() + vOffset * 2:
                     cm2->getPo(i)->getIn0().litId();
      outfile << j*2 << " " << po1 << " " << po2 << endl;
      outfile << (j+1)*2 << " " << (po1^0x1) << " " << (po2^0x1) << endl;
      outfile << (j+2)*2 << " " << j*2+1 << " " << (j+1)*2+1 << endl;
   }
   for (size_t i = 0; i < npi; ++i)
      if (cm1->getPi(i)->getName())
         outfile << "i" << i << " " << cm1->getPi(i)->getName() << endl;
   for (size_t i = 0; i < npo; ++i)
      if (cm1->getPo(i)->getName())
         outfile << "o" << i << " " << cm1->getPo(i)->getName() << endl;
   outfile << "c" << endl;
   outfile << "Miter output by Chung-Yang (Ric) Huang" << endl;

   outfile.close();

   deleteCircuit();
   return readCircuit("/tmp/miter.aag");
}
*/

void CirMgr::equivalaceChecking(CirMgr* cm1){

   SatSolver* solver = new SatSolver;
   solver->initialize();
   unordered_set<unsigned> _dfsPO; // po in dfs list
   _const0->setMiterVar(solver->newVar());
   solver->assertProperty(_const0->getMiterVar(), false);
   for(auto _gate : _dfsList){
      _gate->setMiterVar(solver->newVar());
      if(_gate->isAig()){
         solver->addAigCNF(_gate->getMiterVar(), _gate->getIn0().gate()->getMiterVar(), _gate->getIn0().isInv(), _gate->getIn1().gate()->getMiterVar(), _gate->getIn1().isInv()); 
      }
      else if(_gate->isPo()){
         _dfsPO.insert(_gate->getGid());
        solver->addEqCNF(_gate->getMiterVar(), 0,_gate->getIn0().gate()->getMiterVar(), _gate->getIn0().isInv());
      }
   }
   for(size_t i = 0, ni = getNumPOs(); i < ni; ++i){
      // if(_dfsPO.find(getPo(i)->getGid()) == _dfsPO.end())
      //    solver->assumeProperty(getPo(i)->getMiterVar(), 0);
      // else
        solver->assumeProperty(getPo(i)->getMiterVar(), true);
   }
   bool result = solver->assumpSolve();
   if(result){
      cout << "non-equivalent" << endl;
   }
   else{
      cout << "equivalent" << endl;
   }
   delete solver;
   return;
   /*
   cm1->genDfsList();
   vector<Var> miters;
   unordered_set<unsigned> _dfsPO, _dfsPI; // pi, po in dfs list
   _const0->setMiterVar(solver->newVar());
   solver->assertProperty(_const0->getMiterVar(), false);
   // add miter var to this cirmgr's gate
   for(size_t i = 0, ni = _numDecl[PI]; i < ni; ++i){
      getPi(i)->setMiterVar(solver->newVar());
      cm1->getPi(i)->setMiterVar(solver->newVar());
   }
   for(auto _gate : _dfsList){
      if(_gate->isPi() || _gate->isConst()){
         continue;
      }
      _gate->setMiterVar(solver->newVar()); 
      if(_gate->isAig()){
         solver->addAigCNF(_gate->getMiterVar(), _gate->getIn0().gate()->getMiterVar(), _gate->getIn0().isInv(), _gate->getIn1().gate()->getMiterVar(), _gate->getIn1().isInv()); 
      }
      else if(_gate->isPo()){
         _dfsPO.insert(_gate->getGid());
        solver->addEqCNF(_gate->getMiterVar(), 0,_gate->getIn0().gate()->getMiterVar(), _gate->getIn0().isInv());
      }
   }
   // add miter var to cm1's gate
   for(auto _gate : cm1->getDfsList()){
      if(_gate->isPi() || _gate->isConst()){
         continue;
      }
      _gate->setMiterVar(solver->newVar());
      if(_gate->isAig()){
         solver->addAigCNF(_gate->getMiterVar(), _gate->getIn0().gate()->getMiterVar(), _gate->getIn0().isInv(), _gate->getIn1().gate()->getMiterVar(), _gate->getIn1().isInv()); 
      }
      else if(_gate->isPo()){
        solver->addEqCNF(_gate->getMiterVar(), 0,_gate->getIn0().gate()->getMiterVar(), _gate->getIn0().isInv());
      }
   }
   // two circuits' PI should be the same
   for(size_t i = 0, ni = _numDecl[PI]; i < ni; ++i){
      solver->addEqCNF(getPi(i)->getMiterVar(), 0, cm1->getPi(i)->getMiterVar(), 0);
   }
   // add miter to the output pair
   // only po that are not float in the new circuit (current circuit) should be considered
   for(size_t i = 0, ni = _numDecl[PO]; i < ni; ++i){
      if(_dfsPO.find(getPo(i)->getGid()) == _dfsPO.end())
         continue;
      miters.push_back(solver->newVar());
      solver->addXorCNF(miters.back(), getPo(i)->getMiterVar(), 0, cm1->getPo(i)->getMiterVar(), 0);
   }
   // those miters should be 1
   for(auto i : miters)
        solver->assumeProperty(i, true);
   bool result = solver->assumpSolve();
   if(result){
      cout << "non-equivalent" << endl;
   }
   else{
      cout << "equivalent" << endl;
   }
   cm1->deleteCircuit(0);
   delete cm1;
   delete solver;
   CirGate::decreaseGlobalRef();
   return;
   */
}