/****************************************************************************
  FileName     [ array.h ]
  PackageName  [ util ]
  Synopsis     [ Define dynamic array package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef ARRAY_H
#define ARRAY_H

#include <cassert>
#include <algorithm>

using namespace std;

// NO need to implement class ArrayNode
//
template <class T>
class Array
{
public:
   // TODO: decide the initial value for _isSorted
   Array() : _data(0), _size(0), _capacity(0)
   {
      _isSorted = 0;
      // _data = new T[_capacity];
   }
   ~Array() { delete[] _data; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class Array;

   public:
      iterator(T *n = 0) : _node(n) {}
      iterator(const iterator &i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T &operator*() const
      {
         return (*_node);
      }
      T &operator*() { return (*_node); }
      iterator &operator++() //++it
      {
         _node = _node + 1;
         return (*this);
      }
      iterator operator++(int)
      {
         T *_ret = _node;
         _node = _node + 1;
         return _ret;
      }
      iterator &operator--()
      {
         _node = _node - 1;
         return (*this);
      }
      iterator operator--(int)
      {
         T *_ret = _node;
         _node = _node - 1;
         return _ret;
      }

      iterator operator+(int i) const
      {
         iterator _ret = (*this);
         _ret._node = _ret._node + i;
         return _ret;
      }
      iterator &operator+=(int i)
      {
         _node = _node + i;
         return (*this);
      }

      iterator &operator=(const iterator &i)
      {
         _node = i._node;
         return (*this);
      }

      bool operator!=(const iterator &i) const
      {
         return (_node != i._node);
      }
      bool operator==(const iterator &i) const
      {
         return (_node == i._node);
      }

   private:
      T *_node;
   };

   // TODO: implement these functions
   iterator begin() const { return _data; }
   iterator end() const { return _data + _size; }
   bool empty() const { return (_size == 0); }
   size_t size() const { return _size; }

   T &operator[](size_t i)
   {
      assert(i < _size);
      return _data[i];
   }
   const T &operator[](size_t i) const
   {
      assert(i < _size);
      return _data[i];
   }

   void push_back(const T &x)
   {
      if (_size == _capacity)
         expand();
      _data[_size++] = x;
      _isSorted = 0;
   }
   void pop_front()
   {
      if (empty())
      {
         _isSorted = 0;
         return;
      }
      if (_size-- >= 2)
         _data[0] = _data[_size];
      _isSorted = 0;
   }
   void pop_back()
   {
      if (!empty())
         --_size;
      else
         _isSorted = 0;
   }

   bool erase(iterator pos)
   {
      if (empty())
         return 0;
      *pos = _data[--_size];
      _isSorted = 0;
      return 1;
   }
   bool erase(const T &x)
   {
      iterator todel = find(x);
      if (todel == end())
         return 0;
      *todel = _data[--_size];
      _isSorted = 0;
      return 1;
   }

   iterator find(const T &x)
   {
      for (iterator li = begin(); li != end(); ++li)
         if ((*li) == x)
            return li;
      return end();
   }

   void clear()
   {
      _size = 0;
      _isSorted = 0;
   }

   // [Optional TODO] Feel free to change, but DO NOT change ::sort()
   void sort() const
   {
      if (!empty() && !_isSorted)
         ::sort(_data, _data + _size);
      _isSorted = 1;
   }

   // Nice to have, but not required in this homework...
   // void reserve(size_t n) { ... }
   // void resize(size_t n) { ... }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   T *_data;
   size_t _size;           // number of valid elements
   size_t _capacity;       // max number of elements
   mutable bool _isSorted; // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] Helper functions; called by public member functions
   void expand()
   {
      if (_capacity == 0)
      {
         _data = new T[1];
         _capacity = 1;
         return;
      }
      T *newData = new T[_capacity * 2];
      size_t _idx = 0;
      for (iterator li = begin(); li != end(); ++li, ++_idx)
         newData[_idx] = *(li);
      T *todel = _data;
      _data = newData;
      _capacity *= 2;
      delete[] todel;
   }
};

#endif // ARRAY_H
