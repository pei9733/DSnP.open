/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <algorithm>
#include <cassert>

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
  clearList(_undefList);
  clearList(_floatList);
  clearList(_unusedList);
  for(size_t i = 0, n = getNumTots(); i < n; ++i){
    if(find(_dfsList.begin(), _dfsList.end(),_totGateList[i]) == _dfsList.end() && _totGateList[i]->getType() != PI_GATE && _totGateList[i]->getType() != CONST_GATE){
      cout << "Sweeping: " << _totGateList[i]->getTypeStr() <<'(' << _totGateList[i]->getGid() << ") removed...\n"; 
      if(_totGateList[i]->getType() == AIG_GATE)
        --_numDecl[AIG];
      delete _totGateList[i];
    }
    else if(find(_dfsList.begin(), _dfsList.end(),_totGateList[i]) == _dfsList.end() && _totGateList[i]->getType() == PI_GATE ){
      _unusedList.push_back(_totGateList[i]->getGid());
    }
  }
  checkFloatList();

}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
  for(auto i : _dfsList){
    if(i->getType() == AIG_GATE){
      cout << i->getGid() << ';'; // aig gate 11 is weird : i->getIn1().isInv() == 1, _in1.isInv() == 0
      CirGateV _in0 = i->getIn0().gate()->hasEqGate() ? i->getIn0().gate()->getEqGate() : i->getIn0();
      CirGateV _in1 = i->getIn1().gate()->hasEqGate() ? i->getIn1().gate()->getEqGate() : i->getIn1();
      if(i->getIn0().gate()->hasEqGate()){
        _in0.setInv(i->getIn0().isInv());
        i->replaceFanin(i->getIn0().gate(), _in0.gate(), _in0.isInv());
      }
      if(i->getIn1().gate()->hasEqGate()){
        _in1.setInv(i->getIn1().isInv());
        i->replaceFanin(i->getIn1().gate(), _in1.gate(), _in1.isInv());
      }
      bool is0const = _in0.gate()->getType() == CONST_GATE, is1const = _in1.gate()->getType() == CONST_GATE;
      // if(i->getIn0().gate()->getEqGate() == 0)
      //     is0const = i->getIn0().gate()->getType() == CONST_GATE;
      // else
      //     is0const = i->getIn0().gate()->getEqGate().gate()->getType() == CONST_GATE;
      // if(i->getIn1().gate()->getEqGate() == 0)
      //     is1const = i->getIn1().gate()->getType() == CONST_GATE;
      // else
      //     is1const = i->getIn1().gate()->getEqGate().gate()->getType() == CONST_GATE;
      if(is0const || is1const){
        CirGateV _inConstGate = is0const ? _in0 : _in1, _inGate = is0const ? _in1 : _in0;
        if(_inConstGate.isInv()){  // const 1
          cout << _inGate.gateId() << endl;
          // if(_inGate.gate()->getEqGate() != 0)
          //   i->setEqGate(_inGate.gate()->getEqGate().gate(), _inGate.gate()->getEqGate().isInv());
          // else
            i->setEqGate(_inGate.gate(), _inGate.isInv());
        } 
        else{
          cout << _inConstGate.gateId() << endl;
          // size_t tmpgid = i->getGid();
          // assert(_totGateList[tmpgid] == i);
          // delete _totGateList[tmpgid];
          // _totGateList[tmpgid] = _const0;
          i->setEqGate(_const0, 0);
        }
      }
      else if(_in0.gate() == _in1.gate()){
        if(!(_in0.isInv() ^ _in1.isInv())){  // same -> 0
          cout << _in0.gateId() << endl;
          // if(i->getIn0().gate()->getEqGate() != 0)
          //   i->setEqGate(i->getIn0().gate()->getEqGate().gate(), i->getIn0().gate()->getEqGate().isInv());
          // else
            i->setEqGate(_in0.gate(), _in0.isInv());
        }
        else{
          // size_t tmpgid = i->getGid();
          // assert(_totGateList[tmpgid] == i);
          // delete _totGateList[tmpgid];
          // _totGateList[tmpgid] = _const0;
          i->setEqGate(_const0, 0);
        }
      }
    }
    else{
      if(i->getType() == PO_GATE){
        if(i->getIn0().gate()->hasEqGate()){
          i->replaceFanin(i, i->getIn0().gate()->getEqGate().gate(), i->getIn0().gate()->getEqGate().isInv());
        }
      }
      cout << endl;
    }
  }
  genDfsList();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
