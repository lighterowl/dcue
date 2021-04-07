// *******************************************************************
// DCue (github.com/xavery/dcue)
// Copyright (c) 2019-2021 Daniel Kamil Kozar
// Original version by :
// DCue (sourceforge.net/projects/dcue)
// Copyright (c) 2013 Fluxtion, DCue project
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#include "discogs.h"

#ifdef DCUE_OFFICIAL_BUILD
#include "appkey.h"
#endif

bool DiscogsReleaseRequest::send(const std::string& rel_id, nlohmann::json& out,
                                 const bool is_master) {
  HttpGet req;
  if (is_master) {
    req.set_resource("/masters/" + rel_id);
  } else {
    req.set_resource("/releases/" + rel_id);
  }
#ifdef DCUE_OFFICIAL_BUILD
  req.add_header(discogs_key::getHeader());
#endif
  req.send("https://api.discogs.com", res);
  if (success()) {
    out = nlohmann::json::parse(res.body);
    return true;
  }
  return false;
}
