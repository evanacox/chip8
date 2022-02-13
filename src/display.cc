//======---------------------------------------------------------------======//
//                                                                           //
// Copyright 2022 Evan Cox <evanacox00@gmail.com>. All rights reserved. //
//                                                                           //
// Use of this source code is governed by a BSD-style license that can be    //
// found in the LICENSE.txt file at the root of this project, or at the      //
// following link: https://opensource.org/licenses/BSD-3-Clause              //
//                                                                           //
//======---------------------------------------------------------------======//

#include "./display.h"
#include "./debug.h"
#include "./vm.h"
#include "SFML/Audio.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/System.hpp"
#include <bit>
#include <cassert>
#include <cstdlib>

namespace chip8 {
  Display::Display()
      : window_(std::make_unique<sf::RenderWindow>(sf::VideoMode(1024, 512),
          "CHIP-8 Emu",
          sf::Style::Titlebar | sf::Style::Close)) {
    pixels_.reserve(chip8_pixels);

    auto pixel_size = static_cast<float>(1024 / chip8_width); // NOLINT(bugprone-integer-division)

    for (auto i = 0U; i < chip8_height; ++i) {
      for (auto j = 0U; j < chip8_width; ++j) {
        pixels_.emplace_back(std::make_unique<sf::RectangleShape>(sf::Vector2f{pixel_size, pixel_size}));

        auto& rect = *pixels_.back();
        rect.setFillColor(sf::Color::Black);
        rect.setPosition(static_cast<float>(j) * pixel_size, static_cast<float>(i) * pixel_size);
      }
    }
  }

  void Display::cycle() noexcept {
    sf::Event event;

    while (window_->pollEvent(event)) {
      handle_event(event);
    }

    window_->clear();

    for (auto& pixel : pixels_) {
      window_->draw(*pixel);
    }

    window_->display();
  }
  void Display::handle_event(const sf::Event& event) {
    switch (event.type) {
      case sf::Event::Closed:
        window_->close();
        break;
      case sf::Event::KeyPressed:
        handle_keypress(event, KeyEvent::pressed);
        break;
      case sf::Event::KeyReleased:
        handle_keypress(event, KeyEvent::released);
        break;
      case sf::Event::MouseWheelMoved:
      case sf::Event::MouseWheelScrolled:
      case sf::Event::MouseButtonPressed:
      case sf::Event::MouseButtonReleased:
      case sf::Event::MouseMoved:
      case sf::Event::MouseEntered:
      case sf::Event::MouseLeft:
      default:
        break;
    }
  }

  void Display::buzz() noexcept {
    if (!window_) {
      sound_.play();
    }

    chip8::outs() << "BUZZ!!!!!";
  }

  void Display::clear() noexcept {
    for (auto& pixel : pixels_) {
      pixel->setFillColor(sf::Color::Black);
    }
  }

  bool Display::is_open() const noexcept {
    return window_->isOpen();
  }

  Key Display::next_key() noexcept {
    sf::Event event;

    while (true) {
      // block until we have a keypress
      while (window_->waitEvent(event) && event.type != sf::Event::KeyPressed) {
        handle_event(event);
      }

      // key may or may not be one we care about
      auto key = handle_keypress(event, KeyEvent::pressed);

      // if it is, we return. otherwise, try again
      if (key != Key::unknown) {
        return key;
      }
    }
  }

  bool Display::is_key_pressed(Key key) const noexcept {
    return keys_[static_cast<std::uint8_t>(key)];
  }

  Key Display::handle_keypress(const sf::Event& event, KeyEvent type) noexcept {
    Key key;

    chip8::outs() << "got key event! type: " << (type == KeyEvent::pressed ? "pressed" : "released");

    // Keypad                   Keyboard
    // +-+-+-+-+                +-+-+-+-+
    // |1|2|3|C|                |1|2|3|4|
    // +-+-+-+-+                +-+-+-+-+
    // |4|5|6|D|                |Q|W|E|R|
    // +-+-+-+-+       =>       +-+-+-+-+
    // |7|8|9|E|                |A|S|D|F|
    // +-+-+-+-+                +-+-+-+-+
    // |A|0|B|F|                |Z|X|C|V|
    // +-+-+-+-+                +-+-+-+-+
    switch (event.key.code) {
      case sf::Keyboard::Num1:
        key = Key::chip8_1;
        break;
      case sf::Keyboard::Num2:
        key = Key::chip8_2;
        break;
      case sf::Keyboard::Num3:
        key = Key::chip8_3;
        break;
      case sf::Keyboard::Num4:
        key = Key::chip8_c;
        break;
      case sf::Keyboard::Q:
        key = Key::chip8_4;
        break;
      case sf::Keyboard::W:
        key = Key::chip8_5;
        break;
      case sf::Keyboard::E:
        key = Key::chip8_6;
        break;
      case sf::Keyboard::R:
        key = Key::chip8_d;
        break;
      case sf::Keyboard::A:
        key = Key::chip8_7;
        break;
      case sf::Keyboard::S:
        key = Key::chip8_8;
        break;
      case sf::Keyboard::D:
        key = Key::chip8_9;
        break;
      case sf::Keyboard::F:
        key = Key::chip8_e;
        break;
      case sf::Keyboard::Z:
        key = Key::chip8_a;
        break;
      case sf::Keyboard::X:
        key = Key::chip8_0;
        break;
      case sf::Keyboard::C:
        key = Key::chip8_b;
        break;
      case sf::Keyboard::V:
        key = Key::chip8_f;
        break;
      default:
        return Key::unknown;
    }

    keys_[static_cast<std::uint8_t>(key)] = type == KeyEvent::pressed;

    return key;
  }

  bool Display::set_pixel(sf::Vector2i coord, bool value) noexcept {
    auto [x, y] = coord;

    auto wrapped_x = x % chip8_width;
    auto wrapped_y = y % chip8_height;

    auto& pixel = *pixels_[wrapped_y * chip8_width + wrapped_x];
    auto old_value = pixel.getFillColor() == sf::Color::White;
    auto new_value = static_cast<bool>(static_cast<int>(old_value) ^ static_cast<int>(value));

    pixel.setFillColor(new_value ? sf::Color::White : sf::Color::Black);

    // VF needs to be set if and only if a pixel goes from set -> unset
    return (!new_value && old_value);
  }
} // namespace chip8