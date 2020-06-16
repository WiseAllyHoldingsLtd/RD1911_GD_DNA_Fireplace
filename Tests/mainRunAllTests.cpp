#include "CppUTest/TestHarness.h"
#include "CppUTest/SimpleString.h"
#include "CppUTest/PlatformSpecificFunctions.h"
#include "CppUTest/TestMemoryAllocator.h"
#include "CppUTest/TestPlugin.h"
#include "CppUTest/TestRegistry.h"
#include "CppUTest/CommandLineTestRunner.h"


int main(int ac, char** av)
{
  return CommandLineTestRunner::RunAllTests(ac, av);
}
