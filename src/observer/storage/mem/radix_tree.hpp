#pragma once

#include "common/lang/string_view.h"
#include "common/log/log.h"
#include "common/sys/rc.h"
#include <cstdint>

namespace oceanbase {

template <typename V>
struct RadixNode
{
  static constexpr size_t PREFIX_LENGTH = 16;

  char   prefix[PREFIX_LENGTH];
  size_t prefix_size{0};

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

  V            *v = nullptr;
  RadixNode<V> *children[256];
  size_t        children_count = 0;

  inline bool has_child(char ch) const { return children[static_cast<uint8_t>(ch)] != nullptr; }

  inline void add_child(char ch, RadixNode<V> *child)
  {
    children[static_cast<uint8_t>(ch)] = child;
    children_count++;
  }

  inline RadixNode<V> *&at(char ch) { return children[static_cast<uint8_t>(ch)]; }

  RadixNode() { bzero(children, sizeof(RadixNode<V> *) * 256); }

  /**
   * @return return the top and end of radix node. If the size of key is greater
   * than PREFIX_LEN, this function will create nodes which is more than one.
   */
  static inline std::pair<RadixNode<V> *, RadixNode<V> *> make_nodes(std::string_view key)
  {
    ASSERT(key.size(), "");
    auto   top        = new RadixNode<V>();
    size_t prefix_len = top->set_prefix(key);
    key               = key.substr(prefix_len);
    auto end          = top;
    while (key.size()) {
      auto tmp   = new RadixNode<V>();
      prefix_len = tmp->set_prefix(key);
      key        = key.substr(prefix_len);
      end->add_child(tmp->get_prefix()[0], tmp);
      end = tmp;
    }
    return {top, end};
  }

  static void destory_node(RadixNode<V> *node)
  {
    if (node == nullptr)
      return;

    for (auto &child : node->children) {
      if (child != nullptr) {
        destory_node(child);
      }
    }

    delete node->v;
    delete node;
  }
};

template <typename V>
class RadixTree
{
public:
  RadixTree() { root_ = new RadixNode<V>{}; }
  ~RadixTree() { RadixNode<V>::destory_node(root_); }

  RC   search(string_view key, V &val);
  V   *remove(string_view key);
  void put(string_view key, V val);

private:
  V *recursive_remove(RadixNode<V> *parent, RadixNode<V> *node, string_view key);
  /**
   * @brief return the prefix length of str1 and str2
   */
  std::pair<size_t, char> prefix_length(string_view str1, string_view str2)
  {
    const unsigned char *p1 = (unsigned char *)str1.data(), *p2 = (unsigned char *)str2.data();
    int                  n  = std::min(str1.size(), str2.size());
    char                 ch = *p1;
    int                  i  = 0;
    for (; i < n; i++) {
      if ((p1[i] ^ p2[i])) {
        break;
      }
      ch = *p1;
    }
    return {i, ch};
  }

