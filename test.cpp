#include <iostream>
#include "core.hpp"


int main(int argc, char **argv) {
  bool occupy_state[4999] = {0};
  int G1, X1, Y1, LR1;
  int G2, X2, Y2, LR2;
  std::cin >> G1 >> Y1 >> X1 >> LR1;
  std::cin >> G2 >> Y2 >> X2 >> LR2;
  GwanKei::Cell c1(G1, Y1, X1, LR1);
  GwanKei::Cell c2(G2, Y2, X2, LR2);
  std::cout << "??? " << c1.get_id() << " / " << c2.get_id() << '\n';
  std::cout << c1.to_string() << " -> " << c2.to_string() << '\n';
  return 0;
}
