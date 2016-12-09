#define SERVER

#ifdef CLIENT
  #define TO_MS(X) AOD::To_MS(X)
  #define MS AOD::R_MS()
#endif

#ifdef CLIENT
#include "Client_Vars.h"
#include "Map.h"
#endif
#include "Game_Manager.h"
#include "Player.h"
#include "Utility.h"

using Pl_CInfo = std::pair<Tile_Base*, AOD::Collision_Info>;
using Pl_CInfo_Vec = std::vector<Pl_CInfo>;

std::vector<AOD::Collision_Info> Player::Tile_Collision(AOD::Vector vel) {
  auto& t_vec = game_manager->curr_map->R_Tile_Vec(
    (int)(position.x - size.x/2 - vel.x/2),
    (int)(position.y - size.y/2 - vel.y/2),
    size.x + vel.x, size.y + vel.y);
  std::vector<AOD::Collision_Info> col_inf;
  // check tiles
  AOD::Collision_Info temp;
  for ( auto vec_it: t_vec )
    if ( vec_it->collideable ) {
      temp = Collide(vec_it, vel);
      if ( temp.collision || temp.will_collide )
        col_inf.push_back( temp );
    }

  return col_inf;
}

 Pl_CInfo_Vec Player::R_Coll_Tiles(AOD::Vector vel) {
  auto& t_vec = game_manager->curr_map->R_Tile_Vec(
    (int)(position.x - size.x/2), (int)(position.y - size.y/2),
                                               size.x, size.y);
  // refresh vertices
  transformed = 1;
  Add_Position(0, 0);
  R_Transformed_Vertices();
  
  Pl_CInfo_Vec civec;

  for ( auto t : t_vec ) {
    if ( t->collideable ) {
      auto ci = Collide(t, vel);
      if ( ci.will_collide || ci.collision )
        civec.push_back (Pl_CInfo( t, ci ));
    }
  }

  return civec;
}

