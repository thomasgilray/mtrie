

// A simple testing apparatus for mtrie.cpp


#include <cstdlib>
#include <iostream>
#include <unordered_set>

#include "compat.h"
#include "mtrie.h"
#include <chrono>
#include <thread>


u64 utime()
{
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}


u32 x=123456789, y=362436069, z=521288629;
inline u32 rnd()
{
  u32 t;
  x ^= x << 16;
  x ^= x >> 5;
  x ^= x << 1;

  t = x;
  x = y;
  y = z;
  z = t ^ x ^ y;

  return z;
}


void fatal(const char* const msg)
{
  std::cout << msg << std::endl;
  exit(0);
}


void testrandom(mtrie<void>& mp)
{
    for (u64 i = 0; i < 500000; ++i)
    {
        const u64 n = (((u64)rnd()) << 32) | rnd();
        mp.insert(n,(void*)1);
    }
    for (u64 i = 100000; i < 500000; ++i)
    {
        const u64 n = (((u64)rnd()) << 32) | rnd();
        mp.remove(n);
    }
    for (u64 i = 0; i < 700000; ++i)
    {
        const u64 n = (((u64)rnd()) << 32) | rnd();
        mp.find(n);
    }
}


void testseq(mtrie<void>& mp)
{
    const u64 epochs = 5;
    const u64 max = 100000;
    for (u64 ep = 0; ep < epochs; ++ep)
        for (u64 j = 0; j < max; ++j)
        {
            const u64 i = ep*0x300000 + j;
            mp.insert(i,(void*)1);
        }
    for (mtrie<void>::iter it(&mp); it.more(); it.advance())
    {
		//std::cout << "key=" << it.getkey() << "  val=" << (u64)it.getval() << std::endl;
        if (it.getval() != (void*)1)
			fatal("Bad value during iteration.");
		for (u64 ep = 0; ep < epochs; ++ep)
			if (it.getkey() >= ep*0x300000 && it.getkey() < ep*0x300000+max)
				goto continuelab;
		fatal("Key not in range during iteration");
	continuelab:;
    }
    for (u64 ep = 1; ep < epochs; ++ep)
        for (u64 j = 0; j < max; ++j)
        {
            const u64 i = ep*0x300000 + j;
            mp.remove(i);
	}
    for (u64 ep = 0; ep < epochs+2; ++ep)
        for (u64 j = 0; j < max; ++j)
        {
            const u64 i = ep*0x300000 + j;
            void* ans = mp.find(i);
	    if (ans == 0 && ep == 0)
	        fatal("Should have found key.");
	    else if (ans != 0 && ep > 0)
	        fatal("Should not have found key.");
        }
    if (mp.count() != max)
        fatal("Wrong key-count.");
}


int main()
{
  u64 start = utime();
  for (u64 i = 0; i < 3; ++i)
  {    
      mtrie<void> mp;
      testrandom(mp);   
  }
  std::cout << ((utime() - start)/1000000) << std::endl;
  
  start = utime();
  for (u64 i = 0; i < 3; ++i)
  {
      mtrie<void> mp;
      testseq(mp);
  }
  std::cout << ((utime() - start)/1000000) << std::endl;
  
  return 0;
}



