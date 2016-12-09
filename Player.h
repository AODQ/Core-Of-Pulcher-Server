/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#ifndef PLAYER_H_
#define PLAYER_H_
#pragma once
#include <atomic>
#include <map>
#include <string>

#include "Object.h"
#include "NetObj.h"

enum class PlayerStatus {
  dead, alive, spectator, nil, size
};

class PlayerStatusData_Dead {
public:
  float spawn_time;
  PlayerStatusData_Dead();
};

union PlayerStatusData {
  PlayerStatusData_Dead dead_data;
  void* nil;
  PlayerStatusData();
};

enum class PlayerTeam {
  none, blue, red, size
};

class Player;

extern std::atomic<bool> player_consts_being_used;

class Player_Info {
public:
  enum class Team { none, blue, red };
  std::string name, short_name;
  PlayerStatus status;
  PlayerStatusData status_data;
  PlayerTeam team;
  uint8_t character, colour;
  const uint8_t uid;
  bool refresh_netobjs;
  uint8_t client_timestamp;

  Player_Info(unsigned int uid, std::string name = "",
              PlayerStatus = PlayerStatus::nil);

  Player* pl_handle;

  void Update_Data();
};

extern const std::string playerstatus_str[],
                         playerteam_str[];

// returns the status in the form of a 
// string (useful for networking)
std::string R_Status_Str(Player_Info*);

// returns status of player (dead, alive, etc) in form of string
std::string R_Player_Status_Str(Player_Info*);

// return status of team in form of string
std::string R_Team_Status_Str(Player_Info*);


namespace Player_Consts {
  extern float max_ground_speed,
               ground_friction, 
               air_friction,
               accel_ground,
               accel_air,
               max_air_vel_x,
               max_air_vel_y,
               gravity,
               jump_force,
               dash_force,
               dash_timer;

  extern const std::vector<std::pair<float*, std::string>> floats;
};

class Tile_Info;

class Player : public AOD::PolyObj {
public:
  enum class Sprite_Status {
    stand, run, crouch, slide, jump, wall_jump
  };
  enum class Angle {
    Up, Zero, Down, size
  };
  enum class Direction {
    Left, Right, size
  };
private:
  const Player_Info* pl_info;

  int health, armour;

  bool direction;

  bool in_air, wall_jumping, in_liquid;

  bool key_up, key_down, key_left, key_right, key_prim,
       key_sec, key_crouch, key_dash,
       key_jump;

  Angle wjmp_angle, dash_angle;
  Direction wjmp_dir, dash_dir;
  // dash
  bool dashing,
       dashing_free;
  bool air_dash_up  [(int)Direction::size],
       air_dash_down[(int)Direction::size];
  // wjmp
  AOD::Vector wjmp_old_vel;
  Direction wjmp_last_dir;
  // jump
  bool jumping, // key jump is down AND has let go since last jump
       jumping_free; // key jump has been let go of
  // crouch sliding
  float crouch_slide_timer;
  Direction crouch_slide_dir;
  bool crouching;
  // timers
  float timer_fall_jump;
  float timer_dash[(int)Direction::size][(int)Angle::size];

  float timer_keep_wj_vel, timer_wj_last_dir;

  // utility functions
  std::vector<AOD::Collision_Info> Tile_Collision(AOD::Vector velocity);
  std::vector<std::pair<Tile_Base*, AOD::Collision_Info>>
                                          R_Coll_Tiles(AOD::Vector vel);

  // update functions
  void Update_Pre_Var_Set();
  void Update_Crouch();
  void Update_Horiz_Key_Input();
  void Update_Dash();
  void Update_Friction();
  void Update_Jump();
  void Update_Wall_Jump();
  void Update_Gravity();
  void Update_Velocity();
public:
  Player(Player_Info*);
  void Update();

  int R_Health() const;
  int R_Armour() const;
  bool R_In_Liquid() const;
  bool R_In_Air() const;

  void Set_Health(int);
  void Set_In_Liquid(bool);
  void Set_Oxygen(int);

  void Set_Key_Up     ( bool );
  void Set_Key_Down   ( bool );
  void Set_Key_Left   ( bool );
  void Set_Key_Right  ( bool );
  void Set_Key_Sec    ( bool );
  void Set_Key_Prim   ( bool );
  void Set_Key_Crouch ( bool );
  void Set_Key_Dash   ( bool );
  void Set_Key_Jump   ( bool );

  void Set_Keys( char keys, char dash, char wjmp );

  const Player_Info* R_Pl_Info();
  
  bool R_Key_Up()        const ;
  bool R_Key_Down()      const ;
  bool R_Key_Left()      const ;
  bool R_Key_Right()     const ;
  bool R_Key_Sec()       const ;
  bool R_Key_Prim()      const ;
  bool R_Key_Crouch()    const ;
  bool R_Key_Dash()      const ;
  bool R_Key_Jump()      const ;
  Angle R_Dash_Angle()   const ;
  Direction R_Dash_Dir() const ;
  Angle R_WJmp_Angle()   const ;
  Direction R_WJmp_Dir() const ;
};


#endif
