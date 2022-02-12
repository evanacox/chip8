//======---------------------------------------------------------------======//
//                                                                           //
// Copyright 2022 Evan Cox <evanacox00@gmail.com>. All rights reserved. //
//                                                                           //
// Use of this source code is governed by a BSD-style license that can be    //
// found in the LICENSE.txt file at the root of this project, or at the      //
// following link: https://opensource.org/licenses/BSD-3-Clause              //
//                                                                           //
//======---------------------------------------------------------------======//

#include "./driver.h"
#include <string_view>
#include <vector>

int main(int argc, char** argv) {
  std::vector<std::string_view> args;
  args.reserve(argc);

  for (auto i = 0; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }

  return chip8::entry(args);
}