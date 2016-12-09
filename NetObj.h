#ifndef NETWORKOBJ_H_
#define NETWORKOBJ_H_
#pragma once

// More or less the same as NetObj from client except it's tuned more to fit
// with how the server should behave

#include "Event.h"
#include "NetObject_Types.h"
#include "Object.h"
#include <bitset>
#include <functional>
#include <map>

namespace AODP {
  class NetObj {
  public:
    enum FLAGS {
      SYNC_POS,
      SYNC_ROT,
      SYNC_ANIM,
      SIZE
    };
  protected:
    uint8_t net_flags;
    uint8_t obj_flags; // mostly for communication with client/server
    /// ID of object on client/server side (ID on AOD::Object does not sync)
    uint16_t netID;
    /// Type of object, it does nothing for AOD Engine but it is necessary so
    /// so that when you create an object on the client or server, we know
    /// what type of object was made and the game itself must handle the
    /// rest
    uint16_t obj_type;
    AOD::Object* obj;
  public:
    NetObj(uint8_t netflags, uint16_t type, AOD::Object*, uint8_t flags = 0);
    NetObj();
    ~NetObj();
    inline void Set_NetID(uint16_t n)  { netID = n;        }
    void Set_AODObj(AOD::Object* n);
    inline uint16_t R_NetID()    const { return netID;     }
    inline uint16_t R_Obj_Type() const { return obj_type;  }
    std::string R_Obj_Type_Str() const;
    inline AOD::Object* R_AODObj()     { return obj;       }
    inline uint8_t R_Obj_Flag()  const { return obj_flags; }

    virtual uint16_t R_Anim();
  };
  namespace Net {
    /// Remove net obj from local and all clients. This WILL call
    /// AOD::Remove
    void Rem_NetObj(int netID);
    void Rem_NetObj(AODP::NetObj*);
    /// Add net obj to local and all clients, in most cases you should pass in
    /// a value of 0 for netID as the function will utilize the netID_counter
    /// if netID is 0. Be sure to add it to AOD realm too
    void Add_NetObj(AODP::NetObj*, uint16_t netID = 0);
    /// Adds object to network and engine only if it's networkeable
    void Add_NetAodObj(AOD::Object* obj, uint8_t obj_type);
    void Add_NetAodObj(AOD::Object* obj, NetObj_Type obj_type);
    /// Updates all net objects, not in terms of an object but in terms of
    /// a net obj, which means that it will update the net obj on the client
    /// Will also handle deallocation (and communication of that to client)
    /// for net objects that are waiting to be removed. Most likely you
    /// should call this every server tick, which could be like every 50
    /// MS or something.
    enum UPDATE_FLAGS {
      FORCE_UPDATE = 1,
      NO_NETWORK   = 2
    };
    void Update(uint32_t update_flags = 0);
    /* Sends all current net objects to user (probably only should be used
       when a player connects to the server */
    void Send_NetObjs_To_Client(uint16_t uid);
    /// Sends event to client
    void Fire_Event ( const PulEvent::Event& event );
    /// returns current ID counter
    uint32_t R_netObj_Counter();
    /// returns object flag
    /// returns size of net objs array (not the amount of net objs)
    int R_netObj_Size();
    /// returns actual amount of net objects
    int R_True_Amt_Netobjs();
    /// calls function on each net obj
    void Call_Func_On_Every_Netobj(std::function<void(const AODP::NetObj*)>);
  }
};

#endif
