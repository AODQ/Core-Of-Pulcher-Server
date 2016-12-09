/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */

#include "Network.h"
#include "NetObject_Types.h"
#include "Player.h"
#include "Server_Vars.h"
#include "Utility.h"

#include <map>
#include <string>
/*

Smilecythe
2:38 PM
the upper body would have to be at like 45 angle
when ground sliding
Infinine
2:38 PM
ok
Smilecythe
2:38 PM
and the side rotation would be kind of with that 45 angle as well

*/


// crouch 21:32
Player::Player(Player_Info* in) : AOD::PolyObj(), pl_info(in) {
  direction = in_air = jumping = wall_jumping = in_liquid = 0;
  key_up = key_crouch = key_dash = key_jump = key_down =
  key_left = key_right = key_prim = key_sec = 0;

  PolyObj::Set_Vertices({{-21/2, -49/2}, {-21/2,  49/2},
                         { 21/2,  49/2}, { 21/2, -49/2}});
  Set_Size(21, 49);

  health = 100;
  armour = 0;
  timer_fall_jump = -1;
  for ( int i = 0; i != (int)Direction::size; ++ i ) {
    air_dash_down[i] = 0;
    air_dash_up[i]   = 0;
    for ( int o = 0; o != (int)Angle::size;     ++ o )
      timer_dash[i][o] = 0.0f;
  }
  timer_keep_wj_vel = -1;
  timer_wj_last_dir = -1.f;
  jumping_free = 0;
  dashing_free = dashing = 0;
  crouch_slide_timer = -1.f;
  crouching = 0;
  Set_Position(0 ,0);
}

int Player::R_Health()      const { return health;     }
int Player::R_Armour()      const { return armour;     }
bool Player::R_In_Liquid()  const { return in_liquid;  }
bool Player::R_In_Air()     const { return in_air;     }
bool Player::R_Key_Up()     const { return key_up;     }
bool Player::R_Key_Down()   const { return key_down;   }
bool Player::R_Key_Left()   const { return key_left;   }
bool Player::R_Key_Right()  const { return key_right;  }
bool Player::R_Key_Sec()    const { return key_sec;    }
bool Player::R_Key_Prim()   const { return key_prim;   }
bool Player::R_Key_Crouch() const { return key_crouch; }
bool Player::R_Key_Dash()   const { return key_dash;   }
bool Player::R_Key_Jump()   const { return key_jump;   }

Player::Angle Player::R_Dash_Angle()   const { return dash_angle; }
Player::Direction Player::R_Dash_Dir() const { return dash_dir; }
Player::Angle Player::R_WJmp_Angle()   const { return wjmp_angle; }
Player::Direction Player::R_WJmp_Dir() const { return wjmp_dir; }

// ---- player consts ---------------------------------------------------------


float Player_Consts::max_ground_speed = 6.0f,
      Player_Consts::ground_friction  = .82f,
      Player_Consts::air_friction     = 1.0f,
      Player_Consts::accel_ground     = 8.25f,
      Player_Consts::accel_air        = 1.5f,
      Player_Consts::max_air_vel_x    = 15.0f,
      Player_Consts::max_air_vel_y    = 9.0f,
      Player_Consts::gravity          = 9.25f,
      Player_Consts::jump_force       = 4.5f,
      Player_Consts::dash_force       = 3.75f,
      Player_Consts::dash_timer       = 2000.f;

using pconst_fl_pair = std::pair<float*, std::string>;
const std::vector<std::pair<float*, std::string>> Player_Consts::floats {
  { pconst_fl_pair(&Player_Consts::max_ground_speed, "max ground speed") },
  { pconst_fl_pair(&Player_Consts::ground_friction , "ground friction") },
  { pconst_fl_pair(&Player_Consts::air_friction    , "air friction") },
  { pconst_fl_pair(&Player_Consts::accel_ground    , "ground acceleration") },
  { pconst_fl_pair(&Player_Consts::accel_air       , "air acceleration") },
  { pconst_fl_pair(&Player_Consts::max_air_vel_x   , "max air Y velocity") },
  { pconst_fl_pair(&Player_Consts::max_air_vel_y   , "max air X velocity") },
  { pconst_fl_pair(&Player_Consts::gravity         , "gravity") },
  { pconst_fl_pair(&Player_Consts::jump_force      , "jump force") },
  { pconst_fl_pair(&Player_Consts::dash_force      , "dash force") },
  { pconst_fl_pair(&Player_Consts::dash_timer      , "dash timer") }
};

// ---- player info -----------------------------------------------------------

std::atomic<bool> player_consts_being_used = 1;

const std::string playerstatus_str[(int)PlayerStatus::size] {
  "dead", "alive", "spectator", "nil"
};

const std::string playerteam_str[(int)PlayerTeam::size] {
  "none", "blue", "red"
};


std::string R_Status_Str(Player_Info* pl) {
  return R_Player_Status_Str(pl) + ' ' + R_Team_Status_Str(pl);
}

std::string R_Player_Status_Str(Player_Info* pl) {
  return playerstatus_str[(int)pl->status];
}

