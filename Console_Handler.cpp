#include <iostream>
#include <fstream>

#include "Console_Utility.h"
#include "Game_Manager.h"
#include "Network.h"
#include "NetObject_Types.h"
#include "NetObj.h"
#include "Network.h"
#include "Player.h"
#include "Realm.h"
#include "Server_Vars.h"
#include "Utility.h"


static std::string R_Next_Parameter(std::string& str) {
  std::string arg = "";
  if ( str.size() == 0 ) return "";
  while ( str[0] == ' ' ) str.erase(str.begin(), str.begin() + 1);
  if ( str[0] == '"' ) { // look until end of \"
    str.erase(str.begin(), str.begin()+1);
    while ( str.size() != 0 && str[0] != '"' ) {
      arg += str[0];
      str.erase(str.begin(), str.begin()+1);
    }
    if ( str.size() != 0 ) // remove \"
      str.erase(str.begin(), str.begin()+1);
  } else { // look until end of space
    while ( str.size() != 0 && str[0] != ' ' ) {
      arg += str[0];
      str.erase(str.begin(), str.begin()+1);
    }
  }
  if ( str.size() != 0 ) // remove space if one exists
    str.erase(str.begin(), str.begin()+1);
  return arg;
}

static std::string R_Resize(std::string z, int t) {
  z.resize(t);
  return z;
}

template <typename T>
static std::string R_Resize(T x, int t) {
  return R_Resize(std::to_string(x),t);
}

