//======---------------------------------------------------------------======//
//                                                                           //
// Copyright 2022 Evan Cox <evanacox00@gmail.com>. All rights reserved. //
//                                                                           //
// Use of this source code is governed by a BSD-style license that can be    //
// found in the LICENSE.txt file at the root of this project, or at the      //
// following link: https://opensource.org/licenses/BSD-3-Clause              //
//                                                                           //
//======---------------------------------------------------------------======//

#pragma once

#include "./display.h"
#include <array>
#include <cstdint>
#include <random>
#include <vector>

namespace chip8 {
  inline constexpr std::size_t memory_size = 4096;
  inline constexpr std::size_t stack_starting_size = 256;
  inline constexpr std::size_t v0 = 0;
  inline constexpr std::size_t v1 = 1;
  inline constexpr std::size_t v2 = 2;
  inline constexpr std::size_t v3 = 3;
  inline constexpr std::size_t v4 = 4;
  inline constexpr std::size_t v5 = 5;
  inline constexpr std::size_t v6 = 6;
  inline constexpr std::size_t v7 = 7;
  inline constexpr std::size_t v8 = 8;
  inline constexpr std::size_t v9 = 9;
  inline constexpr std::size_t va = 10;
  inline constexpr std::size_t vb = 11;
  inline constexpr std::size_t vc = 12;
  inline constexpr std::size_t vd = 13;
  inline constexpr std::size_t ve = 14;
  inline constexpr std::size_t vf = 15;

  class VM final {
  public:
    explicit VM();

    // executes a single instruction
    void next() noexcept;

    // gets the nth register
    [[nodiscard]] std::uint8_t reg(std::size_t index) const noexcept;

  private:
    // pops a return address and returns it
    [[nodiscard]] std::uint16_t pop() noexcept;

    // views the current return address
    [[nodiscard]] std::uint16_t peek() noexcept;

    // pushes a return address
    void push(std::uint16_t address) noexcept;

    // gets a pseudo-random random number, changes RNG state
    std::uint8_t rand_next() noexcept;

    // skips 1 instruction if `condition` is true
    void skip_if(bool condition) noexcept;

    // moves the program counter to the next instruction
    void next_instruction() noexcept;

    // reads the big-endian current instruction, converts to little-endian
    [[nodiscard]] std::uint16_t current() const noexcept;

    // gets the Nth 4 bit chunk of `current()`
    //
    // given `0xabcd`, n for parts -> a = 1, b = 2, c = 3, d = 4
    [[nodiscard]] std::uint8_t nth_4bit(int n) const noexcept;

    // gets the bottom 12 bits of `current()`
    [[nodiscard]] std::uint16_t bottom_12bit() const noexcept;

    // gets the least-significant byte of `current()`
    [[nodiscard]] std::uint8_t bottom_8bit() const noexcept;

    // gets the most-significant byte of `current()`
    [[nodiscard]] std::uint8_t top_8bit() const noexcept;

    std::uint16_t pc_ = 0;                       // program counter
    std::uint16_t i_ = 0;                        // address register
    std::uint16_t sp_ = 0;                       // stack pointer. `stack_[sp_ - 1]` == current, `stack_[sp_]` is next
    std::minstd_rand rng_;                       // current RNG state. `minstd_rand` has a tiny amount of state
    std::array<std::uint8_t, 16> reg_ = {};      // zero out everything to start
    std::array<std::uint16_t, 128> stack_ = {};  // large 256-byte stack
    std::array<std::uint8_t, 4096> memory_ = {}; // size isn't changed after construction, but no `std::dyn_array`
    Display display_{};
  };
} // namespace chip8