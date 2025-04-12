// /* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
// miniob is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details. */

// #include "storage/mem/radix_tree.hpp"
// #include "gtest/gtest.h"
// #include <map>
// #include <string_view>

// using namespace oceanbase;

// template <typename T>
// std::string integeral_to_bytes(const T &t)
// {
//   static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value,
//       "Type must be an integral or floating point type.");
//   std::string result(sizeof(T), '\0');
//   memcpy(&result[0], &t, sizeof(T));
//   return result;
// }

// TEST(ARTreeNode, IntInsertTest)
// {
//   RadixTree<int> tree;
//   map<int, int>  shadow;

//   for (int i = -100000; i < 100000; ++i) {
//     string str = integeral_to_bytes(i);
//     tree.put(string_view{str.data(), str.size()}, i);
//     shadow[i] = i;
//   }

//   for (auto i : shadow) {
//     int tmp;
//     RC  rc = tree.search(integeral_to_bytes(i.first), tmp);
//     EXPECT_EQ(rc, RC::SUCCESS);
//     EXPECT_EQ(i.second, tmp);
//   }
// }

// int main(int argc, char **argv)
// {

//   // 分析gtest程序的命令行参数
//   testing::InitGoogleTest(&argc, argv);

//   // 调用RUN_ALL_TESTS()运行所有测试用例
//   // main函数返回RUN_ALL_TESTS()的运行结果
//   return RUN_ALL_TESTS();
// }

int main() {
  
}