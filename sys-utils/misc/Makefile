PREFIX   ?= /usr
CC       := $(TOOLCHAIN_PREFIX)gcc
CFLAGS   += -Wall -Wextra -pedantic -O2

BINS := memoryrange readmmap writemmap
OBJS := labelinfo.o memoryrange.o readmmap.o writemmap.o

all: $(BINS)

%.o: %.c labelinfo.h
	$(CC) $(CFLAGS) -c -o $@ $<

memoryrange: memoryrange.o labelinfo.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

readmmap: readmmap.o labelinfo.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

writemmap: writemmap.o labelinfo.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

install: $(BINS)
	install -Dm755 memoryrange $(DESTDIR)$(PREFIX)/bin/memoryrange
	ln -sf memoryrange $(DESTDIR)$(PREFIX)/bin/flashdrive
	ln -sf memoryrange $(DESTDIR)$(PREFIX)/bin/nvram
	install -Dm755 readbe64 $(DESTDIR)$(PREFIX)/bin/
	install -Dm755 writebe64 $(DESTDIR)$(PREFIX)/bin/
	install -Dm755 readmmap $(DESTDIR)$(PREFIX)/bin/
	install -Dm755 writemmap $(DESTDIR)$(PREFIX)/bin/

clean:
	@rm -f $(BINS) $(OBJS)
