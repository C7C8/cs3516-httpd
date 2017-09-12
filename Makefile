CPPFLAGS=-Wall -Wextra -pthread -std=c++1y

all: client server

client: httpc/httpc.cpp httpc/cmdline.o
	g++ ${CPPFLAGS} -o client httpc/httpc.cpp httpc/cmdline.o

httpc/cmdline.o: httpc/cmdline.h httpc/cmdline.c

httpc/cmdline.h httpc/cmdline.c: httpc/options.ggo
	gengetopt --output-dir httpc < httpc/options.ggo

server: httpd/HTTPResponder.o httpd/HTTPHeader.o httpd/cmdline.o httpd/httpd.cpp
	g++ ${CPPFLAGS} -o server httpd/HTTPResponder.o httpd/HTTPHeader.o httpd/cmdline.o httpd/httpd.cpp

httpd/HTTPHeader.o: httpd/HTTPHeader.h httpd/HTTPHeader.cpp

httpd/HTTPResponder.o: httpd/HTTPResponder.h httpd/HTTPResponder.cpp

httpd/cmdline.o: httpd/cmdline.h httpd/cmdline.c

httpd/cmdline.h httpd/cmdline.c: httpd/options.ggo
	gengetopt --output-dir httpd < httpd/options.ggo

clean:
	rm -f client server httpc/*.o httpd/*.o