void Execute_Command(std::string command) {
  std::string arg1 = R_Next_Parameter(command), arg2, arg3, arg4, arg5;
  
  for ( auto& i : arg1 ) i = tolower(i);

  // ---- zero argument -------------------------------------------------------
  if ( arg1 == "quit" || arg1 == "exit" ) {
    SV::server_running = 0;
    return;
  }
  if ( arg1 == "help" ) {
    std::cout << SV::console_help_msg << '\n';
    return;
  }

  if ( arg1 == "printvars" ) {
    for ( auto i : Player_Consts::floats ) {
      std::string& t = i.second + ":";
      t.resize(20);
      std::cout << t << ' ' << *i.first << '\n';
    }
  }

  if ( arg1 == "clear" ) {
    #ifdef WIN32
      system("CLS");
    #endif
    return;
  }

  if ( arg1 == "list_netobjs" ) {
    int tot_netobjs = AODP::Net::R_True_Amt_Netobjs();
    std::cout << "------------------------------------------------------\n";
    std::cout << "Total netobjs: " << tot_netobjs << '\n';
    AODP::Net::Call_Func_On_Every_Netobj([](const AODP::NetObj* netobj) {
      auto otype = netobj->R_Obj_Type();
      NetObj_Type ntype = static_cast<NetObj_Type>(otype);
      std::cout << "#"      << netobj->R_NetID()             << "  "
                << "Type: " << otype                         << " "
                << "( "     << NetObj_Types_To_String(ntype) << ") "
                << "Flag: " << netobj->R_Obj_Flag()          << '\n';
    });
    std::cout << "------------------------------------------------------\n";
    return;
  }

  // ------- debug one arg ----------------------------------------------------
  if ( arg1 == "pl_status" ) {
    std::cout << "--------------------  player  status  -------------------\n";
    std::cout << "ID  NAME        HP    AP    LIQ   INAIR PX  PY  VX  VY\n";
    for ( int i = 0; i != Max_users; ++ i ) {
      if ( game_manager->players[i] ) {
        auto pl = game_manager->players[i]->pl_handle;
        std::cout << R_Resize(i, 2) << ": ";
        if ( pl ) {
          std::cout << R_Resize(game_manager->players[i]->name,11) << ' '
                    << R_Resize(pl->R_Health()    ,5) << ' '
                    << R_Resize(pl->R_Armour()    ,5) << ' '
                    << R_Resize(pl->R_In_Liquid() ,5) << ' '
                    << R_Resize(pl->R_In_Air()    ,5) << ' '
                    << R_Resize(pl->R_Position().x,4) << ' '
                    << R_Resize(pl->R_Position().y,4) << ' '
                    << R_Resize(pl->R_Velocity().x,4) << ' '
                    << R_Resize(pl->R_Velocity().y,4) << ' ';
        } else {
          if ( game_manager->players[i]->status == PlayerStatus::dead )
            std::cout << "DEAD";
          if ( game_manager->players[i]->status == PlayerStatus::spectator )
            std::cout << "SPECTATOR";
          if ( game_manager->players[i]->status == PlayerStatus::nil )
            std::cout << "NIL";
        }
        std::cout << '\n';
      }
    }
    std::cout << "---------------------------------------------------------\n";
    return;
  }
  if ( arg2 == "mapsize" ) {
    std::cout << game_manager->curr_map->R_Width()  << ", "
              << game_manager->curr_map->R_Height() << '\n';
    return;
  }
  if ( arg2 == "mapname" ) {
    std::cout << "File: " << SV::map_name << '\n'
              << "True: " << SV::true_map_name << '\n';
    return;
  }


  // ---- one argument --------------------------------------------------------
  arg2 = R_Next_Parameter(command);
  if ( arg2 == "" ) goto INVALID_COMMAND;
  if ( arg1 == "say" ) {
    PulNet::Send_Packet("say " + arg2, 0);
    return;
  }

  if ( arg1 == "savevars" ) {
    std::ofstream fil("docs\\" + arg2);
    for ( auto i : Player_Consts::floats ) {
      fil << i.first << ' ' << i.second << '\n';
    }
    fil.close();
    return;
  }

  if ( arg1 == "system" ) {
    #ifdef WIN32
      system(arg2.c_str());
    #endif
    return;
  }

  if ( arg1 == "printip" ) {
    std::cout << "Port: " << PulNet::server_address.port << '\n'
              << "IP: "   << PulNet::server_address.host << '\n';
    return;
  }

  if ( arg1 == "kick" ) {
    int uid;
    try {
      uid = std::stoi(arg2);
    } catch ( ... ) {
      std::cout << "Invalid parameter " << arg2 << '\n';
      return;
    }
    if ( game_manager->players[uid] == nullptr ) {
      std::cout << "User " << uid << " doesn't exist\n";
      return;
    }
    PulNet::Send_Packet("GTFO", uid);
    std::cout << "Disconnecting user " << uid << '\n';
    return;
  }

  if ( arg1 == "forcekick" ) {
    int uid;
    try {
      uid = abs(std::stoi(arg2));
    } catch ( ... ) {
      std::cout << "Invalid parameter " << arg2 << '\n';
      return;
    }
    if ( game_manager->players[uid] == nullptr ) {
      std::cout << "User " << uid << " doesn't exist\n";
      return;
    }
    //enet_peer_disconnect
    return;
  }

  // ---- two argument --------------------------------------------------------
  arg3 = R_Next_Parameter(command);
  if ( arg3 == "" )
    goto INVALID_COMMAND;

  if ( arg1 == "setvar" ) {
    // grab variable to manipulate
    int pos = Util::Str_To_T<int>(arg2);
    if ( pos >= 0 && pos < Player_Consts::floats.size() )
      *Player_Consts::floats[pos].first = Util::Str_To_T<float>(arg3);
    std::cout << "Set " << arg2 << " to " << arg3 << '\n';
    return;
  }
  
  // ---- three arguments -----------------------------------------------------
  arg4 = R_Next_Parameter(command);
  if ( arg1 == "setpos" ) {
    int uid;
    try {
      uid = std::stoi(arg2);
    } catch ( ... ) {
      std::cout << arg2 << " not a valid parameter ( needs to be an int )\n";
      return;
    }

    if ( game_manager->players[uid] == nullptr ||
         game_manager->players[uid]->pl_handle == nullptr ) {
      std::cout << arg2 << " not a valid ID\n";
      return;
    }

    try {
      game_manager->players[uid]->pl_handle->Set_Position(
        std::stoi(arg3), std::stoi(arg4));
    } catch ( ... ) {
      std::cout << arg3 << ", " << arg4
                << " not valid parameter ( needs to be ints )\n";
    }
    return;
  }


  // ---- four arguments ------------------------------------------------------
  arg5 = R_Next_Parameter(command);
  if ( arg1 == "new_netobj" ) {
    auto  type = Util::Str_To_T<uint16_t>(arg2);
    auto     x = Util::Str_To_T<int32_t>(arg3);
    auto     y = Util::Str_To_T<int32_t>(arg4);
    auto flags = Util::Str_To_T<uint8_t>(arg5);
    auto aobj = new AOD::AABBObj({32, 32}, {x, y});
    AOD::Add(aobj);
    auto nobj = new AODP::NetObj(0, type, aobj, 0);
    AODP::Net::Add_NetObj(nobj);
  }


  INVALID_COMMAND:
  if ( arg1 != "" )
    std::cout << "Command " << arg1
              << " invalid or not fully implemented yet\n";
}
