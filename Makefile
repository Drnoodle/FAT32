CC=gcc
CFLAGS=-Wall -g -O0 -D_FILE_OFFSET_BITS=64
LDFLAGS=-lfuse

.PHONY: all
all:vfat

vfat: vfat.o util.o debugfs.o vfat_cluster.o vfat_directory.o vfat_boot.o
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.cc *.h
	$(CC) $(CFLAGS) -c $(INCL) $< -o $@

clean:
	rm -f *.o vfat
