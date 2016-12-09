#include "NetObject_Types.h"
#include <map>

std::string NetObj_Types_To_String(NetObj_Type nt) {
  static const std::map<NetObj_Type, std::string> To_str {
    { NetObj_Type::player,      "player"      },
    { NetObj_Type::match_timer, "match_timer" },
    { NetObj_Type::size,        "nil"         }
  };
  auto& key = To_str.find(nt);
  if ( key != To_str.end() )
    return key._Ptr->_Myval.second;
  return "invalid";
}

NetObj_Type String_To_NetObj_Types(const std::string&) {
  //TODO 
  return NetObj_Type::size;
}
