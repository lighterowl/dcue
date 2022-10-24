// *******************************************************************
// DCue (github.com/xavery/dcue)
// Copyright (c) 2022 Daniel Kamil Kozar
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#include "support_types.h"

#include <charconv>
#include <stdexcept>

#include <spdlog/fmt/fmt.h>

Track::Duration::Duration(std::string_view duration) {
  const auto colon_pos = duration.find(':');
  if (colon_pos == std::string_view::npos ||
      colon_pos == duration.length() - 1) {
    throw std::runtime_error(
        fmt::format("Unrecognised duration {}, qutting", duration));
  }
  auto min_conv_result =
      std::from_chars(duration.data(), duration.data() + colon_pos, min);
  auto sec_conv_result =
      std::from_chars(duration.data() + colon_pos + 1,
                      duration.data() + duration.length(), sec);
  if (min_conv_result.ec != std::errc{} || sec_conv_result.ec != std::errc{}) {
    throw std::runtime_error(
        fmt::format("Unrecognised duration {}, qutting", duration));
  }
}
