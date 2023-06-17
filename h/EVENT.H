#ifndef EVENT_H
#define EVENT_H

// LOG.F14 Event types
enum {
  E_TICK,
  E_HIT,
  E_DESTROY,
  E_WAYPOINT,
  E_DISENGAGE,
  E_ARMED,
  E_LOCK_UP,
  E_FIRE,
  E_CRASH,
  E_EJECT,
  E_REFUEL,
  E_LAND,
  E_ABORT,
  E_TAKEOFF,
  E_TYPES };

enum {
  EHC_GUN,
  EHC_MISSILE,
  EHC_TYPES };

#endif  // #ifndef EVENT_H
