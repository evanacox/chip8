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

#include <iostream>

namespace chip8 {
  namespace internal {
    class NewlineOstream {
    public:
      NewlineOstream() = delete;

      explicit NewlineOstream(std::ostream* os) noexcept : os_{os} {}

      NewlineOstream(const NewlineOstream&) = delete;

      NewlineOstream(NewlineOstream&&) noexcept = default;

      NewlineOstream& operator=(const NewlineOstream&) = delete;

      NewlineOstream& operator=(NewlineOstream&&) = delete;

      ~NewlineOstream() {
        *os_ << '\n' << std::flush;
      }

      template <typename T> NewlineOstream& operator<<(T&& entity) noexcept {
        *os_ << entity;

        return *this;
      }

      NewlineOstream& operator<<(bool value) noexcept {
        *os_ << (value ? "True" : "False");

        return *this;
      }

    private:
      std::ostream* os_;
    };
  } // namespace internal

  std::ostream& reg(std::ostream& stream) noexcept;

  std::ostream& instr(std::ostream& stream) noexcept;

  internal::NewlineOstream outs() noexcept;

  internal::NewlineOstream errs() noexcept;
} // namespace chip8