#ifndef PULCHER_EVENT_H_
#define PULCHER_EVENT_H_
#pragma once

#include <stdint.h>
#include <string>
#include <vector>
namespace PulEvent {
  enum class EventType {
    player_death,
  };

  struct Event {
    uint16_t ID, bitmask;
    std::vector<uint8_t> data;
    Event(uint16_t ID, uint16_t bitmask, const std::vector<uint8_t>& data);
    Event(EventType et, uint16_t bitmask, const std::vector<uint8_t>& data);
  };
  void Handle_Event( const Event& );
}
#endif