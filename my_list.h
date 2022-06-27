#ifndef __MY_LIST_
#define __MY_LIST_

#include "my_allocator.h"
#include "my_iterator.h"
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <list>
#include <stdexcept>
#include <utility>
namespace jan {

/**
 * @brief 双向链表的节点
 *
 * @tparam T
 */
template <typename T> struct _list_node {
  using self = _list_node<T>;
  T _data;
  self *_pre;
  self *_next;
  template <typename... Args>
  explicit _list_node(Args &&...args, self *pre = nullptr, self *next = nullptr)
      : _data(10), _pre(pre), _next(next) {}

  explicit _list_node(const T &val = T(), self *pre = nullptr,
                      self *next = nullptr)
      : _data(val), _pre(pre), _next(next) {}
};

/**
 * @brief 双向链表的迭代器, 此迭代器种类为双向迭代器,支持operator->
 *
 * @tparam T
 */
template <typename T> struct _list_iterator {
  using value_type = T;
  using pointer = T *;
  using receference = T &;
  using size_type = size_t;
  using iterator_category = bidirectional_iterator_tag;
  using self = _list_iterator<T>;
  using node_type = _list_node<T>;
  using difference_type = ptrdiff_t;
  T &operator*() { return _node->_data; }
  T *operator->() { return &*_node; }
  bool operator==(const self &rhs) { return _node == rhs._node; }
  bool operator!=(const self &rhs) { return _node != rhs._node; }
  self &operator++() {
    _node = _node->_next;
    return *this;
  }
  self operator++(int) {
    auto ret = *this;
    ++*this;
    return ret;
  }
  self &operator--() {
    _node = _node->_pre;
    return *this;
  }
  self operator--(int) {
    auto ret = *this;
    --*this;
    return ret;
  }
  node_type *_node;
  explicit _list_iterator(node_type *node = nullptr) : _node(node) {}
  _list_iterator(const self &iteraotr) : _list_iterator(iteraotr._node) {}
};

/**
 * @brief 双向链表容器, O(1)的插入和删除，基于双向环状链表组织
 *
 * @tparam T
 * @tparam Alloc
 */
template <typename T, typename Alloc = jan::alloc> class list {
private:
  using node_type = _list_node<T>;

public:
  using iterator = _list_iterator<T>;
  using size_type = size_t;
  iterator begin() const { return iterator(_base_node->_next); }
  iterator end() const { return iterator(_base_node); }
  bool empty() const { return _base_node->_next == _base_node; }
  size_type size() const { return distance(begin(), end()); }
  T &front() { return *begin(); }
  T &back() { return *(--end()); }

  iterator insert(iterator pos, const T &val);
  void push_front(const T &val) { insert(begin(), val); }
  void push_back(const T &val) { insert(end(), val); }

  iterator erase(iterator pos);
  void pop_front() { erase(begin()); }
  void pop_back() { auto tmp = end(); erase(--tmp); }

  void clear();
  void remove(const T & val);

  void unique();
  void transfer(iterator pos, iterator first, iterator last);
  void splice(iterator pos, list & x);
  void splice(iterator pos, list &, iterator i);
  void splice(iterator pos, list & x, iterator first, iterator last);
protected:
  //此节点为基点，也是尾后节点
  using node_alloc = alloc_adapter<node_type, Alloc>;
  //传回一个节点大小内存空间
  node_type *get_node() { return node_alloc::allocate(); }
  void put_node(node_type *p) { node_alloc::deallocate(p,0); }
  template <typename... Args> node_type *creat_node(Args &&...args) {
    auto ret = get_node();
    construct(ret, std::forward<Args>(args)...);
    return ret;
  }
  node_type *creat_node(const T &val) {
    auto ret = get_node();
    construct(ret, val);
    return ret;
  }
  void destroy_node(node_type *p) {
    destroy(&(p->_data));
    put_node(p);
  }

  node_type *_base_node;

public:
  list() {
    _base_node = creat_node(T{});
    _base_node->_next = _base_node;
    _base_node->_pre = _base_node;
  }

  list(const std::initializer_list<T> & ls) : list()
  {
    for(auto it = ls.begin(); it != ls.end(); ++it)
    {
      push_back(*it);
    }
  }
};

/**
 * @brief 将i指向元素拼接在pos之前
 * 
 * @tparam T 
 * @tparam Alloc 
 * @param pos 
 * @param i 
 */
template <typename T, typename Alloc>
void list<T,Alloc>::splice(iterator pos, list &, iterator i)
{
  auto j = i;
  ++j;
  if(pos == i || pos == j) //自己或者本来就在前面
    return;
  transfer(pos, i, j);
}


/**
 * @brief 将[first,last)内的所有元素拼接在pos之前
 *        可以是一个list中，也可是不同的list中，但是pos不能位于[first.last)间
 * @tparam T 
 * @tparam Alloc 
 * @param pos 
 * @param first 
 * @param last 
 */
template <typename T, typename Alloc>
void list<T,Alloc>::splice(iterator pos, list &, iterator first, iterator last)
{
  if(first == last)
    return;
  transfer(pos, first, last);
}

/**
 * @brief 将x(list)拼接在pos之前
 * 
 * @tparam T 
 * @tparam Alloc 
 * @param pos 
 * @param x 
 */
template <typename T, typename Alloc>
void list<T,Alloc>::splice(iterator pos, list<T, Alloc> &x)
{
  if (x.empty())
    return;
  transfer(pos, x.begin(), x.end());
}
/**
 * @brief 将区间[first,last) 的节点移动到pos前, 
 *        可以是一个list中，也可是不同的list中，但是pos不能位于[first.last)间
 * 
 * @tparam T 
 * @tparam Alloc 
 * @param pos 
 * @param first 
 * @param last 
 */
template <typename T, typename Alloc>
void list<T,Alloc>::transfer(iterator pos, iterator first, iterator last)
{
  if(last == pos)
    return;
  last._node->_pre->_next = pos._node;
  first._node->_pre->_next = last._node;
  pos._node->_pre->_next = first._node;
  auto tmp = pos._node->_pre;
  pos._node->_pre = last._node->_pre;
  last._node->_pre = first._node->_pre;
  first._node->_pre = tmp;
}

/**
 * @brief 将连续且相同的元素变为1个
 * 
 * @tparam T 
 * @tparam Alloc 
 */
template <typename T, typename Alloc>
void list<T,Alloc>::unique()
{
  //if empty do nothing
  auto first = begin();
  auto last = end();
  while (first != last)
  {
    auto next = first;
    ++next;
    if(*first == *next)
      erase(next);
    else
      first = next;
  }
}

/**
 * @brief 删除所有值为val的节点
 * 
 * @tparam T 
 * @tparam Alloc 
 * @param val 
 */
template <typename T, typename Alloc>
void list<T,Alloc>::remove(const T &val)
{
  iterator first = begin();
  iterator last = end();
  while(first != last)
  {
    auto next = first;
    ++next;
    if(*first == val)
      erase(first);
    first = next;
  }
}
  
/**
 * @brief 将整个链表清空
 * 
 * @tparam T 
 * @tparam Alloc 
 */
template <typename T, typename Alloc>
void list<T,Alloc>::clear()
{
  auto current_node = _base_node->_next; //begin()._node
  while (current_node != _base_node)
  {
    auto tmp = current_node->_next;
    destroy_node(current_node);
    current_node = tmp;
  }
  _base_node->_next = _base_node;
  _base_node->_pre = _base_node;
}



/**
 * @brief 在pos指向的元素之前插入一个元素such as instert before pos
 * 
 * @tparam T 
 * @tparam Alloc 
 * @param pos 
 * @param val 
 * @return list<T, Alloc>::iterator 
 */
template <typename T, typename Alloc>
typename list<T, Alloc>::iterator
list<T, Alloc>::insert(iterator pos, const T &val)
{
  node_type *node = creat_node(val);
  node->_pre = pos._node->_pre;
  node->_next = pos._node;
  pos._node->_pre->_next = node;
  pos._node->_pre = node;
  return iterator(node);
}

/**
 * @brief 删除pos指向的元素,如果为空链表，则抛出一个logic_error, 如果pos不合法，是一个未定义的行为
 * 
 * @tparam T 
 * @tparam Alloc 
 * @param pos 
 * @return list<T,Alloc>::iterator 
 */
template <typename T, typename Alloc>
typename list<T,Alloc>::iterator
list<T,Alloc>::erase(iterator pos)
{
  if(empty())
    throw std::logic_error("list is empty");
  auto ret = pos._node->_next;
  pos._node->_pre->_next = pos._node->_next;
  pos._node->_next->_pre = pos._node->_pre;
  pos._node->_next = nullptr;
  pos._node->_pre = nullptr;
  destroy_node(pos._node);
  return iterator(ret);
}



} // namespace jan

#endif
