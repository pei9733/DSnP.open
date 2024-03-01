/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"
#include <unordered_map>
#include <utility>
#include  <algorithm>


using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
class strashKey{
  public:
    strashKey(size_t i0, size_t i1): in_0(i0), in_1(i1){}
    struct hashFunc{
      size_t operator()(const strashKey& k) const{
        return k.in_0 ^ k.in_1;
        // size_t key = ((k.in_0 >> 1) << 1) * 1749;
        // key ^= (k.in_0 & 1) * 3905;
        // key ^= ((k.in_1 >> 1) << 1) * 19037;
        // key ^= (k.in_1 & 1) * 7911;
        // return key;
      }
    };
    bool operator == (const strashKey& k) const{
        return (in_0 == k.in_0 && in_1 == k.in_1) || (in_0 == k.in_1 && in_1 == k.in_0);
    }
    size_t in_0, in_1;

};


/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
    unordered_map<strashKey, CirGate*, strashKey::hashFunc> struchash;
    for(auto i : _dfsList){
      if(i->getType() != AIG_GATE) continue;
      strashKey key(i->getIn0()(), i->getIn1()());
      if(struchash.find(key) == struchash.end()){
        assert(struchash.insert(make_pair(key, i)).second);
      }
      else{
        cout << "Strashing: " << struchash[key]->getGid() << " merging " << i->getGid() << "...\n";
        struchash[key]->merge(i);
        delete _totGateList[i->getGid()];
        --_numDecl[AIG];
      }
    }
    genDfsList();
    checkFloatList();
}

