/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <tuple>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;
static unordered_map<unsigned long long, string> m1, m2;
static unordered_map<string ,unsigned long long> m11, m22;

class CirMgr
{
   enum CirMgrFlag { NO_FEC = 0x1 };
   enum ParsePorts { VARS = 0, PI, LATCH, PO, AIG, TOT_PARSE_PORTS };

public:
   CirMgr(): _flag(0), _piList(0), _poList(0), _totGateList(0),
             _fanoutInfo(0), _simLog(0),_numCEX(0) { }
   ~CirMgr() { deleteCircuit(); }

   // Access functions
   CirGate* operator [](unsigned gid) const { return _totGateList[gid]; }
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const {
      if (gid >= getNumTots()) { return 0; } return _totGateList[gid]; }
   CirGate* litId2Gate(unsigned litId) const { return getGate(litId/2); }
   CirGateV litId2GateV(unsigned litId) const;
   bool isFlag(CirMgrFlag f) const { return _flag & f; }
   void setFlag(CirMgrFlag f) const { _flag |= f; }
   void unsetFlag(CirMgrFlag f) const { _flag &= ~f; }
   void resetFlag() const { _flag = 0; }
   unsigned getNumPIs() const { return _numDecl[PI]; }
   unsigned getNumPOs() const { return _numDecl[PO]; }
   unsigned getNumTots() const { return _numDecl[VARS] + _numDecl[PO] + 1; }
   CirPiGate* getPi(unsigned i) const { return _piList[i]; }
   CirPoGate* getPo(unsigned i) const { return _poList[i]; }
   GateList& getFanouts(unsigned i) const { return _fanoutInfo[i]; }

   // Member functions about circuit construction
   bool readCircuit(const string&);
   void deleteCircuit(bool _deleteConst = 1);
   void genConnections();
   void genDfsList();
//   void updateUndefList();
   void checkFloatList();
   void checkUnusedList();
   size_t checkConnectedGate(size_t);

   // Member functions about circuit optimization
  void sweep();
  void optimize();
//   bool checkAigOptimize(CirGate*, const CirGateV&, const CirGateV&,
//                         CirGateV&) const;
//   void deleteAigGate(CirGate *);
//   void deleteUndefGate(CirGate *);


   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void selfSim(bool);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
  void strash();
   IdList* getFECGrps(size_t i) const { return _fecGrps[i]; }
  void printFEC() const;
  void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

   // Member functions about flags

   // for hidden command
   void equivalaceChecking(CirMgr* ); // should be called after cirwrite the original circuit to tmp/miter.aag
//   bool createMiter(CirMgr*, CirMgr*);

   static CirGate *_const0;
   GateList& getDfsList() { return _dfsList; }
private:
   unsigned            _numDecl[TOT_PARSE_PORTS];
   size_t              _numCEX; // number of counter examples
   mutable unsigned    _flag;
   PiArray             _piList;
   PoArray             _poList;
   // IDs in _undefList are NOT sorted!!
   IdList              _undefList;
   // Make sure the IDs of the following lists are sorted!!
   IdList              _floatList;  // gates with fanin(s) undefined
   IdList              _unusedList; // gates defined but not used
   GateArray           _totGateList;
   GateList            _dfsList;
   GateList           *_fanoutInfo;
   vector<IdList*>     _fecGrps;  // store litId; FECHash<GatePValue, IdList*>
//   SimVector           _fecVector;
   ofstream           *_simLog;
   vector<tuple<string, string, int>> comp;
   // unordered_map<int ,pair<string, string>> comp;

   // private member functions for circuit parsing
   bool parseHeader(ifstream&);
   bool parseInput(ifstream&);
   bool parseLatch(ifstream&);
   bool parseOutput(ifstream&);
   bool parseAig(ifstream&);
   bool parseSymbol(ifstream&);
   bool parseComment(ifstream&);
   bool checkId(unsigned&, const string&);
   CirGate* checkGate(unsigned&, ParsePorts, const string&);

   // private member functions for circuit optimization
   CirGateV constSimplify(CirGate *, const CirGateV&, const CirGateV&) const;
   int count1(unsigned long long x) {
   int a=0;
      while(x!=0) {
         x>>=1;
         if(x & 1)
            a++;
      }
      return a;
      } 
   int count0(unsigned long long x) {
   int a=0;
      while(x!=0) {
         x>>=1;
         if(!(x & 0))
            a++;
      }
      return a;
      } 
   
   // private member functions about simulation
//   void setRandPPattern() const;
//   void setPPattern(unsigned long long *const patterns) const;
//   unsigned gatherPatterns(ifstream&, unsigned *const, size_t);
//   void pSim1Pattern() const;
//   void outputSimLog(size_t nPatterns = 32);
   void initSim();
   // private member functions about FRAIG
   bool simAndCheckFEC();
   bool initFEC();
   bool checkFEC();
   size_t fecGrpSize() const { return _fecGrps.size(); }
   unsigned fecId2Id(unsigned fecId) const { return fecId >> 1; }
   bool     fecIdIsInv(unsigned fecId) const { return fecId & 1; }
   unsigned Id2fecId(unsigned id, bool inv) const { return (id << 1) + inv; }
   CirGate* fecId2Gate(unsigned fecId) const { return getGate(fecId2Id(fecId)); }
//   bool checkFECRecur(const IdList&, vector<IdList*>&);
    void finalizeFEC();   // sort each fecGrp
    void reorderConstFEC(IdList* ); // move const 0 to the front (while keeping it positive) and negate the corresponding FecId
//   void simplifyFECGrps();
   void clearFECGrps();
   void CollectValidFecGrp(unordered_map<unsigned long long, IdList*>& , IdList*);
   void initProofModel(SatSolver&, const GateList&);
   bool checkConstFECGrp(SatSolver&, IdList* ); //check const fec group // return 1 -> should break
   bool checkFECGrp(SatSolver&, IdList*); // check normal fec group // return 1 -> should break
   bool satCheckConst(SatSolver&, CirGate *, bool);   // check if same as const0 // return 0 if same
   // bool satCheckConst(SatSolver&, CirGate *, bool, unsigned *const);
   bool satCheckFEC(SatSolver&, CirGate *, bool, CirGate *, bool);
   void updateFECbySatPattern();
//    bool satCheckFEC(SatSolver&, CirGate *, CirGate *, bool, unsigned *const);
//   void getSatAssignment(SatSolver&, unsigned *const) const;
//   void simplifyByEQ();
//   void updateFECbySatPattern(unsigned *const);
};

#endif // CIR_MGR_H
