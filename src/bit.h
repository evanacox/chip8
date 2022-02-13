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

#include <cstdint>

namespace chip8 {
  // gets a bitmask to get the Nth 4 bit chunk out of a given uint16_t
  [[nodiscard]] constexpr std::uint16_t nth_mask(int n) noexcept {
    return 0xFU << 4 * (4 - n);
  }

  // gets the Nth 4 bit chunk of `x`
  //
  // given `0xabcd`, n for parts -> a = 1, b = 2, c = 3, d = 4
  [[nodiscard]] constexpr std::uint8_t nth_4bit(std::uint16_t x, int n) noexcept {
    // extract bits that are zeroed in mask, shift down to bottom 4 bits
    return static_cast<std::uint8_t>((x & nth_mask(n)) >> (4 * (4 - n)));
  }

  // gets the bottom 12 bits of `x`
  [[nodiscard]] constexpr std::uint16_t bottom_12bit(std::uint16_t x) noexcept {
    return static_cast<std::uint16_t>(x & 0x0FFFU);
  }

  // gets the least-significant byte of `x`
  [[nodiscard]] constexpr std::uint8_t bottom_8bit(std::uint16_t x) noexcept {
    return static_cast<std::uint8_t>(x & 0xFFU);
  }

  // gets the most-significant byte of `x`
  [[nodiscard]] constexpr std::uint8_t top_8bit(std::uint16_t x) noexcept {
    return static_cast<std::uint8_t>((x & 0xFF00U) >> 8);
  }

  // gets the least significant bit of `x`
  [[nodiscard]] constexpr std::uint8_t lsb(std::uint8_t x) noexcept {
    return static_cast<std::uint8_t>(x & 0b1U);
  }

  // gets the most significant bit of `x`
  [[nodiscard]] constexpr std::uint8_t msb(std::uint8_t x) noexcept {
    constexpr auto set_msb = 0b1U << 7;

    return static_cast<std::uint8_t>((x & set_msb) != 0);
  }

  // treats bits as an 'array' where 0 = msb and 7 = lsb, gets nth bit
  [[nodiscard]] constexpr bool nth_bit(std::uint8_t x, int n) noexcept {
    return static_cast<bool>((x >> (7 - n)) & 0b1);
  }
} // namespace chip8