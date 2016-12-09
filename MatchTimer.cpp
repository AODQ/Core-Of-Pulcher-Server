#include "MatchTimer.h"
#include "NetObj.h"
#include "NetObject_Types.h"
#include "Server_Vars.h"

MatchTimer::MatchTimer() : AOD::Object() {
  timer.Set_Time();
}

int MatchTimer::R_Milliseconds() const { return timer.R_Milliseconds(); }
int MatchTimer::R_Seconds()      const { return timer.R_Seconds();      }
int MatchTimer::R_Hours()        const { return timer.R_Hours();        }
const Util::Time& MatchTimer::R_Timer() const { return timer; }

void MatchTimer::Update() {
  rotation += SERVER_MS_DT;
  /* static float t = 0.001; */
  /* t += 0.01; */
  /* static bool dir = 1; */
  /* position.x += SERVER_MS_DT/2.0f * t * (dir?1:-1); */
  /* if ( position.x > 600 ) dir = 0; */
  /* if ( position.x < 200 )  dir = 1; */
  timer.Set_Time(static_cast<uint32_t>(rotation));
}
