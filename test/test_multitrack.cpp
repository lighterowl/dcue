#include <gtest/gtest.h>

#include "../../multitrack_strategy.h"

namespace {
struct MultitrackStrategyTest : public ::testing::Test {
  MultitrackStrategyTest()
      : index_(nlohmann::json::parse(R"json(
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
)json")),
        medley_(nlohmann::json::parse(R"json(
[
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
    }
]
)json")),
        medley_vector_({medley_.begin(), medley_.begin() + 1}) {
  }

  const nlohmann::json index_;
  const nlohmann::json medley_;
  const std::vector<nlohmann::json::const_iterator> medley_vector_;
  const Album album_ = {{},
                        "1979",
                        "Geek Rock",
                        "Introducing C with Classes",
                        "Bjarne Stroustroup"};
};
}

TEST_F(MultitrackStrategyTest, SingleUsesFirstMedleyTrackInfo) {
  const auto strategy = multitrack_strategy::single();
  const auto tracks = strategy->handle_medley(medley_vector_, album_);

  const auto expected_track = Track{"Memnon Featuring Seroya",
                                    "Desire (Dub Mix)", Track::Duration{3, 28}};

  ASSERT_EQ(tracks, std::vector<Track>{expected_track});
}

TEST_F(MultitrackStrategyTest, SingleUsesAlbumArtistWhenNoTrackArtist) {
  const auto strategy = multitrack_strategy::single();
  const auto tracks = strategy->handle_index(index_, album_);

  const auto expected_track =
      Track{album_.album_artist, "Alan's Psychedelic Breakfast",
            Track::Duration{13, 1}};

  ASSERT_EQ(tracks, std::vector<Track>{expected_track});
}

TEST_F(MultitrackStrategyTest, MergeMergesSubtracksFromIndex) {
  const auto strategy = multitrack_strategy::merge();
  const auto tracks = strategy->handle_index(index_, album_);

  const auto expected_track = Track{album_.album_artist,
                                    "Alan's Psychedelic Breakfast : Rise And "
                                    "Shine / Sunny Side Up / Morning Glory",
                                    Track::Duration{13, 1}};

  ASSERT_EQ(tracks, std::vector<Track>{expected_track});
}

TEST_F(MultitrackStrategyTest, MergeMergesTitleAndAritstsInfoFromMedleys) {
  const auto strategy = multitrack_strategy::merge();
  const auto tracks = strategy->handle_medley(medley_vector_, album_);

  const auto expected_track = Track{
      "Memnon Featuring Seroya / PQM Featuring Cica",
      "Desire (Dub Mix) / The Flying Song (Acapella)", Track::Duration{3, 28}};

  ASSERT_EQ(tracks, std::vector<Track>{expected_track});
}

TEST_F(MultitrackStrategyTest, SeparateThrowsOnNoDuration) {
  const auto strategy = multitrack_strategy::separate();
  ASSERT_THROW(strategy->handle_medley(medley_vector_, album_),
               std::runtime_error);
  ASSERT_THROW(strategy->handle_index(index_, album_), std::runtime_error);
}

TEST_F(MultitrackStrategyTest, SeparateOutputsEachMedleyTrack) {
  const auto medley_json_with_durations = nlohmann::json::parse(R"json(
[
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
        "duration": "1:22"
    }
]
)json");

  const auto vec = std::vector<nlohmann::json::const_iterator>{
      medley_json_with_durations.begin(),
      medley_json_with_durations.begin() + 1};

  const auto strategy = multitrack_strategy::separate();
  const auto tracks = strategy->handle_medley(vec, album_);

  const auto expected_tracks = std::vector<Track>{
      {"Memnon Featuring Seroya", "Desire (Dub Mix)", Track::Duration{3, 28}},
      {"PQM Featuring Cica", "The Flying Song (Acapella)",
       Track::Duration{1, 22}}};

  ASSERT_EQ(tracks, expected_tracks);
}

TEST_F(MultitrackStrategyTest, SeparateOutputsEachIndexTrack) {
  const auto index_json_with_durations_ = nlohmann::json::parse(R"json(
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
            "duration": "6:00"
        },
        {
            "position": "5b",
            "type_": "track",
            "title": "Sunny Side Up",
            "duration": "6:00"
        },
        {
            "position": "5c",
            "type_": "track",
            "title": "Morning Glory",
            "duration": "1:01"
        }
    ]
}
)json");
  const auto strategy = multitrack_strategy::separate();
  const auto tracks =
      strategy->handle_index(index_json_with_durations_, album_);

  const auto expected_tracks = std::vector<Track>{
      {album_.album_artist, "Alan's Psychedelic Breakfast : Rise And Shine",
       Track::Duration{6, 0}},
      {album_.album_artist, "Alan's Psychedelic Breakfast : Sunny Side Up",
       Track::Duration{6, 0}},
      {album_.album_artist, "Alan's Psychedelic Breakfast : Morning Glory",
       Track::Duration{1, 1}}};

  ASSERT_EQ(tracks, expected_tracks);
}
