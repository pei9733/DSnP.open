#include <iostream>
#include <unordered_map>
#include <string>
#include <unistd.h>
#include <fstream>
using namespace std;

int main()
{
  char arr[10];
  cout << hex << (void *)(arr + 1) << endl;
}
// What’s in file “ttt”? What’s the output??
