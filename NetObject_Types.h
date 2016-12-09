#ifndef OBJECT_TYPES_H_
#define OBJECT_TYPES_H_
#pragma once
// similar to client's PulNetObj.h
/* normally we would never have to assign obj IDs to objects in game, and even
 * if we did, the assignment of the ID would be done in the constructor itself.
 * Alas, as this is a multiplayer game, we must somehow create an object on
 * the client side of the game, and the best way to do this is to keep track
 * of all networked object IDs in the game so we can merely pass a byte or
 * or two to signify which object should be created
 */

enum class NetObj_Type {
  player,
  match_timer,
  size
};

#include <string>
std::string NetObj_Types_To_String(NetObj_Type nt);
NetObj_Type String_To_NetObj_Types(const std::string&);

#endif