  size_t        size_;
  RadixNode<V> *root_;
};

template <typename V>
void RadixTree<V>::put(string_view key, V val)
{
  auto parent = root_;
  auto node   = root_->children[(uint8_t)key[0]];
  while (!key.empty()) {
    if (node == nullptr) {
      auto [top, end] = RadixNode<V>::make_nodes(key);
      parent->add_child(key[0], top);
      end->v = new V{std::move(val)};
      return;
    }

    if (key.empty()) {
      node->v = new V{std::move(val)};
      return;
    }

    auto [matched_prefix_length, latest_matched_ch] = prefix_length(node->get_prefix(), key);

    if (key.size() == matched_prefix_length) {
      if (matched_prefix_length == node->prefix_size) {
        // totally matched
        delete node->v;
        node->v = new V{std::move(val)};
      } else {
        // node: bcde , insert bc
        // old node: de,
        string_view node_prefix       = node->prefix_substr(matched_prefix_length);
        string_view new_parent_prefix = node->prefix_substr(0, matched_prefix_length);
        auto        new_parent        = new RadixNode<V>{};
        new_parent->set_prefix(new_parent_prefix);
        node->set_prefix(node_prefix);

        new_parent->add_child(node->prefix[0], node);
        new_parent->v = new V{std::move(val)};
        // replace
        parent->add_child(new_parent->prefix[0], new_parent);
      }
      return;
    } else {
      if (node->prefix_size == matched_prefix_length) {
        // node: bc, insert bcde
        key     = key.substr(matched_prefix_length);
        char ch = key[0];
        parent  = node;
        node    = node->children[static_cast<uint8_t>(ch)];
      } else {
        // split
        // node: bcef, insert bcde
        // node -> ef, new node -> bc, new node -> de
        string_view new_parent_key = node->prefix_substr(0, matched_prefix_length);
        string_view new_child_key  = key.substr(matched_prefix_length);
        // printf("%s, %s, %s\n", node_key.c_str(), new_parent_key.c_str(),
        // new_child_key.c_str());
        auto new_parent = new RadixNode<V>{};
        new_parent->set_prefix(new_parent_key);

        auto [new_children_top, new_children_end] = RadixNode<V>::make_nodes(new_child_key);
        new_children_end->v                       = new V{std::move(val)};
        node->set_prefix_substr(matched_prefix_length);

        new_parent->add_child(new_children_top->prefix[0], new_children_top);
        new_parent->add_child(node->prefix[0], node);
        // replace
        parent->add_child(new_parent->prefix[0], new_parent);
        return;
      }
    }
  }
}

template <typename V>
V *RadixTree<V>::remove(string_view key)
{
  if (key.empty()) {
    return nullptr;
  }
  return recursive_remove(root_, root_->children[static_cast<uint8_t>(key[0])], key).first;
}

template <typename V>
V *RadixTree<V>::recursive_remove(RadixNode<V> *parent, RadixNode<V> *node, string_view key)
{
  if (node == nullptr || key.empty()) {
    return {nullptr, false};
  }

  auto [matched_prefix_length, latest_matched_ch] = prefix_length(node->subkey2string(), key);
  if (matched_prefix_length == key.size()) {
    if (node->v == nullptr) {
      return {nullptr, false};
    }
    auto res   = node->v;
    bool clean = false;
    node->v    = nullptr;
    if (node->children_count == 0) {
      clean = true;
    }
    return {res, clean};
  } else {
    key                       = key.substr(matched_prefix_length);
    auto [res, child_deleted] = recursive_remove(node, node->children[static_cast<uint8_t>(key[0])], key);
    if (child_deleted) {
      node->children_count -= 1;
      child_deleted          = node->children_count == 0;
      node->children[static_cast<uint8_t>(key[0])] = nullptr;
    }
    return {res, child_deleted};
  }
}

template <typename V>
RC RadixTree<V>::search(string_view key, V &val)
{
  if (key.empty()) {
    return RC::NOT_EXIST;
  }
  auto node = root_->children[static_cast<uint8_t>(key[0])];
  while (key.size() && node) {
    string_view prefix              = node->get_prefix();
    auto [matched_prefix_length, _] = prefix_length(key, prefix);
    if (matched_prefix_length == key.size()) {
      if (matched_prefix_length == node->prefix_size && node->v != nullptr) {
        val = *node->v;
        return RC::SUCCESS;
      }
      return RC::NOT_EXIST;
    } else if (matched_prefix_length == node->prefix_size) {
      char ch = key.at(matched_prefix_length);
      key     = key.substr(matched_prefix_length);
      node    = node->children[static_cast<uint8_t>(ch)];
    } else {
      return RC::NOT_EXIST;
    }
  }

  if (node) {
    if (node->v) {
      val = *node->v;
      return RC::SUCCESS;
    }
  }
  return RC::NOT_EXIST;
}

}  // namespace oceanbase