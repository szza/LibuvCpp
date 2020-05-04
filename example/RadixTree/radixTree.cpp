
#include "src/libuvCpp.h"
#include <random>
#include <map>

using namespace uv;
using namespace http;

template<typename T>
struct KV
{
    std::string key;
    T  value;
};

template<typename T>
bool TestRand(RadixTree<T>& tree,uint64_t cnt)
{
    //生成Key值不相同若干个kv。
    std::vector<struct KV<T>> kvs;
    for (uint64_t i = 0;i < cnt; i++)
    {
        struct KV<T> kv{ std::to_string(i),i };
        kvs.push_back(kv);
    }

    //随机顺序取出元素插入radix tree.
    std::map<std::string, T> kvMap;
    std::default_random_engine random;
    std::uniform_int_distribution<int> dis(0, cnt);
    while (!kvs.empty())
    {
        int index = dis(random) % kvs.size();
        auto kv = kvs[index];
        tree.set(std::forward<std::string>(kv.key), std::forward<uint64_t>(kv.value));
        kvs[index] = kvs.back();
        kvs.pop_back();
        LOG_INFO << "{ " << kv.key << "," << kv.value << "}";
        kvMap[kv.key] = kv.value;
    }

    //遍历对比kvmap与radix tree值
    for (auto& kv : kvMap)
    {
        T value;
        std::string key = kv.first;
        if (!tree.get(key, value))
        {
            //未找到该key
            LOG_INFO << "not find key " << kv.first ;
            return false;
        }
        if (value != kv.second)
        {
            //value值不正确
            LOG_INFO << "error key " << kv.first << " value " << kv.second << " but " << value ;
            return false;
        }

    }
    return true;

}
int main(int argc, char** args)
{
    nanolog::set_log_level(nanolog::LogLevel::INFO);
    nanolog::initialize(nanolog::NonGuaranteedLogger(10), 
                         "/media/szz/Others/Self_study/Cpp/MyPro/LibuvCpp/log/",
                         "radix",
                         1);
    LOG_INFO << "wait ...";
    int cnt = 1000;
    RadixTree<uint64_t> tree;
    //生成cnt个kv并随机顺序插入检测。
    if (TestRand(tree, cnt))
    {
        LOG_INFO << "test success:" << cnt;
    }
}

//遍历节点
template<typename T>
void findAll(TreeNodePtr<T> node, std::string& key)
{
    if (nullptr == node)
    {
        return;
    }
    auto nodeKey = key + node->key;
    if (!node->isEmpty)
    {
        LOG_INFO << nodeKey << ":" << node->value;
    }
    else
    {
        LOG_INFO << nodeKey << ": empty";
    }
    findAll(node->child, nodeKey);
    findAll(node->next, key);
}
