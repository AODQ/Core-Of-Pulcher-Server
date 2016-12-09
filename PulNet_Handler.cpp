#include "NetObject_Types.h"
#include "PulNet_Handler.h"
#include "Player.h"
#include "NetObj.h"
#include "Utility.h"
#include "Server_Vars.h"

#include <sstream>

// ------ static utils --------------------------------------------------------
// returns next parameter from a string received from ENET packet
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

// ------- pulnet packet event builders ---------------------------------------
static void Build_Hard_Client_Refresh(PacketVec& vec, Player_Info* p_info) {
  vec = {(uint8_t)PulNet::PacketEvent::hard_client_refresh, p_info->uid };
  Util::Append_Pack_String(vec, p_info->name);
  Util::Append_Pack_String(vec, p_info->short_name);
  Util::Append_Pack_String(vec, R_Status_Str(p_info));
  Util::Prepend_Pack_Num(vec, 0);
  Util::Prepend_Pack_Num(vec, 0);
}

// ------- PulNet handler functions -------------------------------------------
void PulNet::Handle_Connect(ENetPeer* peer,
                            ENetPeer* client_peer[MAX_ALLOWED_USERS]) {
  if ( SV::clients_connected >= Max_users ) // too many users
    goto NO_ROOM_LEFT;

  peer->data = nullptr;
  for ( uint8_t i = 0; i != Max_users; ++ i ) { // find slot for new player
    if ( game_manager->players[i] == nullptr ) {
      peer->data = (void*)new uint8_t(i);
      break;
    }
  }

  if ( peer->data == nullptr ) {
    NO_ROOM_LEFT:
    /*
    std::cout << "No slots left to connect client, now asking to disconnect";
    std::string gtfo = "";
    gtfo.push_back((char)PacketEvent::gtfo);
    PulNet::Send_Packet(gtfo.c_str(), 1, peer, ENET_PACKET_FLAG_RELIABLE);
    */
    return;
  }
  
  // client successfully connected
  ++SV::clients_connected;
  uint8_t uid = *(uint8_t*)(peer->data);
  game_manager->players[uid] = new Player_Info(uid);
  client_peer[uid] = peer;
  
  std::cout << "User (ID " << uid << ") connected from "
            << Util::R_IP(peer->address.host) << ":"
            << peer->address.port  << "\n";
  // notify client of current players, dismiss self
  Util::Loop_Through_Clients([&](Player_Info* pl_info) {
    if ( pl_info->uid == uid ) return;
    PacketVec pl_pack;
    Build_Hard_Client_Refresh(pl_pack, pl_info);
    PulNet::Send_Packet(pl_pack, uid);
  });
  { // connection event
    PacketVec connection = { uid };
    Util::Prepend(connection, Util::Pack_String(SV::map_name));
    // Util::Prepend(connection, SV::map_md5_hash);

    for ( const auto& i : Player_Consts::floats ) {
      Util::Prepend_Pack_Num(connection, *i.first);
    }
    PulNet::Send_Packet_Event(PacketEvent::connection, connection, uid);
    std::cout << "SENDING CONNECTION PACKET\n";
  }
}

void PulNet::Handle_Disconnect(ENetPeer* peer) {
  int uid = *((uint8_t*)(peer->data));
  if ( uid < 0 || uid > 100 ) return; // player quit immediately
  delete peer->data;
  peer->data = nullptr;
  SV::clients_connected -= 1;
  if ( game_manager->players[uid] ) {
    if ( game_manager->players[uid]->pl_handle ) {
      delete game_manager->players[uid]->pl_handle;
    }
    delete game_manager->players[uid];
  }
  game_manager->players[uid] = nullptr;
  enet_peer_disconnect_now(peer, 0);
}

void PulNet::Send_All_NetObjs_To_Client(uint8_t uid) {
  AODP::Net::Send_NetObjs_To_Client(uid);
}


