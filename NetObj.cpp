/* (c) CiggBit. All rights reserved.
 * See license.txt for more information
 */
#include "Game_Manager.h"
#include "Server_Vars.h"
#include "Realm.h"
#include "NetObj.h"
#include "NetObject_Types.h"
#include "Network.h"
#include "Player.h"
#include "Server_Vars.h"
#include "Utility.h"

namespace AODNet = AODP::Net;
namespace AODP {
  namespace Net {
    static std::vector<AODP::NetObj*> net_objs, new_net_objs;
    static std::vector<uint16_t> rem_net_objs;
    static uint16_t netID_counter = 0;
  }
}

AODP::NetObj::NetObj(uint8_t nf, uint16_t ot, AOD::Object* o, uint8_t fl) {
  net_flags = nf;
  obj_flags = fl;
  obj_type  = ot;
  obj       = o;
}

AODP::NetObj::NetObj() {
  net_flags = 0;
  obj_flags = 0;
  obj_type  = 0;
  obj       = nullptr;
}

AODP::NetObj::~NetObj() {}

uint16_t AODP::NetObj::R_Anim() { return 0; }
std::string AODP::NetObj::R_Obj_Type_Str() const {
  return NetObj_Types_To_String(static_cast<NetObj_Type>( obj_type ));
}

// returns if netID is within valid range of net objs AND that no instance
// exists if we were to overwrite it anyways (I don't know if the former
// portion we will ever have to worry about being false)
static bool Check_Valid_NetID ( uint16_t netID ) {
  using AODNet::net_objs;
  return net_objs.size() > netID && net_objs[netID] == nullptr;
}

void AODP::Net::Add_NetObj(AODP::NetObj* obj, uint16_t netID) {
  // TODO: Assert that this is already in AOD
  if ( netID == 0 ) {
    // will loop through until we find an empty spot or hit the end of the
    // vector, if we hit the end then we push back a null and can then
    // continue
    while ( !Check_Valid_NetID( netID_counter ) ) {
      if ( netID_counter++ == net_objs.size() ) {
        net_objs.push_back(nullptr);
        break;
      }
    }
    obj->Set_NetID(netID_counter == 0 ? 1 : netID_counter);

    Add_NetObj(obj, obj->R_NetID()); // call self w/ new ID
    return;
  }
  --netID;
  obj->Set_NetID(netID);
  // make sure we are not overwriting another object
  if ( !Check_Valid_NetID( netID ) ) {
    std::cout << "Trying to overwrite NetObj ID: " << std::to_string(netID)
      << " -- Will deallocate object from AOD\n";
    AOD::Remove(net_objs[netID]->R_AODObj());
  }
  net_objs[netID] = obj;
  new_net_objs.push_back(obj);
}

void AODP::Net::Add_NetAodObj(AOD::Object* obj, uint8_t t) {
  auto n = new AODP::NetObj(0, t, obj);
  AOD::Add(obj);
  AODP::Net::Add_NetObj(n);
}
void AODP::Net::Add_NetAodObj(AOD::Object* obj, NetObj_Type t) {
  Add_NetAodObj(obj, static_cast<uint8_t>( t ));
}


void AODP::NetObj::Set_AODObj(AOD::Object* n) {
  obj = n;
}

void AODP::Net::Rem_NetObj(int netID) {
  if ( net_objs.size() <= netID || net_objs[netID] == nullptr ) {
    std::cout << "Trying to remove nonexistent NetObj of ID: " <<
      std::to_string(netID);
  } else {
    auto* obj = net_objs[netID]->R_AODObj();
    if ( obj != nullptr );
    AOD::Remove( obj );
    rem_net_objs.push_back(netID);
  }
}

void AODP::Net::Rem_NetObj(AODP::NetObj* obj) {
  Rem_NetObj(obj->R_NetID());
}

uint32_t AODP::Net::R_netObj_Counter() { return netID_counter;  }
int AODP::Net::R_netObj_Size()         { return net_objs.size(); }

void Broadcast_NetObj_Packet(PacketVec& packet) {
  for ( int i = 0, cnt = 0; cnt != SV::clients_connected; ++ i ) {
    if ( game_manager->players[i] ) {
      ++ cnt;
      if ( game_manager->players[i]->refresh_netobjs )
        PulNet::Send_Packet(packet, i);
    }
  }
}

