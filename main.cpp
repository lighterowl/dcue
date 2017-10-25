// *******************************************************************
// DCue (sourceforge.net/projects/dcue)
// Copyright (c) 2013 Fluxtion, DCue project
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#include "defs.h"
#include "dcue.h"

#include <cstring>
#include <string>
#include <iostream>
#include <algorithm>

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::strcmp;
using std::transform;

static const string help = "********" COMMENT "********" LINE_END
"DCue is a cue sheet generator which uses Discogs.com to find track titles, lengths and other information." LINE_END LINE_END
"SYNTAX:" LINE_END
"dcue [(r)elease=|(m)aster=]<id> <audio filename>" LINE_END LINE_END
"FIRST ARGUMENT: a Discogs release or master release ID. Specify \"release=<id>\" or \"r=<id>\" or just \"<id>\" for a regular release and \"master=<id>\" or \"m=<id>\" for a master." LINE_END
"SECOND ARGUMENT: filename with optional absolute path of the AUDIO FILE you want to make a cue for. The cue file will be created alongside it. \"?\" characters will be replaced by the disc number." LINE_END LINE_END
"EXAMPLES:" LINE_END
"dcue master=218406 \"Clubland X-Treme Hardcore-Disc?.wav\"" LINE_END
"dcue r=1 \"/path/to/the punisher - stockholm.mp3\"" LINE_END
"dcue 1432 \"Release filename.flac\"" LINE_END LINE_END
"OPTIONS:" LINE_END
"--help (-h) - this command list" LINE_END;

static const string error = "Invalid syntax, use --help for help";

int main(int argc, char* argv[]) {
	string first;
	if(argc < 2) {
		cout << error << endl;
		return 1;
	}
	else {
		first = argv[1];
	}

	if(first == "--help" || first == "-h" || first == "-H") {
		cout << help << endl;
		return 0;
	}
	else {
		if(argc != 3) {
			cout << error << endl;
			return 1;
		}
		else {
			string rel = first;
			transform(rel.begin(), rel.end(), rel.begin(), ::tolower);
			string fn(argv[2]);
			string single = rel.substr(0, 2);
			string full = rel.substr(0, 8);
			string mfull = rel.substr(0, 7);
			if(rel.find("=") == string::npos) {
				DCue::generate(rel, fn);
			}
			else if(single == "r=" || full == "release=") {
				DCue::generate(rel.substr(rel.find("=") + 1), fn);
			}
			else if(single == "m=" || mfull == "master=") {
				DCue::generate(rel.substr(rel.find("=") + 1), fn, true);
			}
			else {
				cout << error << endl;
				return 1;
			}
		}
	}
	return 0;
}