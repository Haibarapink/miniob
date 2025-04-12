#pragma once

#include "common/lang/string_view.h"
#include "common/log/log.h"
#include "common/sys/rc.h"
#include <cstdint>

namespace oceanbase {

class ARTreeInnerNodeIterator;

class ARTreeInnerNode
{
public:
  ARTreeInnerNode()          = default;
  virtual ~ARTreeInnerNode() = default;

  virtual ARTreeInnerNode         *get_child(unsigned char ch)                         = 0;
  virtual bool                     has_child(unsigned char ch)                         = 0;
  virtual bool                     add_child(unsigned char ch, ARTreeInnerNode *child) = 0;
  virtual void                     remove_child(unsigned char ch)                      = 0;
  virtual ARTreeInnerNodeIterator *get_iterator()                                      = 0;

  virtual ARTreeInnerNode *grow() { return nullptr; }
  virtual bool             is_full() { return false; }

  size_t size{0};
  void  *node_ref;
};

template <size_t n>
class ARTreeNode4Or16 : public ARTreeInnerNode
{
  friend class ARTreeInnerNode4Or16Iterator;

public:
  ARTreeNode4Or16()
  {
    for (size_t i = 0; i < n; ++i) {
      key_[i]      = 0;
      children_[i] = nullptr;
    }
  }

  ~ARTreeNode4Or16() override
  {
    for (size_t i = 0; i < n; ++i) {
      if (children_[i]) {
        delete children_[i];
      }
    }
  }

  ARTreeInnerNode *grow() override { return nullptr; }

  ARTreeInnerNode *get_child(unsigned char ch) override
  {
    for (size_t i = 0; i < n; ++i) {
      if (key_[i] == ch && children_[i]) {
        return children_[i];
      }
    }
    return nullptr;
  }

  bool is_full() override { return size == n; }

  bool has_child(unsigned char ch) override
  {
    for (size_t i = 0; i < n; ++i) {
      if (key_[i] == ch && children_[i] != nullptr) {
        return true;
      }
    }
    return false;
  }

  bool add_child(unsigned char ch, ARTreeInnerNode *child) override
  {
    for (size_t i = 0; i < n; ++i) {
      if (key_[i] == ch && children_[i] != nullptr) {
        return false;
      }
      if (children_[i] == nullptr) {
        key_[i]      = ch;
        children_[i] = child;
        size++;
        return true;
      }
    }
    return false;
  }

  void remove_child(unsigned char ch) override
  {
    for (size_t i = 0; i < n; ++i) {
      if (key_[i] == ch) {
        if (children_[i]) {
          delete children_[i];
        }
        children_[i] = nullptr;
        return;
      }
    }
  }

  ARTreeInnerNodeIterator *get_iterator() override;

private:
  unsigned char    key_[n];
  ARTreeInnerNode *children_[n];
};

class ARTreeNode48 : public ARTreeInnerNode
{
  friend class ARTreeInnerNode48Iterator;

public:
  ARTreeNode48()
  {
    std::fill(std::begin(child_exists_), std::end(child_exists_), false);
    std::fill(std::begin(child_index_), std::end(child_index_), 0);
    std::fill(std::begin(children_), std::end(children_), nullptr);
  }

  ~ARTreeNode48() override
  {
    for (auto *child : children_) {
      if (child) {
        delete child;
      }
    }
  }

  ARTreeInnerNode *get_child(unsigned char ch) override
  {
    if (!child_exists_[ch])
      return nullptr;
    return children_[child_index_[ch]];
  }

  bool is_full() override { return size == 48; }

  bool has_child(unsigned char ch) override { return child_exists_[ch]; }

  bool add_child(unsigned char ch, ARTreeInnerNode *child) override
  {
    if (child_exists_[ch]) {
      return false;
    }

    for (uint8_t i = 0; i < 48; ++i) {
      if (children_[i] == nullptr) {
        child_exists_[ch] = true;
        child_index_[ch]  = i;
        children_[i]      = child;
        size++;
        return true;
      }
    }

    return false;
  }

  void remove_child(unsigned char ch) override
  {
    if (!child_exists_[ch])
      return;

    uint8_t idx = child_index_[ch];
    delete children_[idx];
    children_[idx]    = nullptr;
    child_exists_[ch] = false;
  }

  ARTreeInnerNode         *grow() override;
  ARTreeInnerNodeIterator *get_iterator() override;

private:
  bool             child_exists_[256];
  uint8_t          child_index_[256];
  ARTreeInnerNode *children_[48];
};

class ARTreeNode256 : public ARTreeInnerNode
{
  friend class ARTreeInnerNode256Iterator;

public:
  ARTreeNode256() { std::fill(std::begin(children_), std::end(children_), nullptr); }

  ~ARTreeNode256() override
  {
    for (auto *child : children_) {
      if (child) {
        delete child;
      }
    }
  }

  ARTreeInnerNode *get_child(unsigned char ch) override
  {
    if (children_[ch]) {
      return children_[ch];
    }

    return nullptr;
  }

  bool has_child(unsigned char ch) override { return children_[ch] != nullptr; }

  bool add_child(unsigned char ch, ARTreeInnerNode *child) override
  {
    if (children_[ch] != nullptr) {
      return false;
    }
    size++;
    children_[ch] = child;
    return true;
  }

  void remove_child(unsigned char ch) override
  {
    if (children_[ch]) {
      delete children_[ch];
    }
    children_[ch] = nullptr;
  }

  ARTreeInnerNodeIterator *get_iterator() override;

private:
  ARTreeInnerNode *children_[256];
};

inline ARTreeInnerNode *ARTreeNode48::grow()
{
  ARTreeInnerNode *node = new ARTreeNode256;
  for (size_t i = 0; i < 256; ++i) {
    if (child_exists_[i]) {
      auto child_idx = child_index_[i];
      node->add_child(i, children_[child_idx]);
      child_exists_[i]     = false;
      children_[child_idx] = nullptr;
    }
  }
  return node;
}

/// 模板特化
template <>
inline ARTreeInnerNode *ARTreeNode4Or16<4>::grow()
{
  ARTreeNode4Or16<16> *node16 = new ARTreeNode4Or16<16>;
  for (size_t i = 0; i < 4; ++i) {
    if (children_[i]) {
      node16->add_child(key_[i], children_[i]);
      children_[i] = nullptr;
    }
  }
  return node16;
}

template <>
inline ARTreeInnerNode *ARTreeNode4Or16<16>::grow()
{
  ARTreeNode48 *node48 = new ARTreeNode48;
  for (size_t i = 0; i < 16; ++i) {
    if (children_[i]) {
      node48->add_child(key_[i], children_[i]);
      children_[i] = nullptr;
    }
  }
  return node48;
}

using ARTreeNode4  = ARTreeNode4Or16<4>;
using ARTreeNode16 = ARTreeNode4Or16<16>;
}  // namespace oceanbase