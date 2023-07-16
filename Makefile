INPUT_FILES=$(wildcard inputs/*.png)
OUTDAT_FILES=$(patsubst inputs/%.dat,outdat/%.dat,$(INPUT_FILES))
OUTPNG_FILES=$(patsubst inputs/%.dat,outpng/%.png,$(INPUT_FILES))

CFLAGS=-O2 -Iout

all: out/cga2im out/im2cga

images: $(OUTPNG_FILES)

out:
	mkdir -p $@

out/cgafont.h: cgafont.txt | out
	echo "unsigned char CGA_FONT[] = {" >$@
	awk '{ print $$1 "," }' < $< >>$@
	echo "};" >>$@

out/%.o: %.c cga.h out/cgafont.h | out
	$(CC) $(CFLAGS) -c $< -o $@

out/cga2im: out/cga2im.o out/common.o

out/im2cga: out/im2cga.o out/common.o

outpng/%.png: outdat/%.dat out/cga2im
	./out/cga2im $< $@

outdat/%.dat: inputs/%.png out/im2cga
	./out/im2cga $< $@

clean:
	rm -rf out/