std::string R_Team_Status_Str(Player_Info* pl) {
  return playerteam_str[(int)pl->team];
}


Player_Info::Player_Info(unsigned int _uid, std::string _name,
                         PlayerStatus _ps) : uid(_uid) {
  name = short_name = "";
  status = _ps;
  status_data.nil = nullptr;
  team = PlayerTeam::none;
  pl_handle = nullptr;
  client_timestamp = 0;
  refresh_netobjs = 0;
}

void Player_Info::Update_Data() {
  switch ( status ) {
    case PlayerStatus::dead:
      status_data.dead_data.spawn_time -= float(SERVER_MS_DT)/1000.f;
    break;
    case PlayerStatus::alive:
      if ( !pl_handle ) { // something fucked up
        status = PlayerStatus::dead;
        return;
      }
      // wait until consts not being used
      while ( player_consts_being_used.load() == 0 );
      player_consts_being_used.store(0);
      pl_handle->Update();
      player_consts_being_used.store(1);
    break;
  }
}
PlayerStatusData::PlayerStatusData() {
  nil = nullptr;
}
PlayerStatusData_Dead::PlayerStatusData_Dead() {
  spawn_time = 3.0f;
}


// ---- player update ---------------------------------------------------------

void Horiz_Key_Input();

void Player::Update() {
  Update_Velocity();

  Update_Pre_Var_Set();
  Update_Crouch();
  Update_Horiz_Key_Input();
  Update_Dash();
  Update_Jump();
  Update_Wall_Jump();
  Update_Gravity();

  // ---- send state to client ----------------------------
  std::string packet = "! ";
  char buffer[100];
  sprintf_s(buffer, 100, "! %d %.3f %.3f %.3f %.3f %d",
    pl_info->uid, position.x, position.y, velocity.x, velocity.y, crouching);

  PulNet::Broadcast_Packet(buffer, ENET_PACKET_FLAG_UNSEQUENCED);
}

void Player::Set_Key_Up    (bool key) { key_up     = key; }
void Player::Set_Key_Down  (bool key) { key_down   = key; }
void Player::Set_Key_Left  (bool key) { key_left   = key; }
void Player::Set_Key_Right (bool key) { key_right  = key; }
void Player::Set_Key_Sec   (bool key) { key_sec    = key; }
void Player::Set_Key_Prim  (bool key) { key_prim   = key; }
void Player::Set_Key_Crouch(bool key) { key_crouch = key; }
void Player::Set_Key_Dash  (bool key) { key_dash   = key; }
void Player::Set_Key_Jump  (bool key) { key_jump   = key; }

static Player::Angle R_Angle_From_Mask(unsigned int mask) {
  switch ( mask ) {
    case KEY_MISC_ANGLE_DOWN:
      return Player::Angle::Down;
    break;
    case KEY_MISC_ANGLE_UP:
      return Player::Angle::Up;
    break;
    case KEY_MISC_ANGLE_ZERO:
      return Player::Angle::Zero;
    break;
  }
  return Player::Angle::size;
}

static Player::Direction R_Dir_From_Mask(unsigned int mask) {
  switch ( mask ) {
    case KEY_MISC_DIR_LEFT:
      return Player::Direction::Left;
    break;
    case KEY_MISC_DIR_RIGHT: default:
      return Player::Direction::Right;
    break;
  }
  return Player::Direction::size;
}

void Player::Set_Keys(char keys, char dash, char wjmp) {
  key_jump   = keys&KEY_JUMP;
  key_down   = keys&KEY_DOWN;
  key_left   = keys&KEY_LEFT;
  key_right  = keys&KEY_RIGHT;
  key_crouch = keys&KEY_CROUCH;
  key_prim   = keys&KEY_PRIM;
  key_sec    = keys&KEY_SEC;
  
  if ( dash > 1 ) {
    // set keys
    key_dash = 1;
    dash_angle = R_Angle_From_Mask( 
      (dash&KEY_MISC_ANGLE_DOWN) +
      (dash&KEY_MISC_ANGLE_ZERO) + (dash&KEY_MISC_ANGLE_UP));
    dash_dir = R_Dir_From_Mask((dash&KEY_MISC_DIR_LEFT) +
                               (dash&KEY_MISC_DIR_RIGHT));
    // check validity
    if ( dash_angle == Angle::size || dash_dir == Direction::size )
      key_dash = 0;
  } else
    key_dash = 0;

  if ( wjmp > 1 ) {
    // set keys
    wall_jumping = 1;
    wjmp_angle = R_Angle_From_Mask((wjmp&KEY_MISC_ANGLE_DOWN) +
      (wjmp&KEY_MISC_ANGLE_ZERO) + (wjmp&KEY_MISC_ANGLE_UP));
    wjmp_dir = R_Dir_From_Mask((wjmp&KEY_MISC_DIR_LEFT) +
                               (wjmp&KEY_MISC_DIR_RIGHT));
    // check validity
    if ( wjmp_angle == Angle::size || wjmp_dir == Direction::size )
      wall_jumping = 0;
  } else
    wall_jumping = 0;
}
