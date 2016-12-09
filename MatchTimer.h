#ifndef MATCHTIMER_H_
#define MATCHTIMER_H_
#pragma once

#include "Object.h"
#include "NetObj.h"
#include "Utility.h"

// Match timer uses X position as hours, Y position as seconds and rotation
// as milliseconds. This is to fit in nicely with net obj and for the most
// part you shouldn't have to worry about this
class MatchTimer : public AOD::Object {
  Util::Time timer;
public:
  MatchTimer();
  ~MatchTimer();

  int R_Milliseconds() const;
  int R_Seconds() const;
  int R_Hours() const;
  const Util::Time& R_Timer() const;

  void Update();
};

#endif