void PulNet::Handle_Packet(ENetPacket* packet, ENetPeer* peer) {
  // get notation
  uint8_t* raw_data = packet->data;
  uint8_t base_event_type = raw_data[0];
  std::vector<uint8_t> data(raw_data+1, raw_data + packet->dataLength);
  auto uid = *(uint8_t*)peer->data;
  auto pl = game_manager->players[uid];
  int it = 0;
  if ( pl == nullptr ) {
    std::cout << "Player connected incorrectly, player info not initilaized\n";
    enet_peer_disconnect(peer, 0);
    return;
  }
  
  static const std::map<PacketEvent, std::string> packetevent_to_string {
    {PacketEvent::nil                 , "nil"                 },
    {PacketEvent::netObj_create       , "netObj_create"       },
    {PacketEvent::netObj_refresh      , "netObj_refresh"      },
    {PacketEvent::netObj_remove       , "netObj_remove"       },
    {PacketEvent::fire_event          , "fire_event"          },
    {PacketEvent::client_refresh      , "client_refresh"      },
    {PacketEvent::connection          , "connection"          },
    {PacketEvent::client_connection   , "client_connection"   },
    {PacketEvent::hard_client_refresh , "hard_client_refresh" },
    {PacketEvent::query               , "query"               }
  };


  /* std::cout << "Received packet type " */
  /*           << packetevent_to_string.at( (PacketEvent)base_event_type ) */
  /*           << " from user ID " << uid << '\n'; */
  /* std::cout << "Contents: "; */

  /*for ( auto i : data )
    std::cout << i << "/" << (int)i << ", ";*/
  std::cout << "(size " << data.size() << ")\n";

  // util functions
  static const auto String_To_Playerteam =
  [&](const std::string& str) {
    return str == "red"  ? PlayerTeam::red  :
           str == "blue" ? PlayerTeam::blue : PlayerTeam::none;
  };
  static const auto String_To_Playerstatus =
  [&](const std::string& str) {
    std::cout << "\n" << "((((" << str << "))))" << '\n';
    return  str == "dead"      ? PlayerStatus::dead      :
            str == "alive"     ? PlayerStatus::alive     :
            str == "spectator" ? PlayerStatus::spectator :
                                 PlayerStatus::nil;
  };
  // actual logic
  switch ( (PacketEvent)base_event_type ) {
    case PacketEvent::hard_client_refresh:{
      auto lhn = Util::Unpack_Str(data, it);
      auto shn = Util::Unpack_Str(data, it);
      auto status = Util::Unpack_Str(data, it);
      auto character = Util::Unpack_Num<uint8_t>(data, it);
      auto colour    = Util::Unpack_Num<uint8_t>(data, it);

      std::cout << "Received HCR @ Handle_Packet from user #" << uid << '\n';
      std::cout << " -name:   " << lhn       << '\n'
                << " -shname: " << shn       << '\n'
                << " -status: " << status    << '\n'
                << " -char:   " << character << '\n'
                << " -colour: " << colour    << '\n';
      
      pl->name       = lhn;
      pl->short_name = shn;
      pl->character  = character;
      pl->colour     = colour;

      std::stringstream ss(status);
      std::string playerstatus, teamstatus;
      ss >> playerstatus >> teamstatus;
      std::cout  << " -teamstatus:   "  << "(" << teamstatus   << ")\n"
                 << " -playerstatus: "  << "(" << playerstatus << ")\n";
      auto prev_status = pl->status; // in few LOC we need to check to refresh
                                     // netobjs
      pl->team   = String_To_Playerteam(teamstatus);
      pl->status = String_To_Playerstatus(playerstatus);
      std::cout << " -teamstatus: " << (int)pl->team   << '\n'
                << " -status:     " << (int)pl->status << '\n';
      { // broadcast HCR/connection, have to modify original a bit though
        std::vector<uint8_t> ins {(uint8_t)PacketEvent::hard_client_refresh,
                                   uid};
        data.insert(data.begin(), ins.begin(), ins.end());
        PulNet::Broadcast_Skip fn = [&](uint8_t id) { return uid == id; };
        PulNet::Broadcast_Packet(data, fn);
      }
      // check if we should start refreshing netobjects
      if ( pl->refresh_netobjs == 0 && pl->status != PlayerStatus::nil ) {
        pl->refresh_netobjs = 1;
        PulNet::Send_All_NetObjs_To_Client(uid);
      }
      std::cout << " -previous status: "
                << ( prev_status == PlayerStatus::nil ? "nil" : "else" ) << '\n'
                << " -ref netobj: " << (pl->refresh_netobjs ? "Y" : "N") << '\n'
                << "--------------------------\n";
    }break;
    case PacketEvent::query:{
      static std::map<PacketQuery, std::string> packetquery_to_string {
        {PacketQuery::specific_player_info,  "specific_player_info" },
        {PacketQuery::team_info,             "team_info"            },
        {PacketQuery::join_game,             "join_game"            }
      };
      PacketQuery pquery = static_cast<PacketQuery>(data[it]);
      ++it;
      std::cout << "User " << uid << " queried: "
                << packetquery_to_string[pquery] << "\n";
      switch ( pquery ) {
        case PacketQuery::join_game:
          auto team = Util::Unpack_Str(data, it);
          std::cout << "User " << uid << " attempted to join " << team << '\n';
          // check if teams full ... later, for now just accept
          pl->team = String_To_Playerteam(team);
          // now broadcast HCR
          PacketVec hcr_ref;
          Build_Hard_Client_Refresh(hcr_ref, pl);
          PulNet::Broadcast_Packet(hcr_ref);
          // and let's just spawn the player, why not
          std::cout << "Spawning player, sending to client\n";
          AODP::Net::Add_NetAodObj(new Player(pl), NetObj_Type::player);
        break;
      }
    }break;
  }
  /*
    case PacketEvent::listmap:
    break;
    case PacketEvent::join:
      std::cout << pl->name << " joined game\n";
      Broadcast_Packet("SAY -- " + pl->name + " joined the game");
      Broadcast_Packet_Event(PacketEvent::join, Util::Pack_Num(uid));
      pl->status = PlayerStatus::dead;
      pl->status_data.dead_data = PlayerStatusData_Dead();
    break;
    case PacketEvent::spec:
      std::cout << pl->name << " joined spectator\n";

      Broadcast_Packet("SAY -- " + pl->name + " switched to spectator");
    break;
    case PacketEvent::vimap:
    break;
    case PacketEvent::ch_name:
      /*pl->name = args[0];
      while ( pl->name[pl->name.size()-1] == '\0' )
        pl->name.pop_back();
    break;
    case PacketEvent::say:
      // Broadcast_Packet("SAY " + pl->name + ": " + data);
    break;
    case PacketEvent::votekick:
    break;
    case PacketEvent::kick:
    break;
    case PacketEvent::ch_map:
    break;
    case PacketEvent::promote:
    break;
    case PacketEvent::demote:
    break;
    case PacketEvent::kill:
    break;
    case PacketEvent::votemap:
    break;
    case PacketEvent::whisper:
    break;
    case PacketEvent::keystate:
    {
      /*
      uint8_t t_stamp = data[0];
      if ( data.size() == 7 &&
           (t_stamp > pl->client_timestamp ||
           (200     < pl->client_timestamp && 100 > t_stamp ))) {
        pl->client_timestamp = data[0];
        game_manager->players[uid]->pl_handle->Set_Keys(
                              data[1], data[2], data[3]);
      } else
        t_stamp = data[0];
      *//*
    }
    break;
    case PacketEvent::spawntime:
      // user is asking for current spawn time
      if ( game_manager->players[uid]->status == PlayerStatus::dead ) {
        Send_Packet("SPAWNTIME " +
              std::to_string(
                game_manager->players[uid]->status_data.dead_data.spawn_time),
              uid);
      }
    break;
    case PacketEvent::netObj_create:
    case PacketEvent::netObj_remove:
      // this should never happen, reject
    break;*/
}
