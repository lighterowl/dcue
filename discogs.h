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

#ifndef DCUE_DISCOGS_H
#define DCUE_DISCOGS_H

#include "http.h"
#include "json.hpp"

#include <string>

class DiscogsApiRequest {
protected:
  HttpResponse res;

  bool success() const {
    return res.status == HttpStatus::OK;
  }

public:
  bool send(const std::string&, std::string&);
};

class DiscogsReleaseRequest : public DiscogsApiRequest {
public:
  bool send(const std::string& rel_id, nlohmann::json& out,
            const bool is_master = false);
};

#endif
