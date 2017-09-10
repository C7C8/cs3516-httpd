#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include "cmdline.h"
#include "HTTPHeader.h"
#include "HTTPResponderThread.h"
using std::cout;
using std::cerr;
using std::endl;
using std::thread;

int main(int argc, char* argv[]){
	gengetopt_args_info args;
	cmdline_parser(argc, argv, &args);

	string hstr = "GET / HTTP/1.1\r\n" \
					"Host: motherfuckingwebsite.com\r\n" \
					"Accept: */*\r\n" \
					"Connection: close\r\n" \
					"\r\n";

	HTTPHeader header(hstr);
	cout << "Got header of type " << header.method() << " requesting file " << header.filename() << endl;
	return 0;

	/*
	//Thread count checker
	if (!args.threads_given){
		args.threads_arg = thread::hardware_concurrency();
		if (args.verbose_given) {
			cout << "Maximum thread count not supplied, setting to autodetected value " << args.threads_arg << endl;
		}
	}

	int netsocket = socket(AF_INET, SOCK_STREAM, 0);
	if (netsocket < 0){
		perror("Failed to open socket");
		exit(1);
	}

	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons((uint16_t)args.port_arg);
	if (bind(netsocket, (sockaddr*)&addr, sizeof(addr)) < 0){
		perror("Failed to bind to port");
		exit(1);
	}

	listen(netsocket, 0);
	while (true){
		//Main server loop. Accept connections as they come up, spin up a new responder thread
		//to address each of them. That part of the program is currently unimplemented.
		socklen_t socklen; //?
		sockaddr_in* clientAddr = new sockaddr_in;
		int connection = accept(netsocket, (sockaddr*)&clientAddr, &socklen);

	}
	*/
}