#include <iostream>
#include "game.hpp"


int main(int argc, char **argv) {
  GwanKei::Layout layout;
  std::cout << layout.to_string();
  return 0;
}
