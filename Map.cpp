/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#include "Map.h"

std::vector<Tile_Base*> Map::R_Tile_Vec(int x, int y, int w, int h) {
  std::vector<Tile_Base*> vec;
  if ( x+w < 0 || y+h < 0 ) return vec;


  int end_x = float(x+w)/32.f + 2.5f,
      end_y = float(y+h)/32.f + 2.5f;
  x -= 3; y -= 3;
  x /= 32; y /= 32;
  if ( x < 0 ) x = 0;
  if ( y < 0 ) y = 0;
  // grab all tiles
  for ( int i_x = x; i_x != end_x && i_x < map_tiles.size();    ++ i_x )
  for ( int i_y = y; i_y != end_y && i_y < map_tiles[0].size(); ++ i_y ) {
    for ( auto& i : map_tiles[i_x][i_y] )
      vec.push_back(&i);
  }
  return vec;
}

int Map::R_Height() {
  if ( map_tiles.size() > 0 )
    return map_tiles[0].size();
  return 0;
}

int Map::R_Width() {
  return map_tiles.size();
}

int Map::R_Layer(int x, int y) {
  if ( map_tiles.size() > x && map_tiles[x].size() > y )
    return map_tiles[x][y].size();
  return 0;
}

Tile_Base* Map::R_Tile(int x, int y, int z) {
  if ( map_tiles.size() > x && map_tiles[x].size() > y && map_tiles[x][y].size() > z )
    return &map_tiles[x][y][z];
  return nullptr;
}

Map::~Map() {

}