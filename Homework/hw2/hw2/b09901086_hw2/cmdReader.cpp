/****************************************************************************
  FileName     [ cmdReader.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command line reader member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <cstring>
#include "cmdParser.h"

using namespace std;

//----------------------------------------------------------------------
//    cursor manipulation
//----------------------------------------------------------------------

#define CUF "\033[1C"   // cursor forward
#define CUR "\r\033[5C" // cursor to the beginning of the line

//----------------------------------------------------------------------
//    Extrenal funcitons
//----------------------------------------------------------------------
void mybeep();
char mygetc(istream &);
ParseChar getChar(istream &);

//----------------------------------------------------------------------
//    Member Function for class Parser
//----------------------------------------------------------------------
void CmdParser::readCmd()
{
   if (_dofile.is_open())
   {
      readCmdInt(_dofile);
      _dofile.close();
   }
   else
      readCmdInt(cin);
}

void CmdParser::readCmdInt(istream &istr)
{
   resetBufAndPrintPrompt();
   while (1)
   {
      ParseChar pch = getChar(istr);
      if (pch == INPUT_END_KEY)
         break;
      switch (pch)
      {
      case LINE_BEGIN_KEY:
      case HOME_KEY:
         moveBufPtr(_readBuf);
         break;
      case LINE_END_KEY:
      case END_KEY:
         moveBufPtr(_readBufEnd);
         break;
      case BACK_SPACE_KEY: /* TODO */ // check
         if (_readBufPtr - _readBuf == 0)
         {
            mybeep();
            break;
         }
         for (int i = 0, n = _readBufEnd - _readBufPtr; i < n; ++i)
            *(_readBufPtr - 1 + i) = *(_readBufPtr + i);
         _readBufEnd -= 1;
         *_readBufEnd = 0;
         moveBufPtr(_readBufPtr - 1);
         for (int i = _readBufPtr - _readBuf, n = _readBufEnd - _readBuf; i < n; ++i)
            cout << _readBuf[i];
         cout << " " << CUR;
         for (int i = 0, n = _readBufPtr - _readBuf; i < n; ++i)
            cout << CUF;
         break;
      case DELETE_KEY: // check
         deleteChar();
         break;
      case NEWLINE_KEY: // check
         addHistory();
         cout << char(NEWLINE_KEY);
         resetBufAndPrintPrompt();
         break;
      case ARROW_UP_KEY: // check
         moveToHistory(_historyIdx - 1);
         break;
      case ARROW_DOWN_KEY: // check
         moveToHistory(_historyIdx + 1);
         break;
      case ARROW_RIGHT_KEY: /* TODO */ // check
         moveBufPtr(_readBufPtr + 1);
         break;
      case ARROW_LEFT_KEY: /* TODO */ // check
         moveBufPtr(_readBufPtr - 1);
         break;
      case PG_UP_KEY: // check
         moveToHistory(_historyIdx - PG_OFFSET);
         break;
      case PG_DOWN_KEY: // check
         moveToHistory(_historyIdx + PG_OFFSET);
         break;
      case TAB_KEY: /* TODO */ // check
         insertChar(' ', TAB_POSITION - (_readBufPtr - _readBuf) % TAB_POSITION);
         break;
      case INSERT_KEY:    // not yet supported; fall through to UNDEFINE
      case UNDEFINED_KEY: // ctrl+L    // remember to recover to mybeep() only
         mybeep();
         // printH(_history);
         break;
      default: // printable character  //check
         insertChar(char(pch));
         break;
      }
#ifdef TA_KB_SETTING
      taTestOnly();
#endif
   }
}

// This function moves _readBufPtr to the "ptr" pointer
// It is used by left/right arrowkeys, home/end, etc.
//
// Suggested steps:
// 1. Make sure ptr is within [_readBuf, _readBufEnd].
//    If not, make a beep sound and return false. (DON'T MOVE)
// 2. Move the cursor to the left or right, depending on ptr
// 3. Update _readBufPtr accordingly. The content of the _readBuf[] will
//    not be changed
//
// [Note] This function can also be called by other member functions below
//        to move the _readBufPtr to proper position.
bool CmdParser::moveBufPtr(char *const ptr)
{
   // TODO...
   if (ptr < _readBuf || ptr > _readBufEnd)
   {
      mybeep();
      return 0;
   }
   bool cursorDir = _readBufPtr > ptr ? 1 : 0; // true : left; false: right
   int tomove = cursorDir ? _readBufPtr - ptr : ptr - _readBufPtr;
   // cout << tomove << endl;
   for (int i = 0; i < tomove; ++i)
   {
      cout << (cursorDir ? "\b" : CUF);
      // if (cursorDir)
      //    cout << "\b";
   }

   _readBufPtr = ptr;
   return true;
}

// [Notes]
// 1. Delete the char at _readBufPtr
// 2. mybeep() and return false if at _readBufEnd
// 3. Move the remaining string left for one character
// 4. The cursor should stay at the same position
// 5. Remember to update _readBufEnd accordingly.
// 6. Don't leave the tailing "visible" character.
// 7. Call "moveBufPtr(...)" if needed.
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling deleteChar()---
//
// cmd> This is he command
//              ^
//
bool CmdParser::deleteChar()
{
   // TODO...
   if (_readBufEnd == _readBufPtr)
   {
      mybeep();
      return 0;
   }
   for (int i = 0, n = _readBufEnd - _readBufPtr; i < n; ++i)
      *(_readBufPtr + i) = *(_readBufPtr + i + 1);
   _readBufEnd -= 1;
   *_readBufEnd = 0;
   for (int i = _readBufPtr - _readBuf, n = _readBufEnd - _readBuf; i < n; ++i)
      cout << _readBuf[i];
   cout << " " << CUR;
   for (int i = 0, n = _readBufPtr - _readBuf; i < n; ++i)
      cout << CUF;
   // printRead();
   return true;
}

