/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#define NOMINMAX
#include <iostream>
#include <chrono>
#include <fstream>
#include <string>
#include <thread>

#include "Console_Handler.h"
#include "Console_Utility.h"
#include "Game_Manager.h"
#include "Network.h"
#include "NetObject_Types.h"
#include "Player.h"
#include "Realm.h"
#include "TestUnit.h"
#include "MatchTimer.h"

void Init() {
  game_manager = new Game_Manager();
  AOD::realm = new AOD::Realm();
}

void Clear_Input() {
  std::cin.clear();
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void Load_Map() {
  std::string input = "";
  Clear_Screen();
  Output_Map_Files();
  std::cout << "Enter map to load (Format: \"map_name\"):\n";
  bool quote_hit = 0;

  // Get map file
  /*do {
    auto i = std::cin.get(); // if " not hit yet we dispose anyways
    if ( i != '"' && quote_hit )
      input += i;
    if ( i == '"' )
      quote_hit = 1;
  } while ( !quote_hit || (quote_hit && std::cin.peek() != '"')  );
  Clear_Input();
*/
  input = "TestJump.pmd";
  // check if file exists
  if ( !std::ifstream(input).good() && input != "debug_skip" ) {
    std::cout << "Could not open map\n";
    return Load_Map();
  }

  // load the map
  std::cout << "Loading... ";
  game_manager->Load_Map(input.c_str());
  if ( SV::map_name == "" ) {
    std::cout << "Error loading map\n";
    return Load_Map();
  };

  // finished
  std::cout << "done loading map " << SV::true_map_name << '\n';

  // bring this to match object sometime in the future? away from source...
  std::cout << "Starting match\n";
  AODP::Net::Add_NetAodObj(new MatchTimer(), NetObj_Type::match_timer );

  /*// debug print
  std::cout << "Debug print map? (Y/N)" << '\n';
  if ( toupper(std::cin.get()) == 'Y' ) {
    for ( int x = 0; x != game_manager->curr_map->R_Height(); ++ x ) {
      std::cout << '\n';
      for ( int y = 0; y != game_manager->curr_map->R_Width(); ++ y ) {
        if ( game_manager->curr_map->R_Tile(y, x) )
          std::cout << 'x';
        else
          std::cout << ' ';
      }
    }
    std::cout << "\rDone printing map\n";
    Clear_Input();
  }*/
}

void Update() {
  float prev_dt = 0, // DT from prev frame
        curr_dt = 0,
        elapsed_dt = 0,
        accumulated_dt = 0; // DT needing to be processed


  using HiResClock = std::chrono::high_resolution_clock;
  using ms         = std::chrono::milliseconds;
  using duration   = std::chrono::duration<float, std::milli>;

  float network_update = 0.0f;
  auto time_zero = HiResClock::now();
  float timer = 0;
  while ( true ) {
    // refresh time handlers
    duration time_stamp = HiResClock::now()-time_zero;
    curr_dt = time_stamp.count();
    elapsed_dt = curr_dt - prev_dt;
    accumulated_dt += elapsed_dt;
    network_update += elapsed_dt;

    // refresh calculations
    while ( accumulated_dt >= SERVER_MS_DT ) {
      PulNet::Handle_Client_Input();
      accumulated_dt -= SERVER_MS_DT;

      // update world
      AOD::realm->Update();
    }
    // update network
    if ( network_update > SV::cl_update_rate ) {
      while ( network_update > SV::cl_update_rate )
        network_update -= SV::cl_update_rate;
      PulNet::Handle_Network();
    }
    // set current time mark
    prev_dt = curr_dt;
    // check if we need to exit
    if ( !SV::server_running )
      return;
  }
}


void Clean_Up() {
  delete game_manager;
}

#if _MSC_VER >= 1900
FILE _iob[] = {*stdin, *stdout, *stderr};
extern "C" FILE * __cdecl __iob_func(void){ return _iob; }

extern "C" void __cdecl __fprintf(void){ }
#endif


int main(int argc, char* argv[]) {
  std::cout << "Initializing server\n";  
  Init();
  PulNet::Init();
  player_consts_being_used.store(1);
  std::cout << "Server initialized.\n";
 TestUnits::Query_Test_Units();
  
  Load_Map();

  std::cout << "Now running server\n";
  {
    std::thread server_manager(Update);

    while ( SV::server_running ) {
      std::string user_input = "";
      while ( std::cin.peek() != '\n' ) user_input.push_back(std::cin.get());
      Clear_Input();
      Execute_Command(user_input);
    }
    server_manager.join();
  }
  Clean_Up();
  std::cin.get();
}
