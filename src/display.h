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

#include "SFML/Audio.hpp"
#include "SFML/Graphics.hpp"
#include <array>
#include <cstdint>

namespace chip8 {
  //  Keypad                   Keyboard
  // +-+-+-+-+                +-+-+-+-+
  // |1|2|3|C|                |1|2|3|4|
  // +-+-+-+-+                +-+-+-+-+
  // |4|5|6|D|                |Q|W|E|R|
  // +-+-+-+-+       =>       +-+-+-+-+
  // |7|8|9|E|                |A|S|D|F|
  // +-+-+-+-+                +-+-+-+-+
  // |A|0|B|F|                |Z|X|C|V|
  // +-+-+-+-+                +-+-+-+-+
  enum class Key : std::uint8_t {
    unknown = 255,
    chip8_0 = 0,
    chip8_1,
    chip8_2,
    chip8_3,
    chip8_4,
    chip8_5,
    chip8_6,
    chip8_7,
    chip8_8,
    chip8_9,
    chip8_a,
    chip8_b,
    chip8_c,
    chip8_d,
    chip8_e,
    chip8_f,
  };

  enum class KeyEvent : bool {
    pressed = true,
    released = false,
  };

  class Display {
  public:
    explicit Display();

    // draws the screen
    void cycle() noexcept;

    // clears the screen
    void clear() noexcept;

    // plays a buzz noise
    void buzz() noexcept;

    // checks if the window is open
    [[nodiscard]] bool is_open() const noexcept;

    // blocks until a key that maps to 0-f is pressed
    [[nodiscard]] Key next_key() noexcept;

    // checks if a given key is currently pressed
    [[nodiscard]] bool is_key_pressed(Key key) const noexcept;

    // XORs the pixel at `coord` with `value`, returns true if a bit was unset
    [[nodiscard]] bool set_pixel(sf::Vector2i coord, bool value) noexcept;

  private:
    Key handle_keypress(const sf::Event& event, KeyEvent type) noexcept;

    std::unique_ptr<sf::RenderWindow> window_;
    std::array<bool, 16> keys_ = {};
    std::vector<std::unique_ptr<sf::RectangleShape>> pixels_;
    sf::Sound sound_;
    void handle_event(const sf::Event& event);
  };
} // namespace chip8