// 1. Insert character 'ch' for "repeat" times at _readBufPtr
// 2. Move the remaining string right for "repeat" characters
// 3. The cursor should move right for "repeats" positions afterwards
// 4. Default value for "repeat" is 1. You should assert that (repeat >= 1).
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling insertChar('k', 3) ---
//
// cmd> This is kkkthe command
//                 ^
//
void CmdParser::insertChar(char ch, int repeat)
{
   // TODO...
   assert(repeat >= 1);

   char *curEnd = _readBufEnd;
   _readBufEnd += repeat;
   // printf("\033[%dC", _readBufEnd - _readBufPtr);
   for (int i = 0, n = _readBufEnd - _readBufPtr; i < n; ++i)
   {
      *(_readBufEnd - i) = *(curEnd - i);
   }
   for (int i = 0; i < repeat; ++i)
   {
      *(_readBufPtr + i) = ch;
      // cout << ch;
   }
   cout << CUR;
   for (int i = 0; i < _readBufEnd - _readBuf; ++i)
      cout << _readBuf[i];
   _readBufPtr += repeat;
   for (int i = 0, n = _readBufEnd - _readBufPtr; i < n; ++i)
      cout << "\b";
}

// 1. Delete the line that is currently shown on the screen
// 2. Reset _readBufPtr and _readBufEnd to _readBuf
// 3. Make sure *_readBufEnd = 0
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling deleteLine() ---
//
// cmd>
//      ^
//
void CmdParser::deleteLine()
{
   cout << "\r\033[0K";
   printPrompt();
   _readBufPtr = _readBufEnd = _readBuf;
   *_readBufPtr = 0;
}

// This functions moves _historyIdx to index and display _history[index]
// on the screen.
//
// Need to consider:
// If moving up... (i.e. index < _historyIdx)
// 1. If already at top (i.e. _historyIdx == 0), beep and do nothing.
// 2. If at bottom, temporarily record _readBuf to history.
//    (Do not remove spaces, and set _tempCmdStored to "true")
// 3. If index < 0, let index = 0.
//
// If moving down... (i.e. index > _historyIdx)
// 1. If already at bottom, beep and do nothing
// 2. If index >= _history.size(), let index = _history.size() - 1.
//
// Assign _historyIdx to index at the end.
//
// [Note] index should not = _historyIdx
//
void CmdParser::moveToHistory(int index)
{

   if ((index < _historyIdx && _historyIdx == 0) || (index > _historyIdx && _historyIdx >= _history.size() - 1))
   {
      mybeep();
      return;
   }
   index = index < 0 ? 0 : index;
   index = (index >= _history.size()) ? _history.size() - 1 : index;
   if (_historyIdx == _history.size())
   {
      string strToAdd(_readBuf);
      _history.push_back(strToAdd);
      _tempCmdStored = 1;
   }
   _historyIdx = index;
   retrieveHistory();
   // TODO...
}

// This function adds the string in _readBuf to the _history.
// The size of _history may or may not change. Depending on whether
// there is a temp history string.
//
// 1. Remove ' ' at the beginning and end of _readBuf
// 2. If not a null string, add string to _history.
//    Be sure you are adding to the right entry of _history.
// 3. If it is a null string, don't add anything to _history.
// 4. Make sure to clean up "temp recorded string" (added earlier by up/pgUp,
//    and reset _tempCmdStored to false
// 5. Reset _historyIdx to _history.size() // for future insertion
//
void CmdParser::addHistory()
{
   string strToAdd(_readBuf);
   if (strToAdd.find_first_not_of(' ') == strToAdd.npos)
   {
      if (_tempCmdStored)
      {
         _history.pop_back();
         _tempCmdStored = 0;
      }
      _historyIdx = _history.size();
      return;
   }

   int _front = 0, _back = 0;
   bool _isFronted = 0, _isBacked = 0;
   for (int i = 0, n = strToAdd.size(), j = n - 1; i < n && j >= 0 && (!_isFronted || !_isBacked); ++i, --j)
   {
      if (!_isFronted && strToAdd[i] != ' ')
      {
         _front = i;
         _isFronted = 1;
      }
      if (!_isBacked && strToAdd[j] != ' ')
      {
         _back = j;
         _isBacked = 1;
      }
   }
   strToAdd = strToAdd.substr(_front, _back - _front + 1);
   if (!strToAdd.empty())
   {
      if (_tempCmdStored)
      {
         _history[_history.size() - 1] = strToAdd;
         _tempCmdStored = 0;
      }
      else
         _history.push_back(strToAdd);
      _historyIdx = _history.size();
   }

   // print(_history);

   // TODO...
}

// 1. Replace current line with _history[_historyIdx] on the screen
// 2. Set _readBufPtr and _readBufEnd to end of line
//
// [Note] Do not change _history.size().
//
void CmdParser::retrieveHistory()
{
   deleteLine();
   strcpy(_readBuf, _history[_historyIdx].c_str());
   cout << _readBuf;
   _readBufPtr = _readBufEnd = _readBuf + _history[_historyIdx].size();
}
