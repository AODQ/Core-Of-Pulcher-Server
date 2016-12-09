/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#include "Server_Vars.h"
#include "Game_Manager.h"

uint8_t Max_users = MAX_ALLOWED_USERS;
Game_Manager* game_manager = nullptr;
namespace SV {
  std::string map_name = "";
  std::string true_map_name;
  std::string map_md5_hash = "";
  uint8_t clients_connected = 0;
  int cl_update_rate = 60;
  int cl_update_rate_count = 0;
  bool server_running = true;

  std::string console_help_msg = 
    "\n\n --- debug purpose --- \n"
    "d (type) -- outputs message to server.\n"
    "  -'pl_pos' -- prints all player positions\n"
    "  -'mapsize' -- prints map size\n"
    "  -'mapname' -- prints filename and true map name\n"
    "  -'pl_status' -- prints player health, armour and death\n"
    "  -'pl_keys' -- prints keystate 60 times at 1 s intervals\n"
    "  -'maxmaxclients' -- prints maximum amount of maximum amount of clients\n"
    "  -'maxclients' -- prints maximum amount of clients\n"
    "lag (ID) -- forces client to simulate lag\n"
    "system (command) -- calls Windows system command\n"
    " --- community purpose --- \n"
    "who -- prints all clients on server\n"
    "say (msg) -- outputs message to all clients\n"
    "kick (ID) -- boots player from server\n"
    "mute (ID) -- disables player from talking\n"
    "mutefish (ID) -- drops their keypresses\n"
    "move (ID) (X) (Y) -- moves player to new position\n"
    "mod (ID) -- grants moderator status to player\n"
    "maxclients (INT) -- set maximum amount of players\n"
    " --- general purpose -- \n"
    "quit/exit -- shuts server down\n"
    "changemap (map) -- changes map\n"
    "clear -- clears console\n"
    "printip -- prints ip and port used\n"
    /////////////////////////////////
    "\nExample: \"move 0 250 295\"\n";
}
