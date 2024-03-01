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

template <class T> class DList;

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

   DListNode(const T& d, DListNode<T>* p = 0, DListNode<T>* n = 0):
      _data(d), _prev(p), _next(n) {}

   // [NOTE] DO NOT ADD or REMOVE any data member
   T              _data;
   DListNode<T>*  _prev;
   DListNode<T>*  _next;
};


template <class T>
class DList
{
public:
   // TODO: decide the initial value for _isSorted
   DList() {
      _head = new DListNode<T>(T());
      _head->_prev = _head->_next = _head; // _head is a dummy node
   }
   ~DList() { clear(); delete _head; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class DList;

   public:
      iterator(DListNode<T>* n= 0): _node(n) {}
      iterator(const iterator& i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return _node->_data; }
      T& operator * () { return _node->_data; }
      iterator& operator ++ () { _node = _node->_next; return *(this); }
      iterator operator ++ (int) { iterator temp = *this; ++*(this); return *(this); }
      iterator& operator -- () { _node = _node->_prev; return *(this); }
      iterator operator -- (int) { iterator temp = *this; --*(this); return *(this); }

      iterator& operator = (const iterator& i) { 
         _node = i._node;
         return *(this);
      }

      bool operator != (const iterator& i) const { return _node != i._node; }
      bool operator == (const iterator& i) const { return _node == i._node; }

   private:
      DListNode<T>* _node;
   };

   // TODO: implement these functions
   iterator begin() const { return iterator(_head->_next); }
   iterator end() const { return iterator(_head); }
   bool empty() const { return _head->_next == _head; }
   size_t size() const {
      // cout <<"size()" << endl;
      size_t i = 0;
      if (!empty()) {
         for (auto e: *this) {
            ++i;
         }
      }
      return i;
   }

   void push_back(const T& x) const {
      DListNode<T>* node = new DListNode<T>(x,_head->_prev,_head);
      _head->_prev->_next = node;
      _head->_prev = node;
      _isSorted = false;
   }
   void pop_front() {
      DListNode<T>* pop = _head->_next;
      pop->_prev->_next = pop->_next;
      pop->_next->_prev = pop->_prev;

   }
   void pop_back() {
      DListNode<T>* pop = _head->_prev;
      pop->_prev->_next = pop->_next;
      pop->_next->_prev = pop->_prev;
   }

   // return false if nothing to erase
   bool erase(iterator pos) const {
      if (size() == 0) return false;
      DListNode<T>* pop = pos._node;
      pop->_prev->_next = pop->_next;
      pop->_next->_prev = pop->_prev;
      return true;
   }
   bool erase(const T& x) { 
      iterator itr = find(x);
      if (itr == end()) return false;
      else {
         erase(itr);
         return true;
      }
   }

   iterator find(const T& x) {
      for (iterator ite = begin(); ite != end(); ++ite) {
         if (*ite == x) {
            return ite;
         }
      }
      return end();
   }

   void clear() {
      _head->_next = _head;
      _head->_prev = _head;
   }  // delete all nodes except for the dummy node

   void sort() const {
      size_t i = size();
      iterator ite;
      iterator min;
      if (!empty()) {
         while(i > 0) {
            ite = begin();
            min = begin();
            for (size_t j = 0; j < i; ++j) {
               // cout << *ite << endl;
               if (*ite < *min) min = ite;
               ++ite;
            }
            erase(min);
            push_back(*min);
            --i;
         }
      }
      _isSorted = true;
   }


private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   DListNode<T>*  _head;     // = dummy node if list is empty
   mutable bool   _isSorted; // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] helper functions; called by public member functions
};

#endif // DLIST_H
