#ifndef CONSOLE_UTIL_H_
#define CONSOLE_UTIL_H_
#pragma once
#include <string>
void Clear_Screen();
void Output_Map_Files();
enum class CONSOLE_COLOURS {
  RED,
  GREEN,
  BLUE,
  PURPLE,
  BROWN,
  TEAL,
  BRIGHT_RED,
  BRIGHT_GREEN,
  BRIGHT_BLUE,
  BRIGHT_PURPLE,
  BRIGHT_BROWN,
  BRIGHT_TEAL
};
void Output_In_Colour(std::string str, CONSOLE_COLOURS colour);

#endif