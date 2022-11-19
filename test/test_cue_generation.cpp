#include <gtest/gtest.h>

#include <array>
#include <sstream>

#include "../album.h"
#include "../cue.h"
#include "../multitrack_strategy.h"
#include <nlohmann/json.hpp>

using namespace std::string_view_literals;

namespace {
const auto strategy = multitrack_strategy::single();

template <std::size_t N>
void test_cue_generation(
    std::string_view json,
    const std::array<std::vector<std::string_view>, N>& cues) {
  const auto album =
      Album::from_json(nlohmann::json::parse(json), *strategy, *strategy);

  std::vector<std::shared_ptr<std::iostream>> streams;
  streams.reserve(N);

  const auto basepath = std::filesystem::path{
      ::testing::UnitTest::GetInstance()->current_test_info()->name()};

  std::vector<std::filesystem::path> expected_cue_paths;
  if (album.discs.size() == 1) {
    auto path = basepath;
    path.replace_extension(".cue");
    expected_cue_paths.push_back(path);
  } else {
    for (unsigned int d = 0; d < album.discs.size(); ++d) {
      auto s = basepath.filename().string();
      s.append("-");
      s.append(std::to_string(d + 1));
      auto path = basepath;
      path.replace_filename(s);
      path.replace_extension(".cue");
      expected_cue_paths.push_back(path);
    }
  }

  auto wavpath = basepath;
  wavpath.replace_extension(".wav");

  cue::generate(album, wavpath, [&](auto& cue_path) {
    auto stream = std::make_shared<std::stringstream>();
    streams.push_back(stream);
    EXPECT_EQ(cue_path, expected_cue_paths.at(streams.size() - 1));
    return stream;
  });

  ASSERT_EQ(streams.size(), cues.size());
  for (unsigned int cue_idx = 0; cue_idx < N; ++cue_idx) {
    auto& stream = *(streams[cue_idx]);
    auto& expected = cues[cue_idx];

    unsigned int line_idx;
    std::string line;
    for (line_idx = 0; std::getline(stream, line); ++line_idx) {
      if (line.find("REM COMMENT \"DCue") == 0) {
        continue;
      }
      ASSERT_EQ(line.back(), '\r');
      line.pop_back();
      ASSERT_EQ(line, expected[line_idx]);
    }
    std::getline(stream, line);
    ASSERT_TRUE(line.empty()) << "last line not empty";
    ASSERT_TRUE(stream.eof());
    ASSERT_EQ(line_idx, expected.size());
  }
}
}

TEST(CueGeneration, m218406) {
#include "data/m218406.cpp"
  test_cue_generation(json, cue);
}

TEST(CueGeneration, r1) {
#include "data/r1.cpp"
  test_cue_generation(json, cue);
}

TEST(CueGeneration, r13835) {
#include "data/r13835.cpp"
  test_cue_generation(json, cue);
}

TEST(CueGeneration, r121788) {
#include "data/r121788.cpp"
  test_cue_generation(json, cue);
}

TEST(CueGeneration, r374278) {
#include "data/r374278.cpp"
  test_cue_generation(json, cue);
}

TEST(CueGeneration, r3151795) {
#include "data/r3151795.cpp"
  test_cue_generation(json, cue);
}

TEST(CueGeneration, r9922074) {
#include "data/r9922074.cpp"
  test_cue_generation(json, cue);
}
