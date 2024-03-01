/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BST_H
#define BST_H

#include <cassert>

using namespace std;

template <class T> class BSTree;
template <class T> class BSTreeNode;

// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data membeㄑ or function to this class!!
//
template <class T>
class BSTreeNode
{
   // TODO: design your own class!!
   friend class BSTree<T>;
   friend class BSTree<T>::iterator;

   BSTreeNode(const T& d, BSTreeNode<T>* p = NULL, BSTreeNode<T>* l = NULL, BSTreeNode<T>* r = NULL):
      _data(d), _par(p), _left(l), _right(r) {}

   // [NOTE] DO NOT ADD or REMOVE any data member
   bool isParL() {return _par == NULL ? false : _par->_left == this;}
   bool isParR() {return _par == NULL ? false : _par->_right == this;}
   bool isLeaf() {return (_left == NULL) && (_right == NULL);}
   BSTreeNode<T>* min () {
      BSTreeNode<T>* m = this;
      while (m->_left) m = m->_left;
      return m;
   }
   BSTreeNode<T>* max () {
      BSTreeNode<T>* m = this;
      while (m->_right) m = m->_right;
      return m;
   }
   T              _data;
   BSTreeNode<T>*  _par;
   BSTreeNode<T>*  _left;
   BSTreeNode<T>*  _right;
};


template <class T>
class BSTree
{
   // TODO: design your own class!!
   public:
   BSTree() {
      _root = NULL;
   }
   ~BSTree() { clear(); delete _root; delete _finder; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class BSTree;

   public:
      iterator(BSTreeNode<T>* n= 0): _node(n) {}
      iterator(const iterator& i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return _node->_data; }
      T& operator * () { return _node->_data; }
      iterator& operator ++ () { 
         if (_node->_right) _node = _node->_right->min();
         else {
            while(_node->isParR()) _node = _node->_par;
            _node = _node->_par;
         }
         return *(this);
      }
      iterator operator ++ (int) { iterator temp = *this; ++*(this); return *(this); }
      iterator& operator -- () { 
         if (_node->_left) _node = _node->_left->max();
         else {
            while(_node->isParL()) _node = _node->_par;
            _node = _node->_par;
         }
         return *(this);
      }
      iterator operator -- (int) { iterator temp = *this; --*(this); return *(this); }

      iterator& operator = (const iterator& i) { 
         _node = i._node;
         return *(this);
      }
      bool operator != (const iterator& i) const { return _node != i._node; }
      bool operator == (const iterator& i) const { return _node == i._node; }

   private:
      BSTreeNode<T>* _node;
   };

   // TODO: implement these functions
   iterator begin() const {return empty() ? iterator(NULL) : iterator(_root->min());}
   iterator end() const { return iterator(NULL);}
   bool empty() const { return _root == NULL;}
   size_t size() const {
      size_t i = 0;
      if (!empty()) {
         for (auto e: *this) {
            ++i;
         }
      }
      return i;
   }

   void insert(const T& x) {
      BSTreeNode<T>* node;
      if (empty()) {
         node = new BSTreeNode<T> (x);
         _root = node;
      } else {
         find(x); 
         if (!_pseudoFinder) {
            node = new BSTreeNode<T> (x, _finder, _finder->_left, NULL);
            if (_finder->_left) _finder->_left->_par = node;
            _finder->_left = node;
            // _finder->_right = _finder->_right;
         } else {
            node = _finder;
            if (node->_data < node->_par->_data) node->_par->_left = node;
            else node->_par->_right = node;
         }
      }
   }
   void pop_front() {
      if (empty()) return;
      iterator ite = iterator(_root->min());
      erase(ite);
   }
   void pop_back() {
      if (empty()) return;
      iterator ite = iterator(_root->max());
      erase(ite);
   }

   // return false if nothing to erase
   bool erase(iterator pos) {
      if (empty()) return false;
      BSTreeNode<T>* n = pos._node;
      if (n->isLeaf()) {
         if (n->isParL()) n->_par->_left = NULL;
         else if (n->isParR()) n->_par->_right = NULL;
         else clear();
      } else if (n->_left == NULL) {
         if (n->isParL()) n->_par->_left = n->_right;
         else if (n->isParR()) n->_par->_right = n->_right;
      } else if (n->_right == NULL) {
         if (n->isParL()) n->_par->_left = n->_left;
         else if (n->isParR()) n->_par->_right = n->_left;
      } else {
         BSTreeNode<T>* suc = (++pos)._node;
         n->_data = suc->_data;
         erase(suc);
      }
      return true;
   }
   bool erase(const T& x) { 
      const iterator itr = find(x);
      if (itr == end()) return false;
      else {
         // cout << "in" << endl;
         erase(itr);
         return true;
      }
   }

   iterator find(const T& x) {
      _pseudoFinder = false;
      _finder = _root;
      while (true) {
         if (_finder->_data == x) return iterator(_finder);
         else if (_finder->_data > x && _finder->_left != NULL) {_finder = _finder->_left;}
         else if (_finder->_data < x && _finder->_right != NULL) {_finder = _finder->_right;}
         else {
            _finder = new BSTreeNode<T>(x, _finder);
            _pseudoFinder = true;
            return end();
         }
      }

   }

   void clear() {
      _root = NULL;
      _pseudoFinder = false;
      _finder = NULL;
   }  // delete all nodes except for the dummy node
   void sort() {}
   void print() const {
      for (auto& e: *this) {
         cout << e << endl;
      }
   }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   BSTreeNode<T>* _root;
   BSTreeNode<T>* _finder;
   bool _pseudoFinder;
   size_t _size;
   mutable bool   _isSorted; // (optionally) to indicate the array is sorted
};

#endif // BST_H
