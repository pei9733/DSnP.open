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
#define COUNT 10
using namespace std;

template <class T>
class BSTree;

// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class BSTreeNode
{
   friend class BSTree<T>;
   friend class BSTree<T>::iterator;

   BSTreeNode(const T &d, BSTreeNode<T> *p = 0, BSTreeNode<T> *l = 0, BSTreeNode<T> *r = 0) : _parent(p), _left(l), _right(r), _data(d) {}
   BSTreeNode *_parent;
   BSTreeNode *_left;
   BSTreeNode *_right;
   T _data;

   // TODO: design your own class!!
};

template <class T>
class BSTree
{
   friend class iterator;

public:
   // TODO: decide the initial value for _isSorted
   BSTree() : _root(0)
   {
   }
   ~BSTree()
   {
      clear();
      delete _root;
   }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class BSTree;

   public:
      iterator(BSTreeNode<T> *n = 0) : _node(n) {}
      iterator(const iterator &i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T &operator*() const { return _node->_data; }
      T &operator*() { return _node->_data; }
      iterator &operator++() // ++it
      {
         _node = BSTree::successor(_node);
         return *(this);
      }
      iterator operator++(int) // it++
      {
         iterator _ret = *(this);
         _node = BSTree::successor(_node);
         return *(this);
      }
      iterator &operator--()
      { //--it
         _node = BSTree::predecessor(_node);
         return *(this);
      }
      iterator operator--(int)
      { // it--
         iterator _ret = *(this);
         _node = BSTree::predecessor(_node);
         return *(this);
      }

      iterator &operator=(const iterator &i)
      {
         _node = i._node;
         return *(this);
      }

      bool operator!=(const iterator &i) const { return (i._node != _node); }
      bool operator==(const iterator &i) const { return (i._node == _node); }

   private:
      BSTreeNode<T> *_node;
   };

   // TODO: implement these functions
   iterator begin() const { return min(_root); }
   iterator end() const { return 0; }
   bool empty() const { return (_root == 0); }
   size_t size() const
   {
      size_t cnt = 0;
      for (iterator li = begin(); li != end(); ++li, ++cnt)
         ;
      return cnt;
   }

   void insert(const T &z)
   {
      BSTreeNode<T> *_z = new BSTreeNode<T>(z);
      BSTreeNode<T> *_y = 0, *_x = _root;
      while (_x != 0)
      {
         _y = _x;
         if (_z->_data < _x->_data)
            _x = _x->_left;
         else
            _x = _x->_right;
      }
      _z->_parent = _y;
      if (_y == 0)
         _root = _z; // Tree is empty
      else if (_z->_data < _y->_data)
         _y->_left = _z;
      else
         _y->_right = _z;
   }
   void pop_front()
   {
      if (empty())
         return;
      BSTreeNode<T> *z = min(_root);
      deleteZ(z);
      delete z;
   }
   void pop_back()
   {
      if (empty())
         return;
      BSTreeNode<T> *z = max(_root);
      deleteZ(z);
      delete z;
   }

   // return false if nothing to erase
   bool erase(iterator pos)
   {
      if (empty())
         return false;
      deleteZ(pos._node);
      delete pos._node;
      return true;
   }
   bool erase(const T &x)
   {
      iterator z = find(x);
      if (z == 0)
         return false;
      deleteZ(z._node);
      delete z._node;
      return true;
   }

   iterator find(const T &x)
   {
      BSTreeNode<T> *rot = _root;
      while (rot != 0 && x != rot->_data)
      {
         if (x < rot->_data)
            rot = rot->_left;
         else
            rot = rot->_right;
      }
      return rot;
   }

   void clear()
   {
      for (iterator li = begin(); li != end(); ++li)
      {
         deleteZ(li._node);
         delete li._node;
      }
      _root = 0;
   }

   void sort() const {}
   void print() const
   {
      print2DUtil(_root, 0);
   }

private:
   BSTreeNode<T> *_root;
   // [OPTIONAL TODO] helper functions; called by public member functions
   static bool _isLeftChild(BSTreeNode<T> *_tocheck) { return (_tocheck->_parent->_left == _tocheck); }
   static bool _isRightChild(BSTreeNode<T> *_tocheck) { return (_tocheck->_parent->_right == _tocheck); }
   static BSTreeNode<T> *min(BSTreeNode<T> *_root)
   {
      if (_root == 0)
         return 0;
      while (_root->_left != 0)
         _root = _root->_left;
      return _root;
   }

   static BSTreeNode<T> *max(BSTreeNode<T> *_root)
   {
      if (_root == 0)
         return 0;
      while (_root->_right != 0)
         _root = _root->_right;
      return _root;
   }

   static BSTreeNode<T> *successor(BSTreeNode<T> *_root)
   {
      if (_root == 0)
         return 0;
      if (_root->_right != 0)
         return min(_root->_right);
      BSTreeNode<T> *_p = _root->_parent;
      while (_p != 0 && _isRightChild(_root))
      {
         _root = _p;
         _p = _p->_parent;
      }
      return _p;
   }

   static BSTreeNode<T> *predecessor(BSTreeNode<T> *_root)
   {
      if (_root == 0)
         return 0;
      if (_root->_left != 0)
         return max(_root->_left);
      BSTreeNode<T> *_p = _root->_parent;
      while (_p != 0 && _isLeftChild(_root))
      {
         _root = _p;
         _p = _p->_parent;
      }
      return _p;
   }

   void print2DUtil(BSTreeNode<T> *root, int space) const
   {
      // Base case
      if (root == NULL)
         return;

      // Increase distance between levels
      space += COUNT;

      // Process right child first
      print2DUtil(root->_right, space);

      // Print current node after space
      // count
      cout << endl;
      for (int i = COUNT; i < space; i++)
         cout << " ";
      cout << root->_data << "\n";

      // Process left child
      print2DUtil(root->_left, space);
   }

   void transplant(BSTreeNode<T> *u, BSTreeNode<T> *v)
   {
      if (_root == 0)
         return;
      if (u->_parent == 0)
         _root = v;
      else if (_isLeftChild(u))
         u->_parent->_left = v;
      else
         u->_parent->_right = v;
      if (v != 0)
         v->_parent = u->_parent;
   }

   void deleteZ(BSTreeNode<T> *z)
   {
      if (_root == 0)
         return;
      if (z->_left == 0)
         transplant(z, z->_right);
      else if (z->_right == 0)
         transplant(z, z->_left);
      else
      {
         BSTreeNode<T> *y = min(z->_right);
         if (y->_parent != z)
         {
            transplant(y, y->_right);
            y->_right = z->_right;
            y->_right->_parent = y;
         }
         transplant(z, y);
         y->_left = z->_left;
         y->_left->_parent = y;
      }
   }
};

#endif // BST_H
