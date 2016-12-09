/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#include "Utility.h"
#include "Server_Vars.h"

#include <fstream>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>
#include <sstream>
#include <vector>
#include <string>


const float Util::E         =  2.718282f,
            Util::Log10E    =  0.4342945f,
            Util::Log2E     =  1.442695f,
            Util::Pi        =  3.141593f,
            Util::Tau       =  6.283185f,
            Util::Max_float =  3.402823E+38f,
            Util::Min_float = -3.402823E+38f,
            Util::Epsilon   =  0.000001f;

void assert(bool condition, std::string error_msg) {
  if ( !condition ) {
    std::ofstream efil("ERROR_DETAILS");
    efil << error_msg << '\n';
    exit(0);
  }
}

std::random_device Util::rand;
std::mt19937 Util::rgen(Util::rand());

float Util::R_Rand(float bot, float top) {
  std::uniform_real_distribution<> rn(bot, top);
  return rn(Util::rgen);
}


template <typename T>
T Util::R_Max(T x, T y) { return x > y ? x : y; }

template <typename T>
T Util::R_Min(T x, T y) { return x < y ? x : y; }


bool Util::Find_Substr(std::string& out, std::string in) {
  if ( in == "" ) return false;
  int it = 0, hit_count = 0;
  for ( auto i : out ) {
    if ( in[it++] == i ) ++hit_count;
    else { it = 0; hit_count = 0; }
    if ( hit_count == in.size() ) break;
  }
  if ( hit_count == in.size() ) return true;

  return false;
}

AOD::Vector Util::Rotate_Point(AOD::Vector o, AOD::Vector p, float angle) {
  return {
    std::cos(angle)*(p.x-o.x) - std::sin(angle)*(p.y-o.y) + o.x,
    std::sin(angle)*(p.x-o.x) - std::cos(angle)*(p.y-o.y) + o.y
  };
}

void ex_assert(bool statement, std::string err_msg) {
  if ( !statement ) {
    std::ofstream err("__EXIT_ERROR");
    err << "Err: " << err_msg << "\n";
    err.close();
    exit(0);
  }
}

std::vector<std::string> Util::Extract_Tokens(const std::string& str,
                                              std::string& rgx_str) {
  std::regex rgx ( rgx_str );
  std::smatch match;
  std::sregex_iterator it(str.begin(), str.end(), rgx), end;
  std::vector<std::string> strs;
  while ( it != end ) {
    strs.push_back(it->str());
    ++ it;
  }
  return strs;
}


// --- time -------------------------------------------------------------------
Util::Time::Time(uint16_t ms, uint16_t min, uint16_t sec, uint16_t h) {
  Set_Time(ms, min, sec, h);
}
void Util::Time::Refresh_Value() {
  seconds += milliseconds/1000;
  milliseconds %= 1000;
  minutes += seconds/60;
  seconds %= 60;
  hours   += minutes/60;
  minutes %= 60;
}

void Util::Time::Set_Time(uint16_t ms, uint16_t sec,
                          uint16_t min, uint16_t h) {
  milliseconds = ms;
  minutes      = min;
  seconds      = sec;
  hours        = h;
  Refresh_Value();
}

void Util::Time::Set_Time(uint32_t ms) {
  uint32_t sec, min, h;
  sec  = ms/1000;
  ms  %= 1000;
  min  = sec/60;
  sec %= 60;
  h    = min/60;
  min %= 60;
  Set_Time(ms, sec, min, h);
}

void Util::Time::Add_Time(uint16_t ms, uint16_t min,
                          uint16_t sec, uint16_t h) {
  milliseconds += ms;
  minutes      += min;
  seconds      += sec;
  hours        += h;
  Refresh_Value();
}

std::string Util::Time::R_Time_Str() {
  return std::to_string(hours) + ":"   + std::to_string(minutes) + ":" +
         std::to_string(seconds) + "." + std::to_string(milliseconds);
}

uint16_t Util::Time::R_Hours() const { return hours; }
uint16_t Util::Time::R_Minutes() const { return minutes; }
uint16_t Util::Time::R_Seconds() const { return seconds; }
uint16_t Util::Time::R_Milliseconds() const { return milliseconds; }

uint64_t Util::R_File_Size(const char* fname) {
  std::ifstream in(fname, std::ifstream::ate | std::ifstream::binary);
  return in.tellg();
}

std::string Util::R_IP(uint32_t host) {
  auto vec = Util::Pack_Num(host);
  std::string ip = "";
  for ( auto i : vec )
    ip += std::to_string(i) + '.';
  return ip;
}


void Util::Loop_Through_Clients(std::function<void(Player_Info*)>&& func) {
  for ( int it = 0, i = 0; it < MAX_ALLOWED_USERS && i < SV::clients_connected;
            ++ it ) {
    auto pl_info = game_manager->players[it];
    if ( pl_info != nullptr ) {
      func(pl_info);
      ++ i;
    }
  }
}