// *******************************************************************
// DCue (github.com/xavery/dcue)
// Copyright (c) 2019-2022 Daniel Kamil Kozar
// Original version by :
// DCue (sourceforge.net/projects/dcue)
// Copyright (c) 2013 Fluxtion, DCue project
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#include "discogs.h"

#include <spdlog/fmt/fmt.h>

#ifdef DCUE_OFFICIAL_BUILD
#include "appkey.h"
#endif

using namespace std::string_view_literals;

namespace {
void set_discogs_resource(HttpGet& req, std::string_view type,
                          std::string_view id) {
  req.set_resource(fmt::format("/{}/{}", type, id));
}
}

HttpGet DiscogsRequestFactory::create(const std::string& dcue_id) {
  HttpGet req;
  req.set_hostname("https://api.discogs.com");
#ifdef DCUE_OFFICIAL_BUILD
  req.add_header(discogs_key::getHeader());
#endif
  if (auto eq_pos = dcue_id.find("="); eq_pos != std::string::npos) {
    if (dcue_id.find("r=") == 0 || dcue_id.find("release=") == 0) {
      const auto rel_id = std::string_view{dcue_id}.substr(eq_pos + 1);
      set_discogs_resource(req, "releases"sv, rel_id);
    } else if (dcue_id.find("m=") == 0 || dcue_id.find("master=") == 0) {
      const auto mas_id = std::string_view{dcue_id}.substr(eq_pos + 1);
      set_discogs_resource(req, "masters"sv, mas_id);
    }
  } else {
    set_discogs_resource(req, "releases"sv, dcue_id);
  }
  return req;
}
