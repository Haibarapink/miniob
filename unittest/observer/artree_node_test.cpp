#include "storage/mem/radix/radix_tree_inner_node.hpp"
#include "storage/mem/radix/radix_tree_node.hpp"
#include "gtest/gtest.h"

using namespace oceanbase;


std::string random_string(int length) {
    const std::string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string randomString;
    srand(static_cast<unsigned int>(time(nullptr)));

    for (int i = 0; i < length; ++i) {
        int randomIndex = rand() % characters.length();
        randomString += characters[randomIndex];
    }

    return randomString;
}

TEST(ARTreeNodeTest, make_nodes_test) {

    int len = ARTreeNode<int>::PREFIX_LENGTH * 3 + 1;
    // 应该生成四个节点
    string str = random_string(len);
    auto [top, end] = ARTreeNode<int>::make_nodes(str);
    size_t count = 0;
    auto runner = top;
    size_t str_idx = 0;
    while (runner != end) {
        count += 1;
        str_idx += runner->prefix_size;
        char ch = str[str_idx];
        runner = runner->at(ch);
    }
    count++;

    EXPECT_EQ(count, 4);

    ARTreeNode<int>::destory_node(top);

}


int main(int argc, char **argv)
{

  // 分析gtest程序的命令行参数
  testing::InitGoogleTest(&argc, argv);

  // 调用RUN_ALL_TESTS()运行所有测试用例
  // main函数返回RUN_ALL_TESTS()的运行结果
  return RUN_ALL_TESTS();
}