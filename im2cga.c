#include "cga.h"

#include <endian.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define WIDTH 640
#define HEIGHT 400

extern void Cga16Expand(uint16_t p, uint64_t *part);

static int DistanceC(uint64_t x, uint64_t y) {
  int xr = (x >> (48 + 8)) & 0xff;
  int yr = (y >> (48 + 8)) & 0xff;
  int xg = (x >> (32 + 8)) & 0xff;
  int yg = (y >> (32 + 8)) & 0xff;
  int xb = (x >> (16 + 8)) & 0xff;
  int yb = (y >> (16 + 8)) & 0xff;
  int dr = (xr - yr);
  int dg = (xg - yg);
  int db = (xb - yb);
  dr *= dr;
  dg *= dg;
  db *= db;
  return dr + dg + db;
}

static uint64_t Average(uint64_t x, uint64_t y) {
  uint64_t rx = (x >> 48) & 0xffff;
  uint64_t gx = (x >> 32) & 0xffff;
  uint64_t bx = (x >> 16) & 0xffff;
  uint64_t ax = x & 0xffff;
  uint64_t ry = (y >> 48) & 0xffff;
  uint64_t gy = (y >> 32) & 0xffff;
  uint64_t by = (y >> 16) & 0xffff;
  uint64_t ay = y & 0xffff;
  uint64_t r = (rx + ry) / 2;
  uint64_t g = (gx + gy) / 2;
  uint64_t b = (bx + by) / 2;
  uint64_t a = (ax + ay) / 2;
  return (r << 48) | (g << 32) | (b << 16) | a;
}

static int Distance(const uint64_t *a, const uint64_t *b) {
  int total = 0;
  for (int i = 0; i < 16; ++i) {
    int d = DistanceC(a[i], b[i]);
    total += d * 3;
  }
  for (int i = 0; i < 7; ++i) {
    int d = DistanceC(Average(a[i], a[i + 1]),
                      Average(b[i], b[i + 1]));
    total += d;
  }
  for (int i = 8; i < 15; ++i) {
    int d = DistanceC(Average(a[i], a[i + 1]),
                      Average(b[i], b[i + 1]));
    total += d;
  }
  return total;
}

static uint16_t Closest(const uint64_t *goal) {
  int best_distance = 0x7fffffff;
  int best = 0;
  for (int i = 0; i < 0x10000; ++i) {
    uint64_t part[16];
    Cga16Expand(i, part);
    int alt_distance = Distance(part, goal);
    if (alt_distance < best_distance) {
      best = i;
      best_distance = alt_distance;
    }
  }
  return best;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "USAGE: %s <src> <outdat>\n", argv[0]);
    return 1;
  }

  FILE *inf = fopen(argv[1], "rb");
  if (!inf) {
    fprintf(stderr, "ERROR: input '%s' not found\n", argv[1]);
    return 1;
  }

  FILE *out = fopen(argv[2], "wb");
  if (!out) {
    fprintf(stderr, "ERROR: can't open output '%s'\n", argv[2]);
    return 1;
  }

  int channel[2];
  pipe(channel);
  int pid = fork();
  if (!pid) {
    dup2(fileno(inf), 0);
    dup2(channel[1], 1);
    execlp("2ff", NULL);
  }
  fclose(inf);
  FILE *in = fdopen(channel[0], "rb");

  // Write BSAVE header
  fputc(0xfd, out);
  fputc(0x00, out); fputc(0xb8, out);
  fputc(0x00, out); fputc(0x00, out);
  fputc(0x80, out); fputc(0x3e, out);

  struct {
    uint64_t magic;
    uint32_t width, height;
  } header;
  fread(&header, sizeof(uint64_t) * 2, 1, in);
  header.width = be32toh(header.width);
  header.height = be32toh(header.height);

  if (header.width < WIDTH || header.height < HEIGHT) {
    fprintf(stderr, "ERROR: Only size at least 640x400 supported\n");
    return 1;
  }

  uint64_t *buffer = malloc(header.width * sizeof(uint64_t) * 4);
  for (int y = 0; y < HEIGHT; y += 4) {
    fprintf(stderr, "%d            \r", y);
    fread(buffer, sizeof(uint64_t), header.width * 4, in);
    for (int x = 0; x < WIDTH; x += 8) {
      // Pull out 8x2 part.
      uint64_t part[16];
      memcpy(part, buffer + x, sizeof(uint64_t) * 8);
      memcpy(part + 8, buffer + header.width * 2 + x, sizeof(uint64_t) * 8);
      for (int i = 0; i < 16; i++) {
        part[i] = be64toh(part[i]);
      }
      // Process.
      uint16_t pick = Closest(part);
      fwrite(&pick, sizeof(uint16_t), 1, out);
    }
  }
  fclose(in);
  fclose(out);
  fprintf(stderr, "Done.            \n");

  return 0;
}
