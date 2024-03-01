/****************************************************************************
  FileName     [ dlist.h ]
  PackageName  [ util ]
  Synopsis     [ Define doubly linked list package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef DLIST_H
#define DLIST_H

#include <cassert>

template <class T>
class DList;

// DListNode is supposed to be a private class. User don't need to see it.
// Only DList and DList::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class DListNode
{
   friend class DList<T>;
   friend class DList<T>::iterator;

   DListNode(const T &d, DListNode<T> *p = 0, DListNode<T> *n = 0) : _data(d), _prev(p), _next(n) {}

   // [NOTE] DO NOT ADD or REMOVE any data member
   T _data;
   DListNode<T> *_prev;
   DListNode<T> *_next;
};

template <class T>
class DList
{
public:
   // TODO: decide the initial value for _isSorted
   DList()
   {
      _head = new DListNode<T>(T());
      _head->_prev = _head->_next = _head; // _head is a dummy node
      _isSorted = 0;
   }
   ~DList()
   {
      clear();
      delete _head;
   }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class DList;

   public:
      iterator(DListNode<T> *n = 0) : _node(n) {}
      iterator(const iterator &i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T &operator*() const { return _node->_data; }
      T &operator*() { return _node->_data; }
      iterator &operator++()
      {
         _node = _node->_next;
         return *(this);
      }
      iterator operator++(int)
      {
         iterator _old = *(this);
         _node = _node->_next;
         return _old;
      }
      iterator &operator--()
      {
         _node = _node->_prev;
         return *(this);
      }
      iterator operator--(int)
      {
         iterator _old = *(this);
         _node = _node->_prev;
         return _old;
      }
      iterator &operator=(const iterator &i)
      {
         _node = i._node;
         return *(this);
      }

      bool operator!=(const iterator &i) const
      {
         // if (i == 0)
         //    return (_node != 0);
         return _node != i._node ? 1 : 0;
      }
      bool operator==(const iterator &i) const
      {
         // if (i == 0)
         //    return (_node == 0);
         return _node == i._node ? 1 : 0;
      }

   private:
      DListNode<T> *_node;
   };

   // TODO: implement these functions
   iterator begin() const { return _head; }
   iterator end() const { return _head->_prev; }
   bool empty() const
   {
      return _head->_prev == _head ? 1 : 0;
   }
   size_t size() const
   {
      if (empty())
         return 0;
      size_t cnt = 0;
      for (iterator li = begin(); li != end(); ++li)
         cnt += 1;
      return cnt;
   }

   void push_back(const T &x)
   {
      DListNode<T> *newNode = new DListNode<T>(x);
      _isSorted = 0;
      if (empty())
      {
         _head->_prev = _head->_next = newNode;
         newNode->_prev = newNode->_next = _head;
         _head = newNode;
         return;
      }
      newNode->_prev = _head->_prev->_prev; // newNode->_prev points to last element
      newNode->_next = _head->_prev;        // newNode->_next points to dummy node
      _head->_prev->_prev->_next = newNode; // last element's _next points to newNode
      _head->_prev->_prev = newNode;        // dummy node's _prev points to newNode
   }
   void pop_front()
   {
      if (empty())
         return;
      DListNode<T> *_todel = _head;
      _head->_prev->_next = _head->_next;
      _head->_next->_prev = _head->_prev;
      _head = _head->_next;
      delete _todel;
   }
   void pop_back()
   {
      if (empty())
         return;
      DListNode<T> *_todel = _head->_prev->_prev;
      _todel->_next->_prev = _todel->_prev;
      _todel->_prev->_next = _todel->_next;
      delete _todel;
   }

   // return false if nothing to erase
   bool erase(iterator pos)
   {
      if (empty())
         return 0;
      pos._node->_prev->_next = pos._node->_next;
      pos._node->_next->_prev = pos._node->_prev;
      if (pos == begin())
         _head = _head->_next;
      delete pos._node;
      return 1;
   }
   bool erase(const T &x)
   {
      iterator tar = find(x);
      if (tar._node != _head->_prev)
      {
         cout << tar._node->_data << endl;
         tar._node->_prev->_next = tar._node->_next;
         tar._node->_next->_prev = tar._node->_prev;
         if (tar == begin())
            _head = _head->_next;
         // tar._node = tar._node->_next;
         delete tar._node;
         return 1;
      }
      return 0;
   }

   iterator find(const T &x)
   {
      for (iterator li = begin(); li != end(); ++li)
      {
         if (li._node->_data == x)
            return li;
      }
      return end();
   }

   void clear()
   {
      DListNode<T> *_dum = _head->_prev;
      for (iterator li = begin(); li != end();)
      {
         iterator _tmp = ++li;
         delete (--li)._node;
         li = _tmp;
      }
      _dum->_prev = _dum->_next = _dum;
      _head = _dum;
      _isSorted = 0;
      return;
      // _head->_prev->_next = _head->_prev;
      // _head->_prev->_prev = _head->_next;
      // for (iterator li = begin(); li != end(); ++li)
      //    delete li._node;
      // _isSorted = 0;
      // return;
   } // delete all nodes except for the dummy node

   void sort() const
   {
      if (_isSorted)
         return;
      for (iterator li = ++begin(), _end = end(); li != _end; ++li)
      {
         T key = *(li);
         iterator lj = li;
         --lj;
         while (lj != --begin() && *(lj) > key)
         {
            lj._node->_next->_data = *(lj);
            --lj;
         }
         lj._node->_next->_data = key;
      }
      _isSorted = 1;
   }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   DListNode<T> *_head;    // = dummy node if list is empty
   mutable bool _isSorted; // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] helper functions; called by public member functions
};

#endif // DLIST_H
