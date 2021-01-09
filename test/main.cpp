#include <fstream>
#include <iostream>

#include "../cue.h"
#include "../json.hpp"

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage : " << argv[0] << " discogs_data.json out.file\n";
    return 1;
  }
  std::ifstream f(argv[1], std::ios::in);
  if (!f.is_open()) {
    std::cerr << "Could not open " << argv[1] << '\n';
    return 1;
  }
  nlohmann::json j;
  f >> j;
  generate(j, argv[2]);
  return 0;
}
