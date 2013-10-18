CFLAGS+= -Wall
LDADD+= -lX11 -lpthread
LDFLAGS=
EXEC=power2_mon

PREFIX?= /usr/local
BINDIR?= $(PREFIX)/bin

CC=gcc

all: $(EXEC)

power2_mon: power2_mon.o
	$(CC) $(LDFLAGS) -O2 -ffast-math -fno-unit-at-a-time -o $@ $+ $(LDADD)

install: all
	install -Dm 755 power2_mon $(DESTDIR)$(BINDIR)/power2_mon

clean:
	rm -fv power2_mon *.o
