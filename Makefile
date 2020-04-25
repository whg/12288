include Makefile.common

all: pru/segment-block.bin
	$(MAKE) -C src
	mv src/$(EXEC) build


pru/segment-block.bin: pru/segment-block.p
	cpp -Iinclude $^ | perl -p -e 's/^#.*//; s/;/\n/g;' >$^_
	pasm -V3 -b $^_
	# rm $^_
	mv segment-block.bin build
