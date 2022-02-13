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
#include "./display.h"
#include "./vm.h"
#include <chrono>
#include <fstream>
#include <iostream>

using namespace std::literals;

namespace {
  std::vector<std::uint8_t> read_bytes(std::string_view file) noexcept {
    auto ifs = std::ifstream(file.data(), std::ios::binary);
    auto data = std::vector<std::uint8_t>{};
    ifs.unsetf(std::ios::skipws);

    if (!ifs.is_open()) {
      chip8::errs() << "unable to open file '" << file << "'! exiting...";

      std::abort();
    }

    ifs.seekg(0, std::ios::end);
    auto size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    data.reserve(static_cast<std::size_t>(size));

    data.insert(data.begin(), std::istream_iterator<std::uint8_t>(ifs), std::istream_iterator<std::uint8_t>());

    return data;
  }
} // namespace

int chip8::entry(std::span<std::string_view> args) noexcept {
  auto display = chip8::Display();
  auto vm = chip8::VM(&display);
  auto bytes = read_bytes(args.front());

  vm.load(bytes);

  auto last = chrono::system_clock::now();

  while (display.is_open()) {
    vm.cycle();

    // actually enabling vsync in SFML seems to break everything, but I don't want to be
    // drawing constantly for no reason
    if (auto now = chrono::system_clock::now(); now >= last + 16666us) {
      last = now;

      display.cycle();
    }
  }

  return 0;
}