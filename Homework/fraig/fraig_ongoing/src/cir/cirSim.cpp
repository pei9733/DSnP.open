/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
  initSim();
  if(fecGrpSize() == 0)
    initFEC();
  size_t _totalpatternNum = 0, _leastSimPatternNum = 65536;
  float _newFecRate = 0.0, _prevFecNum = (float)fecGrpSize();
  while(1){
    for(size_t i = 0, ni = getNumPIs(); i < ni; ++i){
      CirGate* pi = getPi(i);
      pi->setPValue(CirPValue(rnGen.getRandomULL1(0, ULLONG_MAX)));
    }
    simAndCheckFEC();
    _totalpatternNum += 64;
    if(_prevFecNum == 0.0){
        if(fecGrpSize() == 0)
            _newFecRate = 0.0;
        else
            _newFecRate = 1.0;
    }
    else
        _newFecRate = (abs)(((float)fecGrpSize() - _prevFecNum) / _prevFecNum);
    _prevFecNum = (float)fecGrpSize();
    cout <<  "\rfecGrpNum = " << fecGrpSize();
    if(_newFecRate <= 0.00001 && _totalpatternNum >= _leastSimPatternNum){
        cout << endl << _totalpatternNum << " patterns simulated." << endl;
        finalizeFEC();
        return;
    }
  }
  return;
}

void
CirMgr::fileSim(ifstream& patternFile)
{
  string _pattern;
  int _patternNum = 0, _totalPatternNum = 0;
  bool _first = 1;
  initSim();
  if(fecGrpSize() == 0)
    initFEC();
  while(patternFile >> _pattern){
    if(_pattern.length() != getNumPIs()){
      cout << "Error: Pattern(" << _pattern << ") length(" << _pattern.length() <<
              ") does not match the number of inputs(" << getNumPIs() << ") in a circuit!!" << endl;
      return;
    }
    for(size_t i = 0; i < _pattern.length(); ++i){
      if(_pattern[i] != '0' && _pattern[i] != '1'){
        cout << "Error: Pattern(" << _pattern << "contains a non-0/1 character('" << _pattern[i] << "')." << endl;
        return;
      }
      CirGate* pi = getPi(i);
      pi->setPValue((pi->getPValue()() << 1) | (_pattern[i] - '0'));
    } 
    ++_patternNum;
    ++_totalPatternNum;
    _first = 1;
    if(_patternNum == 64){
      for(auto i : _dfsList){
         i->pSim();
      }
      _patternNum = 0;
      _first = 0;
      checkFEC();
    }
  }
  if(_first){
    for(auto i : _dfsList){
         i->pSim();
    }
    checkFEC();
  }
  finalizeFEC();
  cout << _totalPatternNum << " patterns simulated." << endl;
  return;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
void CirMgr::initSim(){
  _const0->setPValue(0);
}