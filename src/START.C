#include "common.h"

struct setup_data _far *Setup;
struct f15_data   _far *Setup_arming;

char   Called_from_com;


main(int argc, char **argv) {
  unsigned a;

  // Get F15.COM pointers
  if(argc>1 && sscanf(argv[1], "%x", &a)==1 && a) {
    Setup=(void _far *)(long)a;
    Setup_arming=(void _far *)((char _far *)(long)a + sizeof(*Setup));
    Called_from_com=1;
    }
  else {
    Called_from_com=0;
    }
  }
