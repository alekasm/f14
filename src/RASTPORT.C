#include "library.h"
#include "f15defs.h"

  RPS RpX1  =  {0, 0,0, 319,199, JAM3, GREEN, BLACK, 1};
  RPS *Rp1  = &RpX1;

  RPS RpX2  =  {1, 0,0, 319,199, JAM3, GREEN, BLACK, 1};
  RPS *Rp2  = &RpX2;

  RPS RpX3  =  {2, 0,0, 319,199, JAM3, GREEN, BLACK, 1};
  RPS *Rp3  = &RpX3;

  RPS RpHudX = {1, HUDX,HUDY, HUDXSIZ,HUDYSIZ, JAM1, LGREEN, BLACK, 1};
  RPS *RpHud = &RpHudX;

  RPS RpX3D  =  {1,  0,0, 319,SIZE3D, JAM3, GREEN, BLACK, 1};
  RPS *Rp3D  = &RpX3D;
