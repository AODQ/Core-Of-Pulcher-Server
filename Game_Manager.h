/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */

#ifndef GAME_MANAGER_H_
#define GAME_MANAGER_H_
#pragma once

#include <vector>
#include "Map.h"
#include "Server_Vars.h"

class Projectile_Manager;
class Player_Info;
class Tile_Sheet_Container;
class Map;
class Game_Manager {
public:
  uint8_t server_timer;
  Game_Manager();
  Projectile_Manager* projectiles;
  Player_Info* players[128];
  Map* curr_map;

  void Load_Map(const char* map_name);
  void Add_Player();
  void Update();

  void Del_Map();

  ~Game_Manager();
};

#endif
