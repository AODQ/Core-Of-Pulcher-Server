/* (c) CiggBit. All rights reserved.
 * See README.txt for more information
 */
 
#include "Game_Manager.h"
#include "NetObj.h"
#include "Player.h"
#include "Server_Vars.h"

#include <fstream>
#include <iostream>
#include <sstream>


Game_Manager::Game_Manager() {
  projectiles = nullptr;
  curr_map    = nullptr;
  for ( int i = 0; i != 16; ++ i )
    players[i]  = nullptr;
  server_timer = 0;
}

// load map utilities (Whitespace_Extract)
static std::string WS_Extract(std::string& t, int& it) {
  std::string res = "";
  if ( it >= t.size() ) return "";
  while ( t[it] == ' ' || t[it] == '\n' ) {
    ++ it;
    if ( it >= t.size() ) return "";
  }
  while ( t[it] != ' ' || t[it] == '\n' ) {
    res += t[it];
    ++it;
    if ( t.size() <= it ) return res;
  }
  return res;
}

static void Skip_WS(std::string& t, int& it) {
  while ( t[it] == ' ' ||
          t[it] == '\n' )
    ++it;
}

#define WS_EXTRACT_ASSERT(X) if ( temp == "" ) {\
  std::cout << "Error extracting " << X << '\n';\
  SV::map_name = "";\
  return;\
}

