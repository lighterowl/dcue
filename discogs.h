// *******************************************************************
// DCue (sourceforge.net/projects/dcue)
// Copyright (c) 2013 Fluxtion, DCue project
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#ifndef _DISCOGS_H
#define _DISCOGS_H

#include "defs.h"
#include "http.h"

#include <string>

class DiscogsApiRequest {
protected:
	HttpResponse res;

	 inline bool success() const {
		if(res.status == OK) {
			return true;
		}
		return false;
	}

public:
	bool send(const std::string&, std::string&);
};


class DiscogsReleaseRequest : public DiscogsApiRequest {
public:
	bool send(const std::string& rel_id, std::string& out, const bool is_master = false);
};

#endif
