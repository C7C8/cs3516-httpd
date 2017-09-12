#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "cmdline.h"
using std::cout;
using std::cerr;
using std::endl;

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
	char* host;
	char* path;
	uint16_t port = 80;
	if (args.inputs_num == 0 || args.inputs_num > 2){
		cerr << "Incorrect number of arguments specified, please provide a URL and a port." << endl;
		exit(1);
	}
	else if (args.inputs_num == 1){
		cerr << "No port provided, assuming port 80" << endl;
	}

	if (strstr(args.inputs[0], "http://") != NULL) {
		args.inputs[0] += strlen("http://");
		if (args.verbose_given){
			cout << "Eliminating the http:// from your url... why did you put that there anyways?" << endl;
			cout << "New URL: " << args.inputs[0] << endl;
		}
	}
	host = strtok(args.inputs[0], "/");
	path = strtok(NULL, " "); //stateful library functions give me the creeps
	if (path == NULL){
		path = (char*)malloc(1);
		strcpy(path, "\0");
	}
	if (args.inputs_num == 2)
		port = (uint16_t)atoi(args.inputs[1]);

	if (args.verbose_given)
		cout << "Getting file /" << path << " from host " << host << ":" << port << endl;

	int netsocket = socket(AF_INET, SOCK_STREAM, 0);
	if (netsocket < 0){
		perror("Failed to open socket");
		exit(1);
	}

	sockaddr_in serverAddr = resolveHost(args.inputs[0]);
	serverAddr.sin_port = htons(port);
	if(args.verbose_given){
		cout << "Resolved " << args.inputs[0] << " to " << inet_ntoa(serverAddr.sin_addr) << endl;
		cout << "Connecting..." << endl;
	}

	if (connect(netsocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0){
		perror("Failed to connect to server");
		exit(1);
	}
	else if (args.verbose_given) {
		cout << "Successfully connected to server\n" << endl;
		printf("Sending request: \n");
		printf(HTTP_GET, path, host);
		//These two printfs are magical. I removed them once, and it exposed a mysterious buffer overrun bug. How
		//that is in any way connected to these printfs is beyond me.
	}
	timeval startTime;
	gettimeofday(&startTime, NULL);
	dprintf(netsocket, HTTP_GET, path, host);

	char buf[8192] = {'\0'};
	if (read(netsocket, buf, MAX_BUF_SIZE) == -1){
		perror("Failed to read from socket");
		exit(1);

	}

	//Since the server has now responded (otherwise read() would've blocked), we can call that time the RTT. Print it
	//if need be.
	if (args.print_rtt_given){
		timeval endTime;
		gettimeofday(&endTime, NULL);
		time_t ms = ((endTime.tv_sec - startTime.tv_sec) * 1000) + ((endTime.tv_usec - startTime.tv_usec) / 1000);
		cerr << "RTT: " << ms << "ms" << endl; //Makes it easier to filter out data
	}

	//Extract the content length from the response, if there is one.
	int contentLength = 0;
	char tempBuf[MAX_BUF_SIZE];
	if (strstr(buf, "Content-Length: ") != NULL) {
		char *lengthStr = strncpy(tempBuf, strstr(buf, "Content-Length: "), MAX_BUF_SIZE); //because strtok eats strings...
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
			printedBytes += st;
		}
		else
			break;
		//sssh, read errors? what are those?
	}

	if (args.verbose_given)
		cout << "\n\nPrinted " << printedBytes << "/" << contentLength << " bytes" << endl;

	close(netsocket);
}

sockaddr_in resolveHost(char* hostname){
	//Resolve URL using simple getaddrinfo method
	addrinfo hints;
	addrinfo* result;
	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(hostname, NULL, &hints, &result) != 0){
		perror("Failed to resolve host, either your URL is wrong, or");
		exit(1);
	}

	//getaddrinfo actually returns a linked list (each addrinfo struct has a pointer to another addrinfo struct),
	//but we don't actually care about those other results here. Just extract a sockaddr_in struct from the first addr
	//in the chain, then free the memory occupied by it.
	sockaddr_in addr = *(sockaddr_in*)result->ai_addr; //No need to check for NULL, errors are caught above
	freeaddrinfo(result);
	return addr;
}
