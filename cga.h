#ifndef __CGA_H
#define __CGA_H

#include <inttypes.h>

static uint64_t CGA_COLORS[] = {
  0x000000000000ffffull,
  0x00000000aaaaffffull,
  0x0000aaaa0000ffffull,
  0x0000aaaaaaaaffffull,

  0xaaaa00000000ffffull,
  0xaaaa0000aaaaffffull,
  0xaaaa55550000ffffull,
  0xaaaaaaaaaaaaffffull,

  0x555555555555ffffull,
  0x55555555ffffffffull,
  0x5555ffff5555ffffull,
  0x5555ffffffffffffull,

  0xffff55555555ffffull,
  0xffff5555ffffffffull,
  0xffffffff5555ffffull,
  0xffffffffffffffffull,
};

#endif
