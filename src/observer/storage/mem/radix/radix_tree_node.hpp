#pragma once

#include "common/lang/string_view.h"
#include "storage/mem/radix/radix_tree_inner_node.hpp"
#include "common/log/log.h"
#include "common/sys/rc.h"
#include <cstdint>
#include <string_view>

namespace oceanbase {

template <typename V>
class ARTreeNode
{
public:
  static constexpr size_t PREFIX_LENGTH = 16;

  char   prefix[PREFIX_LENGTH];
  size_t prefix_size{0};

  V     *v              = nullptr;
  size_t children_count = 0;

  inline std::string_view get_prefix() const { return {prefix, prefix_size}; }

  inline size_t set_prefix(std::string_view str)
  {
    size_t copy_len = std::min(str.size(), PREFIX_LENGTH);
    memcpy(prefix, str.data(), copy_len);
    prefix_size = copy_len;
    return copy_len;
  }

  inline void set_prefix_substr(size_t pos, size_t len)
  {
    ASSERT(pos + len <= prefix_size, "");
    memmove(prefix, prefix + pos, len);
    prefix_size = len;
  }

  inline void set_prefix_substr(size_t pos)
  {
    ASSERT(pos <= prefix_size, "");
    size_t len = prefix_size - pos;
    set_prefix_substr(pos, len);
  }

  inline std::string_view prefix_substr(size_t pos, size_t len)
  {
    ASSERT(pos + len <= prefix_size, "");
    return std::string_view{prefix + pos, len};
  }

  inline std::string_view prefix_substr(size_t pos)
  {
    ASSERT(pos <= prefix_size, "");
    size_t len = prefix_size - pos;
    return prefix_substr(pos, len);
  }

  inline bool has_child(char ch) { return inner_->has_child(ch); }

  inline void add_child(char ch, ARTreeNode<V> *child) { inner_->add_child(ch, child->inner_); }

  ARTreeNode<V> *at(char ch)
  {
    auto node = inner_->get_child(ch);
    if (node) {
      return static_cast<ARTreeNode<V> *>(node->node_ref);
    }
    return nullptr;
  }

  ARTreeNode()  = default;
  ~ARTreeNode() = default;

  static ARTreeNode<V> *make_node(string_view key)
  {
    auto res              = new ARTreeNode<V>;
    res->inner_           = new ARTreeNode4;
    res->inner_->node_ref = res;
    res->set_prefix(key);
    return res;
  }

  /**
   * @return return the top and end of radix node. If the size of key is greater
   * than PREFIX_LEN, this function will create nodes which is more than one.
   */
  static std::pair<ARTreeNode<V> *, ARTreeNode<V> *> make_nodes(std::string_view key)
  {
    ASSERT(key.size(), "");
    auto   top        = make_node(key);
    size_t prefix_len = top->prefix_size;
    key               = key.substr(prefix_len);
    auto end          = top;
    while (key.size()) {
      auto tmp   = make_node(key);
      prefix_len = tmp->prefix_size;
      key        = key.substr(prefix_len);
      end->add_child(tmp->get_prefix()[0], tmp);
      end = tmp;
    }
    return {top, end};
  }

  static void destory_node(ARTreeNode<V> *node)
  {
    if (node == nullptr)
      return;

    for (auto &child :) {
      if (child != nullptr) {
        destory_node(child);
      }
    }

    delete node->v;
    delete node->inner_;
    delete node;
  }

private:
  ARTreeInnerNode *inner_;
};

}  // namespace oceanbase