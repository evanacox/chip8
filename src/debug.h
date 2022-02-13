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
#ifndef NDEBUG
    inline constexpr bool should_log = false;
#else
    inline constexpr bool should_log = false;
#endif

    class NewlineOstream {
    public:
      NewlineOstream() = delete;

      explicit NewlineOstream(std::ostream* os, std::string_view header) noexcept;

      NewlineOstream(const NewlineOstream&) = delete;

      NewlineOstream(NewlineOstream&&) noexcept = default;

      NewlineOstream& operator=(const NewlineOstream&) = delete;

      NewlineOstream& operator=(NewlineOstream&&) = delete;

      ~NewlineOstream();

      template <typename T> NewlineOstream& operator<<(T&& entity) noexcept {
        if constexpr (should_log) {
          *os_ << entity;
        }

        return *this;
      }

      NewlineOstream& operator<<(bool value) noexcept;

    private:
      std::ostream* os_;
      std::string_view header_;
    };
  } // namespace internal

  std::ostream& reg(std::ostream& stream) noexcept;

  std::ostream& instr(std::ostream& stream) noexcept;

  internal::NewlineOstream outs() noexcept;

  internal::NewlineOstream errs() noexcept;
} // namespace chip8