void Player::Update_Velocity() {
  // do collision for non-AABB tiles
  auto& t_vec = game_manager->curr_map->R_Tile_Vec(
    (int)(position.x - size.x/2), (int)(position.y - size.y/2),
                                            size.x, size.y);
  // refresh vertices
  transformed = 1;
  Add_Position(0, 0);
  if ( crouching )
    crouching = 1;
  R_Transformed_Vertices();
  // get additional information
  if ( velocity.y < 0 )
    in_air = true;
  // check tiles
  AOD::Collision_Info ci;
  //std::cout << t_vec.size() << '\n';
  for ( auto vec_it : t_vec )
    if ( vec_it->collideable ) {
      ci = Collide(vec_it, velocity);
      if ( ci.will_collide ) {
        AOD::Vector old_vel = velocity; // for additional info
        Add_Position(ci.translation);
        ci.projection.Normalize();
        auto t = velocity;
        t.Normalize();
        t = AOD::Vector::Reflect(t, ci.projection);
        std::cout << ci.projection << ", " << t << '\n';
        std::cout << ci.translation << '\n';
        velocity += ci.translation;

        // adjust additional information
        if ( ci.translation.y <= 0 ) { // no longer in air
          in_air = 0;
          air_dash_up[0]   = air_dash_up[1]   =   
          air_dash_down[0] = air_dash_down[1] = 0;
        }

        if ( abs(ci.translation.x) > 0.2f && in_air ) {
          timer_keep_wj_vel = 500.f;
          if ( wjmp_old_vel.Magnitude() < old_vel.Magnitude() ) {
            wjmp_old_vel = old_vel;
            std::cout << "WJMP OVEL " << wjmp_old_vel << '\n';
          }
        }
      }
    }
  if ( !in_air && velocity.y > 1.5f ) {
    in_air = true;
    timer_fall_jump = 200.f;
  }
  Add_Position(velocity);

}
void Player::Update_Pre_Var_Set() {
  // jumping
  if ( !key_jump )
    jumping_free = 1;
  jumping = jumping_free && key_jump;
  // dashing
  if ( !key_dash )
    dashing_free = 1;
  dashing = dashing_free && key_dash;
}
void Player::Update_Crouch() {
  if ( key_crouch != crouching ) {
    crouching = key_crouch;
    if ( !crouching ) {
      if ( Tile_Collision({0,-14.0f}).size() == 0 ) {
        Set_Size(21, 49);
        PolyObj::Set_Vertices({{-21/2, -49/2}, {-21/2,  49/2},
                               { 21/2,  49/2}, { 21/2, -49/2}});
        Add_Position(0, -8.5f);
      } else
        crouching = 1;
    } else {
      Set_Size(21, 32);
      PolyObj::Set_Vertices({{-21/2, -31/2}, {-21/2,  31/2},
                             { 21/2,  31/2}, { 21/2, -31/2}});
      Build_Transform();
      Add_Position(0, 7.0f);
    }
  }

  if ( crouching ) {
    dashing = 0;
    wall_jumping = 0;
  }
}
void Player::Update_Horiz_Key_Input() {
  // -- key input
  if ( key_left ^ key_right ) { // movement
    float neg = (key_left?-1:1); // get dir
    if ( !in_air ) { // not in air
      velocity.x += TO_MS(Player_Consts::accel_ground) * neg;
      if ( jumping_free && !key_jump ) // if jumping don't apply friction
        if ( abs(velocity.x) > Player_Consts::max_ground_speed ) // check lim
          velocity.x *= Player_Consts::ground_friction;
    } else { // in air
      if ( !key_jump ) // no velocity added when holding jump
        velocity.x += TO_MS(Player_Consts::accel_air) * neg;
    }
  } else { // no movement, apply friction
    velocity.x *= (in_air? Player_Consts::air_friction :
                           Player_Consts::ground_friction);
  }

  // more friction
  if ( ((key_left  && velocity.x > 0) ||
        (key_right && velocity.x < 0)) && !in_air ) {
    velocity.x *= Player_Consts::ground_friction/2.f;
  }
}
void Player::Update_Dash() {
  // -- dash
  for ( int i = 0; i != (int)Direction::size; ++ i )
  for ( int o = 0; o != (int)Angle::size;     ++ o ) {
    if ( timer_dash[i][o] >= 0 )
      timer_dash[i][o] -= MS;
  }

  if ( dashing && dashing_free ) {
    dashing_free = 0;
    // --- validate possible
    // timer (X second cooldown per dash)
    bool can_dash = timer_dash[(int)dash_dir][(int)dash_angle] < 0;
    // air dash (only one dash up/down on left/right
    if ( in_air && dash_angle == Angle::Up )
      can_dash &= !air_dash_up[(int)dash_dir];
    if ( in_air && dash_angle == Angle::Down )
      can_dash &= !air_dash_down[(int)dash_dir];
    // --- execute dash
    if ( can_dash ) {
      // update valid vars
      timer_dash[(int)dash_dir][(int)dash_angle] = Player_Consts::dash_timer;
      if ( in_air && dash_angle == Angle::Up )
        air_dash_up[(int)dash_dir] = 1;
      if ( in_air && dash_angle == Angle::Down )
        air_dash_down[(int)dash_dir] = 1;
      can_dash = 0;

      // apply force
      int dir = 1;
      if ( dash_dir == Direction::Left )
        dir *= -1;
      float t_vel = velocity.Magnitude() + Player_Consts::dash_force;
      switch ( dash_angle ) {
        case Angle::Up:
          velocity.x = std::cos(Util::To_Rad(85.f)) * t_vel * dir;
          velocity.y = -abs(std::sin(Util::To_Rad(85.f)) * t_vel);
          if ( velocity.x > 5 ) velocity.x = 5;
        break;
        case Angle::Down:
          velocity.x = std::cos(Util::To_Rad(-85.f)) * t_vel * dir;
          velocity.y = abs(std::sin(Util::To_Rad(-85.f)) * t_vel);
          if ( velocity.x < -5 ) velocity.x = -5;
        break;
        case Angle::Zero:
          velocity.x = std::cos(Util::To_Rad(0)) * t_vel * dir;
          velocity.y = 0;
          if ( !in_air )
            velocity.y = -1.0f;
        break;
      }
    }
  }
}
void Player::Update_Friction() {
  // empty (for now?)
}
void Player::Update_Jump() {
  // ---- jumping
  if ( timer_fall_jump > 0 )
    timer_fall_jump -= MS;
  if ( jumping )
    std::cout << jumping << ", " << in_air << ", " << timer_fall_jump << '\n';
  if ( jumping && (!in_air || timer_fall_jump >= 0.f)) {
    jumping_free = 0;
    float t = Player_Consts::jump_force + abs(velocity.Magnitude()*0.99f);
    if ( key_left ^ key_right ) {
      velocity.x =  std::cos(Util::To_Rad(15)) * (t) * (key_left?-1:1);
      velocity.y = -std::sin(Util::To_Rad(15)) * (t);
      if ( velocity.y < -10 ) velocity.y = -5;
      Add_Position(0, -2);
      #ifdef CLIENT // ---- sprite ----
        anim_legs = (jumping_leg == Direction::Right) ?
                      PSheet::Anim_Legs::jump_strafe2 :
                      PSheet::Anim_Legs::jump_strafe1;
        anim_hands = (jumping_leg == Direction::Right) ?
                      PSheet::Anim_Hands::strafe1 :
                      PSheet::Anim_Hands::strafe2;
        jumping_leg = (Direction)(int(jumping_leg)^1); // flip leg
        timer_inair_anim = .5f;
      #endif
      in_air = 1;
      
    } else {
      std::cout << abs(velocity.y) << ", " << velocity.x << ": ";
      float angle = std::atan2f(abs(velocity.y), velocity.x/20);
      std::cout << angle << '\n';
      float Max = Util::To_Rad(90+60),
            Min = Util::To_Rad(60);
      if ( angle < Min ) angle = Min;
      if ( angle > Max ) angle = Max;
      velocity.x = std::cos(angle) * t;
      velocity.y = -abs(std::sin(angle) * t);
    }
    timer_fall_jump = -3.0f;
  }
}
void Player::Update_Wall_Jump() {
  // ---- wall jump
  
  if ( timer_keep_wj_vel >= 0.0f ) {
    timer_keep_wj_vel -= MS;
    if ( timer_keep_wj_vel <= 0.0f ) {
      wjmp_old_vel = {0,0};
    }
  }
  if ( timer_wj_last_dir >= 0.0f ) {
    timer_wj_last_dir -= MS;
  }

  if ( jumping && wall_jumping ) {
    jumping_free = 0;
    // check validity
    bool valid = 0;
    // last time since jumped that direction
    if ( wjmp_last_dir != wjmp_dir || timer_wj_last_dir < 0.f )
      valid = 1;
    // not in air
    if ( !in_air )
      valid = 0;
    // if i am colliding with anything right now don't wj
    if ( Tile_Collision({0,0}).size() > 0 )
      valid = 0;

    // actual wall to collide with
    if ( valid ) {
      valid = false;
      float wjdir_sign = (wjmp_dir == Direction::Right?1.f:-1.f);
      float wj_off_x = 10.0f;

      auto t = R_Coll_Tiles( {wj_off_x * -wjdir_sign, -3.0f} );
      for ( auto _t : t ) {
        if ( _t.second.collision || !_t.second.will_collide ) continue;
        // check that it's pushing us the right direction
        if ( wjmp_dir == Direction::Right?_t.second.translation.x < 0:
                                          _t.second.translation.x > 0 )
        { continue; }
        // check that the axis is vertical
        auto verts = _t.first->R_Transformed_Vertices(),
           utverts = _t.first->R_Vertices();
        for ( int v = 0; v != verts.size(); ++ v ) {
          // get axis
          auto v1 = utverts[v],
               v2 = utverts[(v+1)%utverts.size()];
          AOD::Vector axis ({ -(v2.y - v1.y), v2.x - v1.x });
          axis.Normalize();
          // in range angle
          std::cout << axis << '\n';
          if ( abs(axis.y) < 0.85f ) continue;
          // in range position
          if ( !(verts[v].x > position.x-size.x/2 + wj_off_x * -wjdir_sign &&
                 verts[v].x < position.x+size.x/2 + wj_off_x * -wjdir_sign) )
          { continue; }
          valid = true;
          break;
        }
        if ( valid ) break;
      }
    }

    if ( valid ) {
      // set timers
      timer_wj_last_dir = 500.f;
      wjmp_last_dir = wjmp_dir;
      // get vel
      AOD::Vector vel = velocity;
      if ( timer_keep_wj_vel >= 0.0f ) { // get old var if timer is in
        if ( vel.Magnitude() < wjmp_old_vel.Magnitude() ) {
          vel = wjmp_old_vel;
          timer_keep_wj_vel = -1.f;
        }
      }

      // get last dash time
      float newest_dash_timer = 0;
      for ( int i = 0; i != (int)Direction::size; ++ i )
      for ( int o = 0; o != (int)Angle::size;     ++ o ) {
        if ( timer_dash[i][o] > newest_dash_timer )
          newest_dash_timer = timer_dash[i][o];
      }
      
      // perform
      float dir = (wjmp_dir == Direction::Left?-1.0f:1.0f);
      float t_vel = vel.Magnitude()*0.9;
      switch ( wjmp_angle ) {
        case Angle::Up:
        {
          float ang = 75.f;
          if ( newest_dash_timer > 1500.f )
            ang = 90.f;
          velocity.x = std::cos(Util::To_Rad(ang)) * t_vel * dir;
          velocity.y = -abs(std::sin(Util::To_Rad(ang)) * t_vel);
        }
        break;
        case Angle::Down:
        {
          float ang = -75.f;
          if ( newest_dash_timer > 1500.f )
            ang = -90.f;
          velocity.x = std::cos(Util::To_Rad(ang)) * t_vel * dir;
          velocity.y = abs(std::sin(Util::To_Rad(ang)) * t_vel);
        }
        break;
        case Angle::Zero:
          velocity.x = -vel.x;
        break;
      }
    }
  }
}
/*// set timers
timer_wj_last_dir = 500.f;
wjmp_last_dir = wjmp_dir;
// get vel
AOD::Vector vel = velocity;
if ( timer_keep_wj_vel >= 0.0f ) { // get old var if timer is in
  if ( vel.Magnitude() < wjmp_old_vel.Magnitude() ) {// do we even want it?
    vel = wjmp_old_vel;
    timer_keep_wj_vel = -1.f;
  }
}

// get last dash time
float newest_dash_timer = 0;
for ( int i = 0; i != (int)Direction::size; ++ i )
for ( int o = 0; o != (int)Angle::size;     ++ o ) {
  if ( timer_dash[i][o] > newest_dash_timer )
    newest_dash_timer = timer_dash[i][o];
}
      
// perform
float dir = (wjmp_dir == Direction::Left?-1.0f:1.0f);
float t_vel = vel.Magnitude();
switch ( wjmp_angle ) {
  case Angle::Up:
  {
    float ang = 75.f;
    if ( newest_dash_timer > 1500.f )
      ang = 90.f;
    velocity.x = std::cos(Util::To_Rad(ang)) * t_vel * dir;
    velocity.y = -abs(std::sin(Util::To_Rad(ang)) * t_vel);
  }
  break;
  case Angle::Down:
  {
    float ang = -75.f;
    if ( newest_dash_timer > 1500.f )
      ang = -90.f;
    velocity.x = std::cos(Util::To_Rad(ang)) * t_vel * dir;
    velocity.y = abs(std::sin(Util::To_Rad(ang)) * t_vel);
  }
  break;
  case Angle::Zero:
    velocity.x = -vel.x;
  break;*/
void Player::Update_Gravity() {

  // gravity
  if ( velocity.y < Player_Consts::max_air_vel_y ) {
    velocity.y += TO_MS(Player_Consts::gravity);
    if ( velocity.y > Player_Consts::max_air_vel_y )
      velocity.y = Player_Consts::max_air_vel_y;
  }
}