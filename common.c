#include "cga.h"
#include "cgafont.h"

#include <inttypes.h>

void Cga16Expand(uint16_t p, uint64_t *part) {
  uint64_t a = CGA_COLORS[(p >> 8) & 0xf];
  uint64_t b = CGA_COLORS[(p >> 12) & 0xf];
  uint8_t row0 = CGA_FONT[(p & 0xff) * 8 + 0];
  uint8_t row1 = CGA_FONT[(p & 0xff) * 8 + 1];
  part[0] = (row0 & 0x80) ? a : b;
  part[1] = (row0 & 0x40) ? a : b;
  part[2] = (row0 & 0x20) ? a : b;
  part[3] = (row0 & 0x10) ? a : b;
  part[4] = (row0 & 0x08) ? a : b;
  part[5] = (row0 & 0x04) ? a : b;
  part[6] = (row0 & 0x02) ? a : b;
  part[7] = (row0 & 0x01) ? a : b;
  part[8] = (row1 & 0x80) ? a : b;
  part[9] = (row1 & 0x40) ? a : b;
  part[10] = (row1 & 0x20) ? a : b;
  part[11] = (row1 & 0x10) ? a : b;
  part[12] = (row1 & 0x08) ? a : b;
  part[13] = (row1 & 0x04) ? a : b;
  part[14] = (row1 & 0x02) ? a : b;
  part[15] = (row1 & 0x01) ? a : b;
}
