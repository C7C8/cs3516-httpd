#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "cmdline.h"
using std::cout;
using std::cerr;
using std::endl;

sockaddr_in resolveHost(char* hostname);

int main(int argc, char* argv[]){
	gengetopt_args_info args;
	cmdline_parser(argc, argv, &args);

	//gengetopt doesn't generate anything to handle bare arguments like the URL or the port, other than a nice
	//array containing them. We need a URL first and a number second, so verify that we have them both.


	if (args.inputs_num != 2){
		cerr << "Incorrect number of arguments specified, please provide a URL and a port." << endl;
		abort();
	}
	char* host = strtok(args.inputs[0], "/");
	char* path = strtok(nullptr, "/"); //stateful library functions give me the creeps
	int port = atoi(args.inputs[1]);

	if (args.verbose_given){
		cout << "Getting file " << path << " from host " << host << ":" << port << endl;
	}

	int netsocket;
	if ((netsocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Failed to open socket");
		abort();
	}

	sockaddr_in serverAddr = resolveHost(args.inputs[0]);
	if(args.verbose_given){
		cout << "Resolved " << args.inputs[0] << " to " << inet_ntoa(serverAddr.sin_addr) << endl;
	}
}

sockaddr_in resolveHost(char* hostname){
	//Resolve URL using magic
	addrinfo hints;
	addrinfo* result;
	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(hostname, NULL, &hints, &result) != 0){
		perror("Failed to resolve host");
		abort();
	}

	sockaddr_in addr;
	if (result != nullptr)
		addr = *(sockaddr_in*)result->ai_addr;
	freeaddrinfo(result);
	return addr;
}
