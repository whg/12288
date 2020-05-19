include Makefile.common

all: build/segment-block.bin
	$(MAKE) -C src
	mv src/$(EXEC) build


build/segment-block.bin: pru/segment-block.p
	cpp -Iinclude $^ | perl -p -e 's/^#.*//; s/;/\n/g;' >$^_
	pasm -V3 -b $^_
	rm $^_
	mv segment-block.bin $@

clean:
	rm build/segment-block.bin
	rm src/*.o
