#ifndef PULNET_HANDLER_H_
#define PULNET_HANDLER_H_
#pragma once
#include <vector>
#include <string>
#include "Network.h"

namespace PulNet {
  void Handle_Packet(ENetPacket* pack, ENetPeer* peer);
  void Handle_Connect(ENetPeer* peer,
                      ENetPeer* client_peer[MAX_ALLOWED_USERS]);
  void Handle_Disconnect(ENetPeer* peer);
  void Send_All_NetObjs_To_Client(uint8_t uid);
};

#endif
