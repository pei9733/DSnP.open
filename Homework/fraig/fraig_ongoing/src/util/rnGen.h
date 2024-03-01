/****************************************************************************
  FileName     [ rnGen.h ]
  PackageName  [ util ]
  Synopsis     [ Random number generator ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#ifndef RN_GEN_H
#define RN_GEN_H

#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>  
#include <limits.h>

#define my_srandom  srandom
#define my_random   random

class RandomNumGen
{
   public:
      RandomNumGen() { my_srandom(getpid()); }
      RandomNumGen(unsigned seed) { my_srandom(seed); }
      const int operator() (const int range) const {
         return int(range * (double(my_random()) / INT_MAX));
      }
   unsigned long long getRandomULL1(unsigned long long const& min, unsigned long long const& max){
      unsigned long long r = 0ULL, m = 0ULL;
    r = (unsigned long long)my_random();
    r <<= 32;
    r |= (unsigned long long)my_random();
    m = (unsigned long long)my_random();
    m <<= 16;
    r ^= m;
    return r;
   }

};

#endif // RN_GEN_H

