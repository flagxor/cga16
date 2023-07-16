#include "cga.h"

#include <endian.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define WIDTH 640
#define HEIGHT 400

extern void Cga16Expand(uint16_t p, uint64_t *part);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "USAGE: %s <srcdat> <outpng>\n", argv[0]);
    return 1;
  }

  FILE *in = fopen(argv[1], "rb");
  if (!in) {
    fprintf(stderr, "ERROR: input '%s' not found\n", argv[1]);
    return 1;
  }

  FILE *outf = fopen(argv[2], "wb");
  if (!in) {
    fprintf(stderr, "ERROR: can't open output '%s'\n", argv[2]);
    return 1;
  }

  int channel[2];
  pipe(channel);
  int pid = fork();
  if (!pid) {
    dup2(channel[0], 0);
    dup2(fileno(outf), 1);
    execlp("ff2png", NULL);
  }
  fclose(outf);
  FILE *out = fdopen(channel[1], "wb");

  // Check header.
  if (fgetc(in) != 0xfd ||
      fgetc(in) != 0x00 || fgetc(in) != 0xb8 ||
      fgetc(in) != 0x00 || fgetc(in) != 0x00 ||
      fgetc(in) != 0x80 || fgetc(in) != 0x3e) {
    fprintf(stderr, "ERROR: Bad BSAVE header\n");
    return 1;
  }

  struct {
    char magic[8];
    uint32_t width, height;
  } header;
  memcpy(header.magic, "farbfeld", 8);
  header.width = be32toh(WIDTH);
  header.height = be32toh(HEIGHT);
  fwrite(&header, sizeof(header), 1, out);

  uint64_t buffer[WIDTH * 4];
  for (int y = 0; y < HEIGHT; y += 4) {
    for (int x = 0; x < WIDTH; x += 8) {
      uint16_t code;
      fread(&code, sizeof(uint16_t), 1, in);
      uint64_t part[16];
      Cga16Expand(code, part);
      // Put in 8x2 slice (stretched vertically).
      for (int i = 0; i < 16; i++) {
        part[i] = htobe64(part[i]);
      }
      memcpy(buffer + WIDTH * 0 + x, part, sizeof(uint64_t) * 8);
      memcpy(buffer + WIDTH * 1 + x, part, sizeof(uint64_t) * 8);
      memcpy(buffer + WIDTH * 2 + x, part + 8, sizeof(uint64_t) * 8);
      memcpy(buffer + WIDTH * 3 + x, part + 8, sizeof(uint64_t) * 8);
    }
    fwrite(buffer, sizeof(uint64_t), WIDTH * 4, out);
  }
  fclose(in);
  pclose(out);

  return 0;
}
