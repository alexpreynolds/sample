BLDFLAGS                  = -Wall -Wextra -pedantic -std=c99
CFLAGS                    = -D__STDC_CONSTANT_MACROS -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1 -O3
CDFLAGS                   = -D__STDC_CONSTANT_MACROS -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1 -DDEBUG=1 -g -O0 -fno-inline
BINDIR                    = ../bin
PROG                      = reservoir-sample
SOURCE                    = reservoir-sample.c

all: build

build:
	mkdir -p $(BINDIR) && ${CC} ${BLDFLAGS} ${CFLAGS} -o ${BINDIR}/${PROG} ${SOURCE}

debug:
	mkdir -p $(BINDIR) && ${CC} ${BLDFLAGS} ${CDFLAGS} -o ${BINDIR}/${PROG} ${SOURCE}

clean:
	rm -f $(BINDIR)/$(PROG)
	rm -f *~