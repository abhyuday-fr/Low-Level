#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << "[producer|consumer]\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
