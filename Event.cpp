#include "Event.h"

PulEvent::Event::Event(uint16_t ID_, uint16_t bitmask_, 
                       const std::vector<uint8_t>& data_) {
  ID = ID_;
  bitmask = bitmask_;
  data = data_;
}

PulEvent::Event::Event(EventType et_, uint16_t bitmask_,
                       const std::vector<uint8_t>& data_) {
  ID = static_cast<int>(et_);
  bitmask = bitmask_;
  data = data_;
}

void PulEvent::Handle_Event(const PulEvent::Event& p_event) {
  switch ( (EventType)p_event.ID ) {
    case EventType::player_death:
      uint8_t pl_id = p_event.bitmask&0xFF;
    break;
  }
}