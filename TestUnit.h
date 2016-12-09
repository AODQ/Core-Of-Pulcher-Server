#ifndef TESTUNIT_H_
#define TESTUNIT_H_
#pragma once

#include <vector>
namespace TestUnits {
  enum class TestUnit {
    General, Utility, Networking,
    Size
  };

  std::string TestUnit_To_Str(TestUnit);
  TestUnit Str_To_TestUnit(const std::string&);

  void Test_Units(std::vector<TestUnit>);
  // Will ask user for test units to test and then test them
  void Query_Test_Units();
}

#endif
