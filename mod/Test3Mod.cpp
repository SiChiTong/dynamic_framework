/*
 * Test3Mod.cpp
 *
 *  Created on: Jun 30, 2014
 *      Author: sam
 */

/*
 * Test2Mod.cpp
 *
 *  Created on: Jun 30, 2014
 *      Author: sam
 */

#include "Test3Mod.h"

MAKE_MODULE(Test3Mod, Thread Two)

void Test3Mod::update(Test3Rep& theTest3Rep)
{
  //std::cout << "provides->Test3Mod" << std::endl;
  theTest3Rep.test3 = rand();
  //std::cout << "theTest3Rep.test3=" << theTest3Rep.test3 << std::endl;
}
