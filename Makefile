CPPFLAGS=-Wall -Wextra -Werror -pthread -std=c++1y -g

all: httpc

clean:
	rm -f *.o cmdline.* httpc httpd
	rm -f cmdline.h cmdline.c

httpc: httpc.cpp cmdline.o
	g++ $(CPPFLAGS) httpc.cpp cmdline.o -o httpc

cmdline.o: cmdline.h cmdline.c
	gcc -g -c cmdline.c

cmdline.h cmdline.c: options.ggo
	gengetopt < options.ggo

