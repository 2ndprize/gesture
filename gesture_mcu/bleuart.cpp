#include "bleuart.h"

void bleUART(int kind) {
  char buf[2];

  if (kind == 0) {
    strcpy((char*)buf, "l"); // left
    bleuart.write(buf, strlen((char*)buf));
  }
  else if (kind == 1) {
    strcpy((char*)buf, "r"); // right
    bleuart.write(buf, strlen((char*)buf));
  }
  else if (kind == 2) {
    strcpy((char*)buf, "c"); // circle(=ring)
    bleuart.write(buf, strlen((char*)buf));
  }
  else {
    strcpy((char*)buf, "n"); // negative
    bleuart.write(buf, strlen((char*)buf));
  }
}