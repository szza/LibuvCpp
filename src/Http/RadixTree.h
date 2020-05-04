#pragma once 
// from https://github.com/wlgq2/uv-cpp

#include "src/Http/Internel.h"
#include<memory>


namespace uv {
namespace http {

auto commonPrefix 
          = [](const std::string& str1, const std::string& str2) ->size_t
            {
                size_t i=0; 
                for(; i < str1.length() && i < str2.length(); ++i) { 
                if(str1[i] != str2[i]) 
                    break;
                }
                return i;
            };

template<typename T> class TreeNode;
template<typename T> using TreeNodePtr = std::shared_ptr<TreeNode<T>>;

template<typename T>
struct TreeNode {
    bool empty;
    std::string key;
    TreeNodePtr<T> child;
    TreeNodePtr<T> next;
    T value;

    TreeNode(bool empty, const std::string& key, TreeNodePtr<T> child, TreeNodePtr<T> next, T value)
    : empty(empty),
      key(key),
      child(child),
      next(next),
      value(value)
    { }
};

template<typename T>
class RadixTree {
public:
    RadixTree()
    : root_(nullptr)
    { }
  
    void set(const std::string& key, T value) 
    { 
      if (nullptr == root_)
      {
          root_ = std::make_shared<TreeNode<T>>(false, key, nullptr, nullptr, value);
      }
      else
      {
        setNode(root_, key, value);
      }
    }

    void set(std::string&& key, T value) 
    { set(key, value); }

    bool get(const std::string& key, T& value)
    { return getNode(root_, key, value);}

    bool get(std::string&& key, T& value)      
    { return get(key, value);}

    TreeNodePtr<T> begin() const 
    { return root_; }

    constexpr static const char WildCard = '*';

private:
    TreeNodePtr<T> root_;

    void setNode(TreeNodePtr<T>& node, const std::string& key, const T& value) { 
      auto commonLength = commonPrefix(node->key, key);
      //相同长度为0，递归next节点。
      if (commonLength == 0)
      {
          //next节点为空，则插入新节点
          if (nullptr == node->next)
          {
              node->next = 
                std::make_shared<TreeNode<T>>(false, key, nullptr, nullptr, value );
              return;
          }
          //否则递归next节点
          setNode(node->next, key, value);
      }
      //相同长度小于节点key长度，则拆分节点
      else if (commonLength < node->key.size())
      {
          std::string key1(node->key, 0, commonLength);
          auto childNode = 
            std::make_shared<TreeNode<T>>(node->empty,
                                        std::string(node->key, commonLength, node->key.size() - commonLength),
                                        node->child,
                                        nullptr,
                                        node->value);
          node->empty = true;
          node->key.swap(key1);
          node->child = childNode;
          //如果共同长度等于新增节点则直接赋值
          if (key.size() == commonLength)
          {
              node->empty = false;
              node->value = value;
          }
          else
          {
              childNode->next
                = std::make_shared<TreeNode<T>>(false,
                                                std::string(key, commonLength,key.size()-commonLength),
                                                nullptr,
                                                nullptr,
                                                value);
          }
      }
      //相同长度等于节点key长度，则递归child节点
      else
      {
          //key和node->key相等，则直接赋值；
          if (commonLength == key.size())
          {
              node->empty = false;
              node->value = value;
          }
          else //否则，则拆分key，递归子节点
          {
              std::string key1(key, commonLength, key.size()-commonLength);
              //子节点为空，直接插入
              if (nullptr == node->child)
              {
                  node->child = 
                    std::make_shared<TreeNode<T>>(false, key1, nullptr, nullptr, value);
              }
              else //否则递归子节点
              {
                  setNode(node->child, key1, value);
              }
          }
      }
    }

    bool getNode(TreeNodePtr<T>& node, const std::string& key, T& value) { 
      auto commonLength = commonPrefix(node->key, key);
      //通配符判定
      if (commonLength == node->key.size() - 1)
      {
          if (node->key.back() == WildCard)
          {
              value = node->value;
              return true;
          }
      }
      //相同长度为0，递归next节点。
      if (commonLength == 0)
      {
          //next节点为空，则未找到该Key
          if (nullptr == node->next)
          {
              return false ;
          }
          //否则递归next节点
          return getNode(node->next, key, value);
      }
      //相同长度小于节点key长度，则未找到该key
      else if (commonLength < node->key.size())
      {
          return false;
      }
      //相同长度等于节点key长度，则递归child节点
      else
      {
          //key和node->key相等，则直接返回value。
          if (commonLength == key.size())
          {
              if (!node->empty)
              {
                  value = node->value;
                  return true;
              }
              //空节点
              return false;
          }
          else //否则，则拆分key，递归子节点
          {
              //子节点为空，未找到该key
              if (nullptr == node->child)
              {
                  return false;
              }
              else //否则递归子节点
              {
                  std::string key1(key, commonLength, key.size() - commonLength);
                  return getNode(node->child, key1, value);
              }
          }
      }
    }
}; // class RadixTree

} // namespace http
}