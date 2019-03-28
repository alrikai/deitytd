#include "gtest/gtest.h"

#include <memory>
#include <vector>
#include <string>

namespace {

//breakpoint on failure:
//gdb --args ./TestFlames --gtest_break_on_failure	
TEST (BasicAttackTest, LinearDefault) {

  int i = 1;  
  EXPECT_EQ(0, i);

}

}
