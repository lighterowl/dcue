// *******************************************************************
// DCue (github.com/xavery/dcue)
// Copyright (c) 2022 Daniel Kamil Kozar
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#include <gmock/gmock.h>

using namespace ::testing;

#include "../../album.h"
#include "../../multitrack_strategy.h"

namespace {
struct MockMultitrackStrategy : public multitrack_strategy {
  MOCK_METHOD(std::vector<Track>, handle_index,
              (const nlohmann::json&, const Album&), (const override));
  MOCK_METHOD(std::vector<Track>, handle_medley,
              (const std::vector<nlohmann::json::const_iterator>&,
               const Album&),
              (const override));
};
}

TEST(Album, FromJsonCallsIndexTrackStrategy) {
  MockMultitrackStrategy mock;
  constexpr std::string_view ahm_tracklist = R"json(
{
    "tracklist": [
        {
            "position": "4",
            "type_": "track",
            "title": "Fat Old Sun",
            "duration": "5:23"
        },
        {
            "position": "",
            "type_": "index",
            "title": "Alan's Psychedelic Breakfast",
            "duration": "13:01",
            "sub_tracks": [
                {
                    "position": "5a",
                    "type_": "track",
                    "title": "Rise And Shine",
                    "duration": ""
                },
                {
                    "position": "5b",
                    "type_": "track",
                    "title": "Sunny Side Up",
                    "duration": ""
                },
                {
                    "position": "5c",
                    "type_": "track",
                    "title": "Morning Glory",
                    "duration": ""
                }
            ]
        }
    ]
}
)json";

  const auto fake_track = Track{"foobar", "testme", Track::Duration{42, 42}};
  EXPECT_CALL(mock, handle_index(Truly([](auto& index_track) {
                                   return index_track.at("type_") == "index" &&
                                          index_track.at("title") ==
                                              "Alan's Psychedelic Breakfast";
                                 }),
                                 _))
      .WillOnce(Return(std::vector<Track>{fake_track}));

  const auto album =
      Album::from_json(nlohmann::json::parse(ahm_tracklist), mock, mock);

  ASSERT_EQ(album.discs.size(), 1);
  auto& disc = album.discs[0];
  ASSERT_EQ(disc.tracks.size(), 2);
  ASSERT_EQ(disc.tracks[0].title, "Fat Old Sun");
  ASSERT_EQ(disc.tracks[0].length, (Track::Duration{5, 23}));
  ASSERT_EQ(disc.tracks[1], fake_track);
}

TEST(Album, FromJsonCallsMedleyTrackStrategy) {
  MockMultitrackStrategy mock;
  constexpr std::string_view discogs = R"json(
{
    "tracklist": [
        {
            "position": "2.12A",
            "type_": "track",
            "artists": [
                {
                    "name": "Memnon",
                    "anv": "",
                    "join": "Featuring",
                    "role": "",
                    "tracks": "",
                    "id": 33076,
                    "resource_url": "https://api.discogs.com/artists/33076"
                },
                {
                    "name": "Seroya",
                    "anv": "",
                    "join": "",
                    "role": "",
                    "tracks": "",
                    "id": 110112,
                    "resource_url": "https://api.discogs.com/artists/110112"
                }
            ],
            "title": "Desire (Dub Mix)",
            "extraartists": [
                {
                    "name": "The Light",
                    "anv": "",
                    "join": "",
                    "role": "Remix",
                    "tracks": "",
                    "id": 2863,
                    "resource_url": "https://api.discogs.com/artists/2863"
                },
                {
                    "name": "Seroya",
                    "anv": "",
                    "join": "",
                    "role": "Vocals",
                    "tracks": "",
                    "id": 110112,
                    "resource_url": "https://api.discogs.com/artists/110112"
                }
            ],
            "duration": "3:28"
        },
        {
            "position": "2.12B",
            "type_": "track",
            "artists": [
                {
                    "name": "Prince Quick Mix",
                    "anv": "PQM",
                    "join": "Featuring",
                    "role": "",
                    "tracks": "",
                    "id": 56372,
                    "resource_url": "https://api.discogs.com/artists/56372"
                },
                {
                    "name": "Cica",
                    "anv": "",
                    "join": "",
                    "role": "",
                    "tracks": "",
                    "id": 64215,
                    "resource_url": "https://api.discogs.com/artists/64215"
                }
            ],
            "title": "The Flying Song (Acapella)",
            "extraartists": [
                {
                    "name": "Cica",
                    "anv": "",
                    "join": "",
                    "role": "Vocals",
                    "tracks": "",
                    "id": 64215,
                    "resource_url": "https://api.discogs.com/artists/64215"
                }
            ],
            "duration": ""
        },
        {
            "position": "2.13",
            "type_": "track",
            "artists": [
                {
                    "name": "Albion",
                    "anv": "",
                    "join": "",
                    "role": "",
                    "tracks": "",
                    "id": 4225,
                    "resource_url": "https://api.discogs.com/artists/4225"
                }
            ],
            "title": "Air 2000 (Oliver Lieb Remix)",
            "extraartists": [
                {
                    "name": "Oliver Lieb",
                    "anv": "",
                    "join": "",
                    "role": "Remix",
                    "tracks": "",
                    "id": 4235,
                    "resource_url": "https://api.discogs.com/artists/4235"
                }
            ],
            "duration": "3:15"
        }
    ]
}
)json";

  const auto fake_track = Track{"foobar", "testme", Track::Duration{42, 42}};
  const auto is_medley_vector =
      [](const std::vector<nlohmann::json::const_iterator>& medley_tracks)
      -> bool {
    return medley_tracks.size() == 2 &&
           medley_tracks[0]->at("title") == "Desire (Dub Mix)" &&
           medley_tracks[1]->at("title") == "The Flying Song (Acapella)";
  };

  EXPECT_CALL(mock, handle_medley(Truly(is_medley_vector), _))
      .WillOnce(Return(std::vector<Track>{fake_track}));

  const auto album =
      Album::from_json(nlohmann::json::parse(discogs), mock, mock);

  ASSERT_EQ(album.discs.size(), 2);
  auto& disc = album.discs[1];
  ASSERT_EQ(disc.tracks.size(), 2);
  ASSERT_EQ(disc.tracks[0], fake_track);
  ASSERT_EQ(disc.tracks[1].title, "Air 2000 (Oliver Lieb Remix)");
  ASSERT_EQ(disc.tracks[1].artist, "Albion");
  ASSERT_EQ(disc.tracks[1].length, (Track::Duration{3, 15}));
}
