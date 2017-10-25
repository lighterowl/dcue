// *******************************************************************
// DCue (sourceforge.net/projects/dcue)
// Copyright (c) 2013 Fluxtion, DCue project
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#ifndef _CUE_H
#define _CUE_H

#include "defs.h"
#include "support_types.h"
#include "string_utility.h"
#include "filename_utility.h"

#include <string>

time_t tm_to_time_t(const Tm_t& t);
Tm_t time_t_to_tm(const time_t t);

class Cue {
	std::string out;

	void add_meta(const std::string& comment);
	void add_generic_time(const unsigned minutes, const unsigned seconds, const unsigned frames);
	void add_index(const std::string& index, const unsigned minutes, const unsigned seconds, const unsigned frames);
	void add_type_from_ext(const std::string& extension);

public:
	void add_genre(const std::string& genre);
	void add_year(const std::string& year);
	void add_comment(const std::string& comment);
	void add_artist(const std::string& artist);
	void add_title(const std::string& title);
	void add_track(const unsigned num);
	void add_track_index(const unsigned minutes, const unsigned seconds, const unsigned frames);
	void add_filename(const std::string& name);
	void add_pregap_index(const unsigned minutes, const unsigned seconds, const unsigned frames);
	void add_artificial_pregap(const unsigned minutes, const unsigned seconds, const unsigned frames);
	void add_indent();
	const std::string& get_output() const;
};

class CueBuilder {
	CueSheet cuesheet;

	void build();
	void write_file(const Cue& c, const unsigned disc = 0);

public:
	CueBuilder(const CueSheet& cue) : cuesheet(cue) {
		build();
	}
};

#endif
