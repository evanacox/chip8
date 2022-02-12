//======---------------------------------------------------------------======//
//                                                                           //
// Copyright 2022 Evan Cox <evanacox00@gmail.com>. All rights reserved. //
//                                                                           //
// Use of this source code is governed by a BSD-style license that can be    //
// found in the LICENSE.txt file at the root of this project, or at the      //
// following link: https://opensource.org/licenses/BSD-3-Clause              //
//                                                                           //
//======---------------------------------------------------------------======//

#include "./vm.h"
#include "./debug.h"
#include <bit>
#include <random>

namespace {
  constexpr std::uint16_t nth_mask(int n) {
    return 0xFU << 4 * (4 - n);
  }

  std::random_device rd{};
} // namespace

namespace chip8 {
  VM::VM() {
    std::seed_seq seq{rd()};

    rng_.seed(seq);
  }

  void VM::next() noexcept {
    // instructions with one unique bit pattern
    if (current() == 0x00E0U) { // display-clear: 0x00E0
      display_.clear();
    } else if (current() == 0x00EEU) { // return: 0x00EE
      pc_ = pop();
    }

    // instructions that have their own unique most-significant byte
    switch (current() & 0xF000U) {
      case 0x2000U: // call nnn: 0x2nnn
        push(pc_);
        pc_ = bottom_12bit();
        return;
      case 0x1000U: // jump to nnn: 0x1nnn
        pc_ = bottom_12bit();
        return;
      case 0x3000U: { // skip next instruction if Vx == nn: 0x3xnn
        auto vx = reg_[nth_4bit(2)];
        skip_if(vx == bottom_8bit());
        break;
      }
      case 0x4000U: { // skip next instruction if Vx != nn: 0x4xnn
        auto vx = reg_[nth_4bit(2)];
        skip_if(vx != bottom_8bit());
        break;
      }
      case 0x5000U: { // skip next instruction if Vx == Vy: 0x5xy0
        auto vx = reg_[nth_4bit(2)];
        auto vy = reg_[nth_4bit(3)];
        skip_if(vx == vy);
        break;
      }
      case 0x6000U: // set Vx to nn: 0x6xnn
        reg_[nth_4bit(2)] = bottom_8bit();
        break;
      case 0x7000U: // increment Vx by nn: 0x7xnn
        reg_[nth_4bit(2)] += bottom_8bit();
        break;
      case 0x9000U: { // skip next instruction if Vx != Vy: 0x9xy0
        auto vx = reg_[nth_4bit(2)];
        auto vy = reg_[nth_4bit(3)];
        skip_if(vx != vy);
        break;
      }
      case 0xA000U: // sets I to nnn: 0xAnnn
        i_ = bottom_12bit();
        break;
      case 0xB000U:
        pc_ = reg(v0) + bottom_12bit();
        return;
      default:
        chip8::errs() << "unsupported instruction! opcode: `" << chip8::instr << current();
        chip8::outs() << "ignoring unknown instruction...";
        break;
    }

    next_instruction();
  }

  std::uint8_t VM::reg(std::size_t index) const noexcept {
    return reg_[index];
  }

  std::uint16_t VM::pop() noexcept {
    auto value = peek();

    stack_[--sp_] = 0;

    return value;
  }

  std::uint16_t VM::peek() noexcept {
    return stack_[sp_ - 1];
  }

  void VM::push(std::uint16_t address) noexcept {
    stack_[sp_++] = address;
  }

  std::uint8_t VM::rand_next() noexcept {
    // `minstd_rand` isn't *terribly* slow per number generated.
    // at least on gcc/clang it's just 2-3 multiplies and some shift/adds.
    // msvc lets a divide slip in, but it's fine for this purpose
    return std::uniform_int_distribution<int>(0, 255)(rng_);
  }

  void VM::skip_if(bool condition) noexcept {
    if (condition) {
      next_instruction();
    }
  }

  void VM::next_instruction() noexcept {
    pc_ += 2;
  }

  std::uint16_t VM::current() const noexcept {
    static_assert(std::endian::native == std::endian::little, "what kind of platform are you using?");

    return static_cast<std::uint16_t>(memory_[pc_] << 8U | memory_[pc_ + 1]);
  }

  std::uint8_t VM::nth_4bit(int n) const noexcept {
    // extract bits that are zeroed in mask, shift down to bottom 4 bits
    return static_cast<std::uint8_t>((current() & nth_mask(n)) >> (4 * (4 - n)));
  }

  std::uint16_t VM::bottom_12bit() const noexcept {
    return static_cast<std::uint16_t>(current() & 0x0FFFU);
  }

  std::uint8_t VM::bottom_8bit() const noexcept {
    return static_cast<std::uint8_t>(current() & 0xFFU);
  }

  std::uint8_t VM::top_8bit() const noexcept {
    return static_cast<std::uint8_t>((current() & 0xFF00U) >> 8);
  }

} // namespace chip8