
CXXFLAGS=-g -O0 -Wall -Wextra -Weffc++ -pedantic
LDFLAGS=-lpng -lmad -lfftw3 -lm -Wl,-O1,--as-needed


all: spectrogramm

clean:
	rm -f *.o *.d core

spectrogramm: $(patsubst %.c,%.o,$(wildcard *.c))
	gcc $^ $(LDFLAGS) -o $@

%.o: %.c
	gcc -c -MD $(CXXFLAGS) $<

include $(wildcard *.d) 

.PHONY: all clean