void AODP::Net::Update(uint32_t update_flags) {
  bool force = update_flags&UPDATE_FLAGS::FORCE_UPDATE;
  bool nonet = update_flags&UPDATE_FLAGS::NO_NETWORK;
  static int update_count;
  if ( SV::cl_update_rate_count < SV::cl_update_rate && !force )
    return;
  // fine to update re6reshes, news, deletes, etc
  
  // ----- new ------
  if ( new_net_objs.size() != 0 ) {
    // Pack packet with netIDS to send to client for creation on client
    std::vector<uint8_t> packet =
      Util::Pack_Num((uint8_t)PulNet::PacketEvent::netObj_create);
    for ( auto& obj : new_net_objs ) {
      const auto* aobj = obj->R_AODObj();
      if ( aobj == nullptr ) {
        std::cout << "Aobj is null @ AODP::Net::Update for object ID "
                  << obj->R_NetID() << " typ " << obj->R_Obj_Type_Str() << '\n';
      }
      Util::Prepend_Pack_Num<uint16_t>(packet,  obj->R_NetID()     );
      Util::Prepend_Pack_Num<uint16_t>(packet,  obj->R_Obj_Type()  );
      Util::Prepend_Pack_Num<int16_t> (packet, aobj->R_Position().x);
      Util::Prepend_Pack_Num<int16_t> (packet, aobj->R_Position().y);
      Util::Prepend_Pack_Num<float>   (packet, aobj->R_Rotation()  );
      Util::Prepend_Pack_Num<float>   (packet,  obj->R_Anim()      );
      Util::Prepend_Pack_Num<uint16_t>(packet,  obj->R_Obj_Flag()  );
    }
    // broadcast NC packet
    if ( !nonet ) {
      Broadcast_NetObj_Packet(packet);
    }
  }
  new_net_objs.clear();

  // ----- remove -----
  if ( rem_net_objs.size() != 0 ) {
    // remove NetObjs from vec while packing packet with netIDs to send to
    // client for removal. Note that we never dealloc an obj, in fact, they're
    // already dealloc'd thanks to realm
    std::vector<uint8_t> packet =
      Util::Pack_Num((uint8_t)PulNet::PacketEvent::netObj_remove);
    for ( auto& id : rem_net_objs ) {
      net_objs[id] = nullptr;
      if ( id < netID_counter ) netID_counter = id;
      Util::Prepend_Pack_Num<uint16_t>(packet, id);
    }
    // then check if we can clear room in the vector
    while ( net_objs.size() > 0 && net_objs[net_objs.size()-1] == nullptr )
      net_objs.pop_back();
    // broadcast ND packet
    if ( !nonet ) Broadcast_NetObj_Packet(packet);
  }
  rem_net_objs.clear();

  // ----- refresh ------
  std::vector<uint8_t> packet =
    Util::Pack_Num((uint8_t)PulNet::PacketEvent::netObj_refresh);
  for ( auto& obj : net_objs ) {
    if ( obj == nullptr ) continue;
    const auto* aobj = obj->R_AODObj();
    if ( aobj == nullptr ) continue;
    Util::Prepend_Pack_Num<uint16_t>(packet, obj->R_NetID());
    Util::Prepend_Pack_Num<int16_t>(packet, aobj->R_Position().x);
    Util::Prepend_Pack_Num<int16_t>(packet, aobj->R_Position().y);
    Util::Prepend_Pack_Num<float>(packet, aobj->R_Rotation());
    Util::Prepend_Pack_Num<uint8_t>(packet, obj->R_Anim());
  }
  // broadcast NR packet
  if ( !nonet ) {
    ++ game_manager->server_timer;
    std::cout << std::to_string ( game_manager->server_timer ) << '\n';
    packet.push_back(game_manager->server_timer);
    Broadcast_NetObj_Packet(packet);
  }
}

void AODP::Net::Send_NetObjs_To_Client(uint16_t uid) {
  std::vector<uint8_t> packet =
      { (uint8_t)PulNet::PacketEvent::netObj_create };
  for ( auto& obj : net_objs ) {
    const auto* aobj = obj->R_AODObj();
    Util::Prepend_Pack_Num<uint16_t>(packet, obj->R_NetID());
    Util::Prepend_Pack_Num<uint16_t>(packet, obj->R_Obj_Type());
    Util::Prepend_Pack_Num<int16_t> (packet, aobj->R_Position().x);
    Util::Prepend_Pack_Num<int16_t> (packet, aobj->R_Position().y);
    Util::Prepend_Pack_Num<float>   (packet, aobj->R_Rotation());
    Util::Prepend_Pack_Num<uint16_t>(packet, obj->R_Anim());
    Util::Prepend_Pack_Num<uint8_t> (packet, obj->R_Obj_Flag());
  }
  std::cout << "Sending " << net_objs.size() << " net objs to client " << uid
            << '\n';
  PulNet::Send_Packet(packet, uid);
}

int AODP::Net::R_True_Amt_Netobjs() {
  int cnt = 0;
  for ( auto& obj : net_objs ) {
    if ( obj != nullptr ) ++ cnt;
  }
  return cnt;
}

void AODP::Net::Call_Func_On_Every_Netobj(
                  std::function<void(const AODP::NetObj*)> fn) {
  for ( auto& obj : net_objs )
    if ( obj != nullptr )
      fn(obj);
}
