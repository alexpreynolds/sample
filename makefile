BLDFLAGS                  = -Wall -Wextra -pedantic -std=c99
CFLAGS                    = -D__STDC_CONSTANT_MACROS -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1 -O3
CDFLAGS                   = -D__STDC_CONSTANT_MACROS -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1 -DDEBUG=1 -g -O0 -fno-inline
#INCLUDES                 := -iquote${PWD}/include
INCLUDES                 := -iquoteinclude
OBJDIR                    = objects
SAMPLELIB                := $(PWD)/sample-library.a
TEST                     := $(PWD)/test
PROG                      = sample
SOURCE                    = src/bin/sample.c

all: mt19937 sample-library build

mt19937:
	mkdir -p $(OBJDIR) && $(CC) $(BLDFLAGS) $(CFLAGS) -c src/sample-library/mt19937.c -o $(OBJDIR)/mt19937.o $(INCLUDES)

sample-library: mt19937
	$(AR) rcs $(SAMPLELIB) $(OBJDIR)/mt19937.o

build: sample-library
	$(CC) $(BLDFLAGS) $(CFLAGS) -c $(SOURCE) -o $(OBJDIR)/$(PROG).o $(INCLUDES)
	$(CC) $(BLDFLAGS) $(CFLAGS) $(OBJDIR)/$(PROG).o -o $(PROG) $(SAMPLELIB)

debug: sample-library
	$(CC) $(BLDFLAGS) $(CDFLAGS) -c $(SOURCE) -o $(OBJDIR)/$(PROG).o $(INCLUDES)
	$(CC) $(BLDFLAGS) $(CDFLAGS) $(OBJDIR)/$(PROG).o -o $(PROG) $(SAMPLELIB)

check: build
	$(PWD)/$(PROG) README.md -d 123 | diff - $(TEST)/README.md.seed123.txt > /dev/null || (echo "check: sample test failed on seed 123" && exit 1)
	$(PWD)/$(PROG) README.md -d 234 | diff - $(TEST)/README.md.seed234.txt > /dev/null || (echo "check: sample test failed on seed 234" && exit 1)
	$(PWD)/$(PROG) README.md -d 9876 | diff - $(TEST)/README.md.seed987.txt > /dev/null || (echo "check: sample test failed on seed 987" && exit 1)
	@echo "sample tests passed"

clean:
	rm -f $(PROG)
	rm -rf $(OBJDIR)
	rm -f $(SAMPLELIB)
	rm -rf *~
