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
#include "./bit.h"
#include "./debug.h"
#include <algorithm>
#include <bit>
#include <bitset>
#include <chrono>
#include <random>
#include <utility>

using namespace std::literals;

namespace {
  std::random_device rd{};

  // clang-format off
  std::array<std::uint8_t, 80> chip8_fontset = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
  };
  // clang-format on
} // namespace

namespace chip8 {
  VM::VM(Display* display) : display_{display} {
    std::seed_seq seq{rd()};

    rng_.seed(seq);

    // load fontset
    std::copy(chip8_fontset.begin(), chip8_fontset.end(), memory_.begin());
  }

  void VM::load(std::span<std::uint8_t> program) noexcept {
    // load program starting at 0x200 (512)
    std::copy(program.begin(), program.end(), memory_.begin() + 0x200);
  }

  void VM::cycle() noexcept {
    auto inst = current();
    auto now = chrono::steady_clock::now();

    // want to run this at approx 500hz
    if (now >= last_cycle_ + 2ms) {
      last_cycle_ = now;

      execute(inst);
    }

    // 16,666us is approx 1/60th of a second. we want close to 60hz for updates on these timers
    if (now >= last_hz_ + 16666us) {
      last_hz_ = now;

      if (delay_ != 0) {
        --delay_;
      }

      if (sound_ != 0) {
        display_->buzz();
        --sound_;
      }
    }
  }

