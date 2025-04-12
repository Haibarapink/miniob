// /* Copyright (c) 2021 OceanBase and/or its affiliates. All rights reserved.
// miniob is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details. */

// #include "gtest/gtest.h"

// #include "common/lang/filesystem.h"
// #include "common/lang/thread.h"
// #include "common/lang/utility.h"
// #include "storage/mem/radix_tree.hpp"

// using namespace oceanbase;


// class KeyValueGenerator
// {
// public:
//   static vector<pair<string, string>> generate_data(size_t count)
//   {
//     vector<pair<string, string>> data;
//     for (size_t i = 0; i < count; ++i) {
//       data.emplace_back("key" + to_string(i), "value" + to_string(i));
//     }
//     return data;
//   }
// };

// class RadixTestBase : public ::testing::TestWithParam<size_t>
// {
// protected:
//   RadixTree<string>       *db;

//   void SetUp() override
//   {
//     db = new RadixTree<string>;
//   }

//   void set_up_options()
//   {
//   }

//   void TearDown() override { delete db; }
// };