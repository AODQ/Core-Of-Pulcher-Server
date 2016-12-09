/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef SERVER_CONSTS_H_
#define SERVER_CONSTS_H_
#pragma once

#include <string>
#include <vector>
#include "Game_Manager.h"

using PacketVec = std::vector<uint8_t>;
// server defined
#define SERVER_VERSION 0001
#define SERVER_MS_DT 15
#define PORT_NUMBER 9111
#define MAX_PACKET_LEN 1024
#define SUPPORTED_CLIENT_VERSION_MIN 0001
#define SUPPORTED_MAP_VERSION_MIN 0002
#define SUPPORTED_CLIENT_VERSION_MAX 0001
#define SUPPORTED_MAP_VERSION_MAX 0002
#define MAX_CHAT_LENGTH 40
// maximum amount of users allowed to assign to Max_users
#define MAX_ALLOWED_USERS 128

// client defined
extern uint8_t Max_users;

class Game_Manager;

// game related
extern Game_Manager* game_manager;
namespace SV {
  extern std::string map_name;
  extern std::string true_map_name;
  extern std::string map_md5_hash;
  extern uint8_t clients_connected;
  // Frames that should occur before an update occurs
  extern int cl_update_rate,
             cl_update_rate_count;


  extern std::string console_help_msg;


  extern bool server_running;
}

#define MS (17)
#define TO_MS(X) ((17.0f * float(X))/1000.0f)

#define KEY_JUMP     1
#define KEY_DASH     2
#define KEY_DOWN     4
#define KEY_LEFT     8
#define KEY_RIGHT   16
#define KEY_CROUCH  32
#define KEY_PRIM    64
#define KEY_SEC    128

#define KEY_MISC_ANGLE_UP    2
#define KEY_MISC_ANGLE_ZERO  4
#define KEY_MISC_ANGLE_DOWN  8
#define KEY_MISC_DIR_LEFT   16
#define KEY_MISC_DIR_RIGHT  32

#endif