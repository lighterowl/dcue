#include <fstream>
#include <iostream>

#include "../album.h"
#include "../cue.h"
#include "../multitrack_strategy.h"
#include <nlohmann/json.hpp>

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage : " << argv[0] << " discogs_data.json out.file\n";
    return 1;
  }
  std::ifstream f(argv[1], std::ios::in);
  nlohmann::json j;
  f >> j;
  const auto strategy = multitrack_strategy::single();
  cue::generate(Album::from_json(j, *strategy, *strategy), argv[2]);
  return 0;
}