void
CirMgr::fraig()
{
    SatSolver* solver = new SatSolver;
    initProofModel(*solver, _dfsList);
    time_t start = clock();
    while(!_fecGrps.empty()){
      for(auto fecGrp : _fecGrps){
        if(fecGrp->at(0) == _const0->getGid()){
            if(checkConstFECGrp(*solver, fecGrp))
                break;
        }
        else{
            if(checkFECGrp(*solver, fecGrp))
                break;
        }
       }
       updateFECbySatPattern();
    //    if(clock() - start > 5000)
    //         break;
    }
    genDfsList();
    delete solver;
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
bool CirMgr::initFEC(){
  clearFECGrps();
  _fecGrps.push_back(new IdList);
  for(auto i : _dfsList){
    if(i->getType() == AIG_GATE){
      _fecGrps.back()->push_back(Id2fecId(i->getGid(), 0));
    }
  }
  _fecGrps.back()->push_back(_const0->getGid());
}
void CirMgr::clearFECGrps(){
  _fecGrps.clear();
}
void CirMgr::CollectValidFecGrp(unordered_map<unsigned long long, IdList*>& newFecGrps, IdList* fecGrp){

}
void CirMgr::finalizeFEC(){
  for(auto fecGrp : _fecGrps){
    ::sort(fecGrp->begin(), fecGrp->end());
    if(fecGrp->front() % 2){
      for(auto& i : *fecGrp){
        i ^= 1; // negate the last bit
      }
    }
  }

}
void CirMgr::reorderConstFEC(IdList* fecGrp){
    for(size_t i = 0, ni = fecGrp->size(); i < ni; ++i){
      if(fecId2Id(fecGrp->at(i)) == _const0->getGid()){
          int tmp = fecGrp->at(i);
          fecGrp->at(i) = fecGrp->front();
          fecGrp->front() = tmp;
          if(fecGrp->front() % 2){
            for(auto& i : *fecGrp){
              i ^= 1;
            }
          }
      }

    }
}

bool CirMgr::checkFEC(){
  vector<unordered_map<unsigned long long, IdList*>*> fecGrps;
//   IdList* constGrp = 0;
  for(auto fecGrp : _fecGrps){
    unordered_map<unsigned long long, IdList*>* newFecGrps = new unordered_map<unsigned long long, IdList*>;
    for(auto gate : *fecGrp){
      if(gate == (unsigned)-1) 
        continue;
      gate = fecId2Id(gate);
      unordered_map<unsigned long long, IdList*>::iterator grp = newFecGrps->find(getGate(gate)->getPValue()());
      bool inv = false;
      if(grp == newFecGrps->end()){
        grp = newFecGrps->find(~getGate(gate)->getPValue()());
        inv = true;
      }
      if(grp != newFecGrps->end()){
        if(fecId2Gate(grp->second->at(0))->isNonEqGate(getGate(gate)))
            continue;
        grp->second->push_back(Id2fecId(gate, inv));
        // if(gate == _const0->getGid()){
        //   constGrp = grp->second;
        // }
      }
      else{
        IdList* newGrp = new IdList({Id2fecId(gate, 0)});
        newFecGrps->insert(make_pair(getGate(gate)->getPValue()(), newGrp));
        // if(gate == _const0->getGid()){
        //   constGrp = newGrp;
        // }
      }
    }
    fecGrps.push_back(newFecGrps);
    // CollectValidFecGrp(newFecGrps, fecGrp);
  }
  for(IdList* fecGrp : _fecGrps){
    delete fecGrp;
  }
  _fecGrps.clear();
//   if(constGrp){
//     reorderConstFEC(constGrp);
//   }
  vector<IdList*>::iterator it = _fecGrps.begin(); 
  for(auto fecGrp : fecGrps){
    for(auto grp : *fecGrp){
      if(grp.second->size() > 1){
        _fecGrps.push_back(grp.second);
      }
      else{
        delete grp.second;
      }
    }
    delete fecGrp;
  }
  return 0;
}

bool CirMgr::simAndCheckFEC(){
    for(auto i : _dfsList){
        i->pSim();
    }
    checkFEC();
}

void CirMgr::updateFECbySatPattern(){
    cout << "resimulating counter examples..." << endl;
    genDfsList();
    simAndCheckFEC();
    finalizeFEC();
    _numCEX = 0;
}

bool CirMgr::checkConstFECGrp(SatSolver& solver, IdList* fecGrp){
  for(size_t i = 1, ni = fecGrp->size(); i < ni; ++i){
    if(fecGrp->at(i) == (unsigned)-1 || getGate(fecId2Id(fecGrp->at(i)))->isNonEqGate(_const0))
        continue;
    if(!satCheckFEC(solver, fecId2Gate(fecGrp->at(i)), fecIdIsInv(fecGrp->at(i)), _const0, 0)){
      cout << "Fraig: " << 0 << " merging " << (fecIdIsInv(fecGrp->at(i)) ? "!" : "") << fecId2Id(fecGrp->at(i)) << "..." << endl;
      _const0->merge(getGate(fecId2Id(fecGrp->at(i))));
      delete _totGateList[fecId2Id(fecGrp->at(i))];
      fecGrp->at(i) = (unsigned)-1;
      --_numDecl[AIG];
    }
    if(_numCEX == 32){
        updateFECbySatPattern();
        return 1;
    }
  }
    return 0;
}

bool CirMgr::checkFECGrp(SatSolver& solver, IdList* fecGrp){
    for(size_t i = 0, ni = fecGrp->size(); i < ni; ++i){
        if(fecGrp->at(i) == (unsigned)-1)
            continue;
        for(size_t j = i + 1, nj = ni; j < nj; ++j){
            if(fecGrp->at(j) == (unsigned)-1 || fecId2Gate(fecGrp->at(i))->isNonEqGate(fecId2Gate(fecGrp->at(j))))
                continue;
            if(!satCheckFEC(solver, fecId2Gate(fecGrp->at(i)), fecIdIsInv(fecGrp->at(i)), fecId2Gate(fecGrp->at(j)), fecIdIsInv(fecGrp->at(j)))){
                unsigned& fecId1 = fecGrp->at(i) < fecGrp->at(j) ? fecGrp->at(i) : fecGrp->at(j);
                unsigned& fecId2 = fecGrp->at(i) < fecGrp->at(j) ? fecGrp->at(j) : fecGrp->at(i);
                cout << "Fraig: " << fecId2Id(fecId1) << " merging " << (fecIdIsInv(fecId1) ? "!" : "") << fecId2Id(fecId2) << "..." << endl;
                fecId2Gate(fecId1)->merge(getGate(fecId2Id(fecId2)));
                delete _totGateList[fecId2Id(fecId2)];
                fecId2 = (unsigned)-1;
                --_numDecl[AIG];
            }
            if(_numCEX == 32){
                updateFECbySatPattern();
                return 1;
            }
        }
    }
    return 0;   
}
/********************************************/
/*   Private member functions about sat     */
/********************************************/
void CirMgr::initProofModel(SatSolver& solver, const GateList& gateList){
    solver.initialize();
    _const0->setSatVar(solver.newVar());
    solver.assertProperty(_const0->getSatVar(), 0);
    for(auto i : gateList){
        i->setSatVar(solver.newVar());
        if(i->getType() == AIG_GATE)
            solver.addAigCNF(i->getSatVar(), i->getIn0().gate()->getSatVar(), i->getIn0().isInv(), i->getIn1().gate()->getSatVar(), i->getIn1().isInv()); 
    }
}

bool CirMgr::satCheckFEC(SatSolver& solver, CirGate * _gate1, bool _isInv1, CirGate * _gate2, bool _isInv2){
    Var miter = solver.newVar();
    solver.assumeRelease();
    solver.addXorCNF(miter, _gate1->getSatVar(), _isInv1, _gate2->getSatVar(), _isInv2);
    solver.assumeProperty(miter, 1);
    bool result = solver.assumpSolve(); // result = 0 -> equivalent
    if(result){ // if non-equivalent -> collect the pattern
        _gate1->addNonEqGate(_gate2);
        _gate2->addNonEqGate(_gate1);
        for(int i = 0, ni = getNumPIs(); i < ni; ++i){
            CirGate* pi = getPi(i);
            int pattern = solver.getValue(pi->getSatVar());
            // assert(pattern != -1);
            if(pattern == -1)
                pi->setPValue((pi->getPValue()() << 1) | 0);
            else
                pi->setPValue((pi->getPValue()() << 1) | pattern);
        }
        ++_numCEX;
    }
    return result;
}


