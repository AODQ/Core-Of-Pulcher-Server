/*(c) CiggBit. All rights reserved.netID
 * See license.txt for more information
 */
 
#include "PulNet_Handler.h"
#include "Network.h"
#include "Utility.h"

#include <atomic>
#include <enet/enet.h>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <utility>
#include <string>
#include <sstream>
#include <thread>


ENetAddress PulNet::server_address;
namespace PulNet {
  static ENetHost* server_host;
  ENetPeer* client_peer[MAX_ALLOWED_USERS] = { nullptr };
}
static uint8_t server_timestamp;

void PulNet::Send_Packet(const void* data, int len, uint8_t sizeof_data,
                         ENetPeer* peer, _ENetPacketFlag flag) {
  ENetPacket* packet = enet_packet_create(data, len*sizeof_data, flag);
  enet_peer_send(peer, 0, packet);
  enet_host_flush(PulNet::server_host);
}
void PulNet::Send_Packet(const char* data, int len, ENetPeer* peer,
                                             _ENetPacketFlag flag) {
  PulNet::Send_Packet((void*)data, len, 1, peer, flag);
}

void PulNet::Send_Packet(const char* data, int len, int uid,
                                       _ENetPacketFlag flag) {
  if ( PulNet::client_peer[uid] != nullptr )
    Send_Packet(data, len, PulNet::client_peer[uid], flag);
}

void PulNet::Send_Packet(const std::string& data, int uid,
                                   _ENetPacketFlag flag) {
  if ( PulNet::client_peer[uid] != nullptr )
    Send_Packet(data.c_str() + '\0', data.size()+(size_t)1,
                            PulNet::client_peer[uid], flag);
}

void PulNet::Send_Packet(const std::vector<uint8_t>& data, int uid,
                         _ENetPacketFlag flag) {
  auto peer = PulNet::client_peer[uid];
  if ( !peer ) return;
  Send_Packet((const void*)&data[0], data.size(), 1, peer, flag);
}

void PulNet::Send_Packet_Event(PacketEvent pe, std::vector<uint8_t>& data,
                               int uid) {
  data.insert(data.begin(), (uint8_t)pe);
  Send_Packet(data, uid);
}

void PulNet::Broadcast_Packet(const std::string& data, _ENetPacketFlag flag) {
  for ( int i = 0, cnt = 0; cnt != SV::clients_connected; ++ i ) {
    if ( PulNet::client_peer[i] )
      Send_Packet(data, i, flag), ++ cnt;
  }
}

void PulNet::Broadcast_Packet(const std::vector<uint8_t>& data,
                      std::function<bool(uint8_t)>& skip,
                      _ENetPacketFlag flag) {
  for ( int i = 0, cnt = 0; cnt != SV::clients_connected; ++ i ) {
    if ( PulNet::client_peer[i] ) {
      ++ cnt;
      if ( !skip(i) ) Send_Packet(data, i, flag);
    }
  }
}

void PulNet::Broadcast_Packet(const std::vector<uint8_t>& data,
                              _ENetPacketFlag flag) {
  for ( int i = 0, cnt = 0; cnt != SV::clients_connected; ++ i ) {
    if ( PulNet::client_peer[i] )
      Send_Packet(data, i, flag), ++ cnt;
  }
}

void PulNet::Broadcast_Packet_Event(PacketEvent pe,
        std::vector<uint8_t>& data, _ENetPacketFlag flag) {
  data.insert(data.begin(), (uint8_t)pe);
  Broadcast_Packet(data, flag);
}

void PulNet::Broadcast_Packet_Event(PacketEvent pe, std::vector<uint8_t>& data,
          std::function<bool(uint8_t)>& skip, _ENetPacketFlag flag) {
  data.insert(data.begin(), (uint8_t)pe);
  const std::vector<uint8_t>& rd = data;
  Broadcast_Packet(rd, skip, flag);
}

// Sends entire map file to the socket (Along with telling client to expect it)
/*static bool Send_Map(TCPsocket socket) {
  // get map file size
  std::ifstream fil(map_name, std::ios::binary);
  if ( !fil.good() ) { // critical error, map no longer readable
    std::cout << "Map no longer readable, recommended to restart server\n";
    return 0;
  }
  unsigned int fil_size = R_File_Size(map_name.c_str());
  // Tell them to get ready to download map
  std::string send = "DMAP \"" + map_name + "\" " + std::to_string(fil_size);
  Send_TCP_Packet(socket, std::string(send));
  // now send the data
  long unsigned int file_it = 0;
  std::cout << "Sending " + send << '\n';
  while ( file_it < fil_size ) {
    char t[1024] = {'\0'};
    fil.read(t, MAX_PACKET_LEN-1);
    t[fil.gcount()] = '\0';
    file_it += fil.gcount();
    std::cout << "Bytes left: " << fil_size - file_it << '\n';
    if ( !Send_TCP_Packet(socket, std::string(t)) ) return 0;
  }
    
  // should be good now
  return 1;
}*/

// ---- declared functions
  
void PulNet::Init() {
  assert(enet_initialize() == 0, "Couldn't initialize ENet");
  do
    server_address.host = ENET_HOST_ANY;
  while ( server_address.host != ENET_HOST_ANY );
  server_address.port = 9113;
    
  server_host = enet_host_create(&server_address, MAX_ALLOWED_USERS, 2, 0, 0);
  assert(server_host != nullptr, "Creating server");
  std::cout << "Server IP: " << Util::R_IP(server_host->address.host)
            << ": "          << server_host->address.port << '\n';
}

void PulNet::Handle_Client_Input() {
  static ENetEvent netevent;
  while ( enet_host_service(server_host, &netevent, 0) > 0 ) {
    switch ( netevent.type ) {
      case ENET_EVENT_TYPE_CONNECT:
        Handle_Connect(netevent.peer, client_peer);
      break;
      case ENET_EVENT_TYPE_RECEIVE:
        if ( netevent.peer->data != 0 ) {
          Handle_Packet(netevent.packet, netevent.peer);
          enet_packet_destroy( netevent.packet );
        }
      break;
      case ENET_EVENT_TYPE_DISCONNECT:
        std::cout << *(int*)netevent.peer->data << " disconnected\n";
        client_peer[netevent.data] = nullptr;
        Handle_Disconnect(netevent.peer);
      break;
    }
  }
}

#include "NetObj.h"
#include "NetObject_Types.h"

void PulNet::Handle_Network() {
  AODP::Net::Update();
}

/* NetObj(uint8_t netflags, uint16_t type, AOD::Object*, uint8_t flags = 0); */