  std::uint16_t VM::pop() noexcept {
    // return current value, set current slot to 0, move current to previous slot
    return std::exchange(stack_[--sp_], 0);
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

  void VM::execute(std::uint16_t inst) noexcept {
    chip8::outs() << "executing instruction `0x" << chip8::instr << inst << "`";
    chip8::outs() << "  sp: " << +sp_;
    chip8::outs() << "  i: " << +i_;
    chip8::outs() << "  pc: " << +pc_;

    switch (inst & 0xF000U) {
      case 0x0000U: {          // instructions with one unique bit pattern
        if (inst == 0x00E0U) { // display-clear: 0x00E0
          display_->clear();
        } else if (inst == 0x00EEU) { // return: 0x00EE
          pc_ = pop();
        }

        break;
      }
      case 0x2000U: // call nnn: 0x2nnn
        push(pc_);
        pc_ = bottom_12bit(inst);
        return;
      case 0x1000U: // jump to nnn: 0x1nnn
        pc_ = bottom_12bit(inst);
        return;
      case 0x3000U: { // skip next instruction if Vx == nn: 0x3xnn
        auto vx = reg_[nth_4bit(inst, 2)];
        skip_if(vx == bottom_8bit(inst));
        break;
      }
      case 0x4000U: { // skip next instruction if Vx != nn: 0x4xnn
        auto vx = reg_[nth_4bit(inst, 2)];
        skip_if(vx != bottom_8bit(inst));
        break;
      }
      case 0x5000U: { // skip next instruction if Vx == Vy: 0x5xy0
        auto vx = reg_[nth_4bit(inst, 2)];
        auto vy = reg_[nth_4bit(inst, 3)];
        skip_if(vx == vy);
        break;
      }
      case 0x6000U: // set Vx to nn: 0x6xnn
        reg_[nth_4bit(inst, 2)] = bottom_8bit(inst);
        break;
      case 0x7000U: // increment Vx by nn: 0x7xnn
        reg_[nth_4bit(inst, 2)] += bottom_8bit(inst);
        break;
      case 0x8000U: // various math/bit ops
        execute_opcode_8(inst);
        break;
      case 0x9000U: { // skip next instruction if Vx != Vy: 0x9xy0
        auto vx = reg_[nth_4bit(inst, 2)];
        auto vy = reg_[nth_4bit(inst, 3)];
        skip_if(vx != vy);
        break;
      }
      case 0xA000U: // sets I to nnn: 0xAnnn
        i_ = bottom_12bit(inst);
        break;
      case 0xB000U: // jump to V0 + nnn: 0xCnnn
        pc_ = reg_[v0] + bottom_12bit(inst);
        return;
      case 0xC000U: { // set Vx to rand() & nn: 0xCxnn
        auto& vx = reg_[nth_4bit(inst, 2)];
        auto nn = bottom_8bit(inst);
        vx = rand_next() & nn;
        break;
      }
      case 0xD000U: { // display N-byte sprite located at I to (Vx, Vy): 0xDxyn
        auto vx = reg_[nth_4bit(inst, 2)];
        auto vy = reg_[nth_4bit(inst, 3)];
        auto n = nth_4bit(inst, 4);

        reg_[vf] = 0;

        chip8::outs() << "drawing sprite located at `0x" << std::hex << +i_ << "` that is 8x" << std::dec << +n;

        // sprite is N bytes long, also known as N pixels tall
        for (auto i = 0; i < n; ++i) {
          auto byte = memory_[i_ + i];

          chip8::outs() << "sprite layer " << i << " looks like: " << std::bitset<8>(byte);

          // go through each pixel of the byte for horizontal
          for (auto j = 0; j < 8; ++j) {
            auto bit = nth_bit(byte, j);

            if (!bit) {
              continue;
            }

            // if jth bit is unset, skip. otherwise, try updating pixel, see if we collide
            if (display_->set_pixel({vx + j, vy + i}, bit)) {
              reg_[vf] = 1;
            }
          }
        }

        break;
      }
      case 0xE000U: { // skip next if key Vx is pressed or not pressed: 0xEx9E, 0xExA1
        auto vx = reg_[nth_4bit(inst, 2)];
        auto value = display_->is_key_pressed(static_cast<Key>(vx));
        skip_if(bottom_8bit(inst) == 0x9E ? value : !value); // 9E = pressed, A1 = not pressed
        break;
      }
      case 0xF000U: // various
        execute_opcode_f(inst);
        break;
      default:
        chip8::errs() << "unsupported instruction! opcode: `0x" << chip8::instr << inst << "`";
        break;
    }

    next_instruction();
  }

  void VM::execute_opcode_8(std::uint16_t inst) noexcept {
    // all opcodes follow pattern 0x8xyN, N = flag for operation
    auto& vx = reg_[nth_4bit(inst, 2)];
    auto vy = reg_[nth_4bit(inst, 3)];

    // switch on N
    switch (nth_4bit(inst, 4)) {
      case 0x0U: // set Vx to Vy
        vx = vy;
        break;
      case 0x1U: // set Vx to Vx | Vy
        vx |= vy;
        break;
      case 0x2U: // set Vx to Vx & Vy
        vx &= vy;
        break;
      case 0x3U: // set Vx to Vx ^ Vy
        vx ^= vy;
        break;
      case 0x4U: // set Vx to Vx + Vy
        vx = wrapping_add(vx, vy);
        break;
      case 0x5U: // set Vx to Vx - Vy
        vx = wrapping_sub(vx, vy);
        break;
      case 0x6U: // put LSB of VX in VF, shift Vx right by 1
        reg_[vf] = lsb(vx);
        vx >>= 1;
        break;
      case 0x7U: // set Vx to Vx - Vy
        vx = wrapping_sub(vy, vx);
        break;
      case 0xEU: // put MSB of VX in VF, shift Vx left by 1
        reg_[vf] = msb(vx);
        vx <<= 1;
        break;
    }
  }

  void VM::execute_opcode_f(std::uint16_t inst) noexcept {
    auto x = nth_4bit(inst, 2);
    auto& vx = reg_[x];

    switch (bottom_8bit(inst)) {
      case 0x07: // set Vx = delay value: 0xFx07
        vx = delay_;
        break;
      case 0x0A: // set Vx = next keypress: 0xFx0A
        vx = static_cast<std::uint8_t>(display_->next_key());
        break;
      case 0x15: // set delay to Vx: 0xFx15
        delay_ = vx;
        break;
      case 0x18: // set sound to Vx: 0xFx18
        sound_ = vx;
        break;
      case 0x1E:  // set I to Vx: 0xFx1E
        i_ += vx; // VF is not changed
        break;
      case 0x29:     // set I to sprite address for a font: 0xFx29
        i_ = vx * 5; // fonts start at 0x0000, each is 5 bytes long. they are ordered by character
        break;
      case 0x33: // decode Vx into decimal representation: 0xFx33
        memory_[i_ + 2] = static_cast<std::uint8_t>(vx % 10);
        memory_[i_ + 1] = static_cast<std::uint8_t>((vx / 10) % 10);
        memory_[i_] = static_cast<std::uint8_t>(((vx / 10) / 10) % 10);
        break;
      case 0x55: { // reg dump V0 to Vx into memory, starting at I: 0xFx55
        for (auto i = 0; i <= x; ++i) {
          memory_[i_ + i] = reg_[i];
        }

        break;
      }
      case 0x65: { // load memory starting at I into V0 to VX: 0xFx65
        for (auto i = 0; i <= x; ++i) {
          reg_[i] = memory_[i_ + i];
        }

        break;
      }
    }
  }

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  std::uint8_t VM::wrapping_add(std::uint8_t a, std::uint8_t b) noexcept {
    auto x = static_cast<int>(a);
    auto y = static_cast<int>(b);
    auto result = x + y;

    // vf = 0 on overflow, 1 otherwise
    reg_[vf] = static_cast<std::uint8_t>(result > 255);

    // if we'd rap at 8-bit, we subtract 256 to wrap it ourselves
    return (result > 255) ? static_cast<std::uint8_t>(result - 256) //
                          : static_cast<std::uint8_t>(result);
  }

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  std::uint8_t VM::wrapping_sub(std::uint8_t a, std::uint8_t b) noexcept {
    auto x = static_cast<int>(a);
    auto y = static_cast<int>(b);
    auto result = x - y;

    // vf = 1 on no overflow, 0 on overflow
    reg_[vf] = static_cast<std::uint8_t>(result >= 0);

    // if the arithmetic would wrap at 8-bit, we start going backwards from 256
    return (result < 0) ? static_cast<std::uint8_t>(256 + result) //
                        : static_cast<std::uint8_t>(result);
  }
} // namespace chip8