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
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <vector>

namespace chip8 {
  namespace chrono = std::chrono;

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
  inline constexpr std::size_t chip8_width = 64;
  inline constexpr std::size_t chip8_height = 32;
  inline constexpr std::size_t chip8_pixels = chip8_height * chip8_width;

  class VM final {
  public:
    explicit VM(Display* display);

    // load a program into memory
    void load(std::span<std::uint8_t> program) noexcept;

    // executes a single instruction
    void cycle() noexcept;

  private:
    // reads the big-endian current instruction, converts to little-endian
    [[nodiscard]] std::uint16_t current() const noexcept;

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

    // actually executes an instruction
    void execute(std::uint16_t inst) noexcept;

    // handles specifically executing 0x8nnn instructions
    void execute_opcode_8(std::uint16_t inst) noexcept;

    // handles specifically executing 0xFnnn instructions
    void execute_opcode_f(std::uint16_t inst) noexcept;

    // performs wrapping 8-bit addition, updates VF on overflow
    std::uint8_t wrapping_add(std::uint8_t a, std::uint8_t b) noexcept; // NOLINT(bugprone-easily-swappable-parameters)

    // performs wrapping 8-bit subtraction, updates VF on overflow
    std::uint8_t wrapping_sub(std::uint8_t a, std::uint8_t b) noexcept; // NOLINT(bugprone-easily-swappable-parameters)

    std::uint16_t pc_ = 0x200; // program counter
    std::uint16_t i_ = 0;      // address register
    std::uint16_t sp_ = 0;     // stack pointer. `stack_[sp_ - 1]` == current, `stack_[sp_]` is next
    std::uint8_t delay_ = 0;
    std::uint8_t sound_ = 0;
    std::minstd_rand rng_;
    chrono::steady_clock::time_point last_cycle_ = chrono::steady_clock::now(); // last cycle update
    chrono::steady_clock::time_point last_hz_ = chrono::steady_clock::now();    // last delay/sound update
    std::array<std::uint8_t, 16> reg_ = {};
    std::array<std::uint8_t, chip8_pixels / 8> display_state_ = {};
    std::array<std::uint16_t, 128> stack_ = {};
    std::array<std::uint8_t, 4096> memory_ = {};
    Display* display_;
  };
} // namespace chip8