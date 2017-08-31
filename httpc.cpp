#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <regex>
#include <sys/sendfile.h>
#include <unistd.h>
#include "cmdline.h"
using std::cout;
using std::cerr;
using std::endl;
using std::regex;
using std::regex_match;

#define MAX_BUF_SIZE 8192	//Apache default, might as well use it here
#define HTTP_GET   	"GET /%s HTTP/1.1\r\n" \
					"Host: %s\r\n" \
					"Accept: */*\r\n" \
					"Connection: close\r\n" \
					"\r\n"

sockaddr_in resolveHost(char* hostname);


int main(int argc, char* argv[]){
	gengetopt_args_info args;
	cmdline_parser(argc, argv, &args);

	//gengetopt doesn't generate anything to handle bare arguments like the URL or the port, other than a nice
	//array containing them. We need a URL first and a number second, so verify that we have them both.


	if (args.inputs_num != 2){
		cerr << "Incorrect number of arguments specified, please provide a URL and a port." << endl;
		abort()
				;
	}
	if (strstr(args.inputs[0], "http://") != nullptr) {
		args.inputs[0] += strlen("http://");
		if (args.verbose_given){
			cout << "Eliminating the http:// from your url... why did you put that there anyways?" << endl;
			cout << "New URL: " << args.inputs[0] << endl;
		}
	}
	char* host = strtok(args.inputs[0], "/");
	char* path = strtok(nullptr, "/"); //stateful library functions give me the creeps
	if (path == nullptr){
		path = (char*)malloc(1);
		strcpy(path, "\0");
	}
	uint16_t port = (uint16_t)atoi(args.inputs[1]);

	if (args.verbose_given){
		cout << "Getting file /" << path << " from host " << host << ":" << port << endl;
	}

	int netsocket;
	if ((netsocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Failed to open socket");
		abort();
	}

	sockaddr_in serverAddr = resolveHost(args.inputs[0]);
	serverAddr.sin_port = htons(port);
	if(args.verbose_given){
		cout << "Resolved " << args.inputs[0] << " to " << inet_ntoa(serverAddr.sin_addr) << endl;
		cout << "Connecting..." << endl;
	}

	if (connect(netsocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0){
		perror("Failed to connect to server");
	}
	else if (args.verbose_given){
		cout << "Successfully connected to server\n" << endl;
	}
	printf("Sending request:\n");
	printf(HTTP_GET, path, host);
	dprintf(netsocket, HTTP_GET, path, host);

	char buf[8192] = {'\0'};
	if (read(netsocket, buf, MAX_BUF_SIZE) == -1){
		perror("Failed to read from socket");
		abort();

	}

	/* Since C/C++ REGEXES SUCK AND DON'T SUPPORT LOOKBEHINDS AND ARE NOT WORTHY OF USE,
	 * I've implemented a very simple manual extraction method for the stupid content
	 * length number.
	 *
	 * Curse you forever, regexcomp(). If only the CCC machines had Boost installed...
	 */

	int contentLength = 0;
	char tempBuf[MAX_BUF_SIZE];
	if (strstr(buf, "Content-Length: ") != nullptr) {
		char *lengthStr = strcpy(tempBuf, strstr(buf, "Content-Length: ")); //because strtok eats strings...
		lengthStr += strlen("Content-Length: ");
		contentLength = atoi(strtok(lengthStr, "\r\n"));
	}
	if (args.verbose_given)
		cout << "Content length: " << contentLength << endl;
	int printedBytes = printf(strstr(buf, "\r\n\r\n") + 4);

	while (!contentLength || printedBytes < contentLength){
		memset(buf, 0, MAX_BUF_SIZE);
		ssize_t st = read(netsocket, buf, MAX_BUF_SIZE);
		if (st > 0){
			printf(buf);
			fflush(stdout);
			printedBytes += st;
		}
		else
			break;

		//If we didn't read as much info as could fit into the buffer last time, check if the connection is still open.
		//This is done by writing to the socket and seeing what happens. This feels wrong.
	}

	if (args.verbose_given)
		cout << "\n\nPrinted " << printedBytes << "/" << contentLength << " bytes" << endl;

	close(netsocket);
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