void Game_Manager::Load_Map(const char* m_name) {
  if ( curr_map != nullptr )
    delete curr_map;
  curr_map = new Map();
  SV::map_name = m_name;
  std::ifstream fil(SV::map_name, std::ios::binary);
  char c;
  // get version
  std::string ver = "";
  while ( true ) {
    fil.read((char*)&c, sizeof c);
    if ( c == ' ' ) break;
    ver += c;
  }
  if ( std::stof(ver)*10 < SUPPORTED_MAP_VERSION_MIN ) {
    std::cout << "Map version " << ver << " outdated (min version allowed: "
              << SUPPORTED_CLIENT_VERSION_MIN << ") \n";
    SV::map_name = "";
    return;
  }
  if ( std::stof(ver)*10 > SUPPORTED_MAP_VERSION_MAX ) {
    SV::map_name = "";
    std::cout << "Map version " << ver << " invalid (max version allowed: "
              << SUPPORTED_CLIENT_VERSION_MAX << ")\nTry updating the server\n";
    return;
  }
  // skip sheet info
  while ( c != '^' ) {
    fil.read((char*)&c, sizeof c);
  }
  // skip background info
  while ( c != '&' ) {
    fil.read((char*)&c, sizeof c);
  }
  // scripts (skip for now)
  while ( c != '@' ) {
    fil.read((char*)&c, sizeof c);
  }
  // load data
  std::stringstream map_data_ss(std::ios::binary | std::ios::in | std::ios::out);
  bool first_tripped = 0;
  while ( 1 ) {
    fil.read(      (char*)&c, sizeof c);
    if ( c == '\n' ) continue;
    map_data_ss.write((char*)&c, sizeof c);
    if ( fil.peek() == '@' )
      if ( first_tripped ) break;
      else first_tripped = 1;
  }
  fil.close();

  /////////////////////////////////////////////
  /////////// map data ////////////////////////
  std::string map_data = map_data_ss.str();
  auto& mtiles = curr_map->map_tiles;
  map_data_ss.clear();
  int map_it = 0;
  // map name
  SV::true_map_name = "";
  while ( map_data[map_it] == ' ' ) ++ map_it;
  while ( map_data[map_it] != '@' )
    SV::true_map_name += map_data[map_it++];
  ++map_it;
  
  // map width/height
  std::string temp;
  temp = WS_Extract(map_data, map_it);
  WS_EXTRACT_ASSERT("map width");
  mtiles.resize(std::stoi(temp)/32 * 2);
  temp = WS_Extract(map_data, map_it);
  WS_EXTRACT_ASSERT("map height");
  { // gen mtiles
    auto t_height = std::stoi(temp)/32 * 2;
    for ( int i = 0; i != mtiles.size(); ++ i )
      mtiles[i].resize(t_height);
  }
  // retrieve tile information
  int sheet_indices_amt = 0;
  while ( map_data[map_it] != '*' ) {
    Tile_Base tbase_temp;
    { // extract everything
      // sheet_index
      temp = WS_Extract(map_data,map_it);
      WS_EXTRACT_ASSERT("sheet index");
      try {
        tbase_temp.sheet_index = std::stoi(temp);
      } catch ( const std::invalid_argument& e ) {
        temp = "";
        WS_EXTRACT_ASSERT("sheet index");
      }
      // tile index
      temp = WS_Extract(map_data,map_it);
      WS_EXTRACT_ASSERT("tile index");
      try {
        tbase_temp.tile_index = std::stoi(temp);
      } catch ( const std::invalid_argument& e ) {
        temp = "";
        WS_EXTRACT_ASSERT("tile index");
      }
      // x position
      temp = WS_Extract(map_data, map_it);
      WS_EXTRACT_ASSERT("x position");
      try {
        tbase_temp.Set_Position(std::stoi(temp), 0);      
      } catch ( const std::invalid_argument& e ) {
        temp = "";
        WS_EXTRACT_ASSERT("x position");
      }
      // y position
      temp = WS_Extract(map_data, map_it);
      WS_EXTRACT_ASSERT("y position");
      try {
        tbase_temp.Set_Position(tbase_temp.R_Position().x, std::stoi(temp));
      } catch ( const std::invalid_argument& e ) {
        temp = "";
        WS_EXTRACT_ASSERT("y position");
      }
      // skip layer
      temp = WS_Extract(map_data,map_it);
      WS_EXTRACT_ASSERT("layer");
      // rotation
      temp = WS_Extract(map_data, map_it);
      WS_EXTRACT_ASSERT("rotation");
      try {
        tbase_temp.Set_Rotation(std::stoi(temp));
      } catch ( const std::invalid_argument& e ) {
        temp = "";
        WS_EXTRACT_ASSERT("rotation");
      }
      // flipx
      temp = WS_Extract(map_data, map_it);
      WS_EXTRACT_ASSERT("flip x");
      try {
        tbase_temp.flip_x = std::stoi(temp);
      } catch ( const std::invalid_argument& e ) {
        temp = "";
        WS_EXTRACT_ASSERT("flip x");
      }
      // flipy
      temp = WS_Extract(map_data, map_it);
      WS_EXTRACT_ASSERT("flip y");
      try {
        tbase_temp.flip_y = std::stoi(temp);
      } catch ( const std::invalid_argument& e ) {
        temp = "";
        WS_EXTRACT_ASSERT("flip y");
      }
      // skip visible
      temp = WS_Extract(map_data,map_it);
      WS_EXTRACT_ASSERT("visibility");
      // collideable
      temp = WS_Extract(map_data,map_it);
      WS_EXTRACT_ASSERT("collideable");
      try {
        tbase_temp.collideable = std::stoi(temp);
       } catch ( const std::invalid_argument& e ) {
        temp = "";
        WS_EXTRACT_ASSERT("collideable");
       }
      // tags ( skip for now )
      while ( map_data[map_it] == ' ' ||
              map_data[map_it] == '\n' ) ++ map_it;
      if ( map_data[map_it] == '{' ) {
        while ( map_data[map_it] != '}' ) ++map_it;
      }
    }
    // insert tile into map
    int px = tbase_temp.R_Position().x/32, py = tbase_temp.R_Position().y/32;
    tbase_temp.Set_Size(32, 32);
    // layer is irrelevant
    mtiles[px][py].push_back(tbase_temp);
    // check sheet index
    if ( (tbase_temp.sheet_index) + 1 > sheet_indices_amt )
      sheet_indices_amt = (tbase_temp.sheet_index) + 1;
  }
  ++map_it;
  // spawners ( TODO )

  // ----- extract tile information
  Skip_WS(map_data, map_it);
  while ( map_data[map_it] == '*' ) ++map_it;
  Skip_WS(map_data, map_it);
  while ( map_data[map_it] == '*' ) ++map_it;
  // gen sheets
  Sheet* sheets = new Sheet[sheet_indices_amt];
  
  int it = 0;
  // read info
  while ( true ) {
    sheets[it].width  = std::stoi(WS_Extract(map_data, map_it));
    sheets[it].height = std::stoi(WS_Extract(map_data, map_it));
    auto& polys = sheets[it].tile_polys;

    // read tile info
    while ( true ) {
      // check end (if not end then it's X)
      std::string end = WS_Extract(map_data, map_it);
      if ( end == ">>" ) break;
      int t = std::stoi(WS_Extract(map_data, map_it)); // grab Y
      const int X = std::stoi(end), Y = t;
      // store ID
      std::pair<int, AOD::PolyObj> pair = {sheets[it].width/32 * Y + X,
                                                      AOD::PolyObj()};
      std::vector<int> ints;
      while ( map_data[map_it] == ' ' ) ++ map_it;
      while ( map_data[map_it] != '*' ) {
        // grab ints
        std::string _int = "";
        while ( map_data[map_it] != ' ' )
          _int.push_back(map_data[map_it++]);
        // store ints
        ints.push_back(std::stoi(_int) - 16); // 16 OFFSET for middle
        //std::cout << _int << ' ';
        while ( map_data[map_it] == ' ' ) ++map_it;
      }
      ++map_it;
      //std::cout << '\n';
      Skip_WS(map_data, map_it);
      // make sense of ints vector
      std::vector < AOD::Vector > aodvecs;
      for ( int i = 0; i < ints.size()-1; i += 2 ) {
        aodvecs.push_back( {(float)ints[i], (float)ints[i+1]} );
        //std::cout << ints[i] << ", " << ints[i+1] << " :: ";
      }
      // set tile
      pair.second.Set_Vertices(aodvecs);
      polys.push_back( pair );
    }
    //std::cout << "END SHEET -----\n";
    // check if end of file
    Skip_WS(map_data, map_it);
    if ( map_data[map_it] == '$' ) break;
    ++it;
  }

  // Iterate through all tiles and assign polygon
  for ( int x = 0; x != curr_map->R_Width(); ++ x ) {
    for ( int y = 0; y != curr_map->R_Height(); ++ y )
    for ( int z = 0; z != curr_map->R_Layer(x, y); ++ z ) {
      auto til = curr_map->R_Tile(x, y, z);
      // assign tile info
      if ( til->collideable ) {
        auto& polys = sheets[til->sheet_index].tile_polys;
        til->Set_Vertices(polys[til->tile_index].second.R_Vertices(), 0);
      }
    }
  }
  delete[] sheets;
}

void Game_Manager::Add_Player() {

}

void Game_Manager::Update() {
  ++ SV::cl_update_rate_count;
  // -- update players
  for ( int i = 0, p = 0; p < SV::clients_connected; ++ i )
    if ( players[i] ) {
      players[i]->Update_Data();
      ++ p;
    }
}

void Game_Manager::Del_Map() {
  delete curr_map;
  curr_map = nullptr;
}

Game_Manager::~Game_Manager() {
  Del_Map();
}
