#include <fstream>
#include <iostream>

#include "../album.h"
#include "../cue.h"
#include <nlohmann/json.hpp>

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage : " << argv[0] << " discogs_data.json out.file\n";
    return 1;
  }
  std::ifstream f(argv[1], std::ios::in);
  nlohmann::json j;
  f >> j;
  cue::generate(Album::from_json(j), argv[2]);
  return 0;
}
