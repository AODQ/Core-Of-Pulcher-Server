/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef MAP_H_
#define MAP_H_
#pragma once
#include "Object.h"
#include <vector>

class Game_Manager;
class Tile_Base;
class Tile_Sheet_Container;
class Spawner;
class Sheet;

class Map {
  bool Is_Valid(int, int);
  
  std::vector<Tile_Base*> map_tiles_ref; // tiles that need to run every frame
  std::vector<std::vector<std::vector<Tile_Base>>> map_tiles;

  std::vector<Spawner> spawners;
  std::vector<Spawner*> player_spawns;
  Sheet* sheets; // only purpose is to dealloc mem
  int sheet_size;
  friend Game_Manager;
public:
  // failure/successful (bool)
  // int pos x, y, width, height
  // Returns vector containing all tiles within specified area
  std::vector<Tile_Base*> R_Tile_Vec(int,int,int,int);
  std::vector<Tile_Base*>& R_Tiles_Ref();

  int R_Height();
  int R_Width();
  int R_Layer(int x, int y);

  void Set_Sheet(Sheet*, int);

  Tile_Base * R_Tile(int x, int y, int z = 0);
  
  void Update();

  std::vector<Tile_Sheet_Container>& R_Tile_Sheets();

  std::vector<Spawner >* R_Spawners();
  std::vector<Spawner*>* R_Player_Spawners();

  ~Map();
};


class Sheet {
public:
  int width, height;
  std::vector < std::pair< int, AOD::PolyObj >> tile_polys;
};

class Tile_Base : public AOD::PolyObj {
public:
  std::string script_name;
  bool flip_x, flip_y;
  bool collideable, affect_player,
       triggered_by_player, step_down_platform;
  int sheet_index, tile_index;
};

class Spawner : public AOD::Object {
  enum class Type {
    hp_shard, hp_capsule, hp_mega,
    ar_shard, ar_bronze, ar_silver, ar_gold,
    p_strength, p_velocity, p_immortality, p_time,
    we_fetus, we_doppler, we_grannibal, we_manshredder, we_pericaliya, we_pmf, we_tornet, we_volnias, we_zeus,
    bx_fetus, bx_doppler, bx_grannibal, bx_manshredder, bx_pericaliya, bx_pmf, bx_volnias, bx_zeus, bx_tornet,
    fl_flag, fl_red, fl_blue,
    pl_right, pl_right_red, pl_right_blue, pl_left, pl_left_red, pl_left_blue, _SIZE
  };
  // animation type
  static void* images[(unsigned int)Type::_SIZE];

  Type type;
  bool active;
  int refresh_time;
};

#endif