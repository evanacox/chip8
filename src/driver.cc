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
#include "./debug.h"

int chip8::entry(std::span<std::string_view> args) noexcept {
  for (auto arg : args) {
    chip8::outs() << arg;
  }

  return 0;
}