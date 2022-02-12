//======---------------------------------------------------------------======//
//                                                                           //
// Copyright 2022 Evan Cox <evanacox00@gmail.com>. All rights reserved. //
//                                                                           //
// Use of this source code is governed by a BSD-style license that can be    //
// found in the LICENSE.txt file at the root of this project, or at the      //
// following link: https://opensource.org/licenses/BSD-3-Clause              //
//                                                                           //
//======---------------------------------------------------------------======//

#include "./debug.h"
#include <iomanip>
#include <iostream>

namespace internal = chip8::internal;

internal::NewlineOstream chip8::outs() noexcept {
  return internal::NewlineOstream{&std::cout};
}

internal::NewlineOstream chip8::errs() noexcept {
  return internal::NewlineOstream{&std::cerr};
}

std::ostream& chip8::reg(std::ostream& stream) noexcept {
  return stream << std::setfill('0') << std::setw(2) << std::hex;
}

std::ostream& chip8::instr(std::ostream& stream) noexcept {
  return stream << std::setfill('0') << std::setw(4) << std::hex;
}
