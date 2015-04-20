#include <iostream>
using namespace std;
int main() {
  unsigned char a = 0;
  unsigned char c = 255;
  unsigned char d = c + 1;
  unsigned char b = a - 1;
  cout << b << " " << (int)b << " " << (int)d << " " << (int)c << endl;
  return 0;
}
