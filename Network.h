/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef NETWORK_H_
#define NETWORK_H_
#pragma once

#include <enet/enet.h>
#include <map>
#include <utility>
#include <functional>
#include <vector>

#include "Server_Vars.h"

namespace PulNet {
  
  extern ENetAddress server_address;
  using Broadcast_Skip = std::function<bool(uint8_t)>;
  // use arrays so as to make it easier for concurrency
  extern ENetAddress client_ip[MAX_ALLOWED_USERS];
  enum class PacketEvent {
    nil,
    netObj_create, netObj_refresh, netObj_remove, fire_event,

    client_refresh,

    connection,

    client_connection, hard_client_refresh,

    query,
    size
  };

  enum class PacketQuery { // specific to "query" packet event
    // questions
    specific_player_info, team_info,
    // "demands"
    join_game
  };

  // will initialize the network library
  void Init();
  // Handle receiving networking until nothing to be done
  void Handle_Client_Input();
  // Handle sending networking
  void Handle_Network();

  // ---- utils ----
  void Send_Packet(const void* data, int len, uint8_t sizeof_data,
                   ENetPeer* peer, _ENetPacketFlag flag);
  // Sends packet to destination peer with data using specified flag
  void Send_Packet(const char* data, int len, ENetPeer* peer,
                                        _ENetPacketFlag flag);
  // Sends packet to destination peer based on user ID with data using
  // specified flag
  void Send_Packet(const char* data, int len, int uid, _ENetPacketFlag flag);
  // Sends packet to destination based on user ID with data using specified
  // flag, note that RELIABLE will more or less mean TCP
  void Send_Packet(const std::string& data, int uid,
            _ENetPacketFlag flag = ENET_PACKET_FLAG_RELIABLE);
  // sends packet to destination based on user ID with data using specified
  // flag. The advantage of using a vector over a string is that you can store
  // a null value and it will work just fine, unlike strings
  // note that RELIABLE will more or less mean TCP
  void Send_Packet(const std::vector<uint8_t>& data, int uid,
            _ENetPacketFlag flag = ENET_PACKET_FLAG_RELIABLE);
  // sends packet event to destination, note packet even is not pulcher event
  // Since packet event should be something that is gaurunteed to arrive at
  // the client, it uses ENET_PACKET_FLAG_RELIABLE
  void Send_Packet_Event(PacketEvent pe, std::vector<uint8_t>& data, int uid);
  // broadcasts packet event to all clients
  void Send_Packet_Event(PacketEvent pe, std::vector<uint8_t>& data, int uid);
  // Sends packet to all connected peers with data using specified flag
  // note that RELIABLE will more or less mean TCP
  void Broadcast_Packet(const std::string& data,
            _ENetPacketFlag flag = ENET_PACKET_FLAG_RELIABLE);
  // Sends packet to all connected peers with data using specified flag
  // The advantage of using a vector over a string is that you can store a
  // null value and it will work just fine, unlike stringss
  // note that RELIABLE will more or less mean TCP
  void Broadcast_Packet(const std::vector<uint8_t>& data,
            _ENetPacketFlag flag = ENET_PACKET_FLAG_RELIABLE);
  void Broadcast_Packet(const std::vector<uint8_t>& data,
                        std::function<bool(uint8_t)>& skip,
            _ENetPacketFlag flag = ENET_PACKET_FLAG_RELIABLE);
  // Senmds packet to all connected peers with data using specified flag.
  // Will prepend packet event to data
  void Broadcast_Packet_Event(PacketEvent pe, std::vector<uint8_t>& data,
            _ENetPacketFlag flag = ENET_PACKET_FLAG_RELIABLE);
  void Broadcast_Packet_Event(PacketEvent pe, std::vector<uint8_t>& data,
            std::function<bool(uint8_t)>& skip,
            _ENetPacketFlag flag = ENET_PACKET_FLAG_RELIABLE);
}

#endif


/*

- client reads his inputs, and calculates a local prediction of the player
  position.
- client sends inputs to the host, as well as his local prediction calculation.
  The input+prediction packet is marked with a unique number
  (aka Sequence Number, aka SQN).
- client also stores the input+prediction packet in a buffer.
- Host receives the client inputs+prediction+SQN. 
- host calculate the real player position, using those inputs.
- host then compares the player location with the client prediction.
- if a difference is detected (within an arbitrary margin of error),
  the host sends a correction packet back to the client (containing
  corrected_position+SQN).
- client receives the correction packet.
- client looks up the original input+prediction packet in his buffer,
  using the SQN. 
- client substitutes the predicted position with the corrected position.
- client then replays his input stack from that point onwards, to adjust his
  current prediction with the server correction.
https://developer.valvesoftware.com/wiki/Source_Multiplayer_Networking
*/
