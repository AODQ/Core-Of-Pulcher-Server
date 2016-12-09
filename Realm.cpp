/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
 
#include "Game_Manager.h"
#include <map>

#include <windows.h>

#include "Realm.h"

using AOD_Realm = AOD::Realm;

AOD_Realm* AOD::realm;

void AOD_Realm::Reset() {
  for ( auto& oit : objects )
    delete oit;
  objects.clear();
  objs_to_rem.clear();
}

void AOD_Realm::Update(UPDATE_FLAGS flags) {
  static auto remove_objects = [&]() {
    for ( int rem_it = 0; rem_it != objs_to_rem.size(); ++ rem_it ) {
      for ( int obj_it = 0; obj_it != objects.size(); ++ obj_it ) {
        if ( objects[obj_it] == objs_to_rem[rem_it] ) {
          delete objects[obj_it];
          objects.erase(objects.begin() + obj_it);
          break;
        }
      }
    }  
  };
  // update objects not affiliated with game manager
  if ( flags&UPDATE_FLAGS::UPDATE_REFRESH ) {
    for ( auto& oit : objects )
      oit->Update();
    // update game manager
    game_manager->Update();
  }
  // remove objects
  if ( flags&UPDATE_FLAGS::UPDATE_REMOVE ) {
    remove_objects();
    objs_to_rem.clear();
  }
}

void AOD_Realm::Add(AOD::Object* obj) {
  objects.push_back(obj);
}

void AOD::Realm::Remove(AOD::Object* obj) {
  objs_to_rem.push_back(obj);
}

void AOD::Add   (AOD::Object* obj) { realm->Add(obj);    }
void AOD::Remove(AOD::Object* obj) { realm->Remove(obj); }