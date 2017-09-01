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


	if (args.inputs_num != 2){
		cerr << "Incorrect number of arguments specified, please provide a URL and a port." << endl;
		exit(1);
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

		/*
    	 * ,-------,  ,  ,   ,-------,
    	 *  )  ,' /(  |\/|   )\ ',  (
    	 *   )'  /  \ (qp)  /  \  '(
    	 *    ) /___ \_\/(_/ ___\ (
    	 *     '    '-(   )-'    '
    	 *            )w^w(
    	 *            (W_W)
    	 *             ((
    	 *              ))					 |
    	 *             ((					/|\
    	 *              )  HERE BE DRAGONS //|\\
		 *									 |
		 *
		 *
		 * Those two lines of code above are the most magical printfs I have ever known, and I don't know why. They used to
		 * be outside the else-if, until I went through this code cleaning up (the program was working flawlessly)
		 * and realized that they were providing verbose output where there shouldn't be. So, I moved them to their
		 * present home.
		 *
		 * Instant chaos. Segfaults *every single time.* So I moved them back. All is well. What the hell? They're
		 * printfs! All I did is make them NOT run, why is that causing a segfault? Where is the segfault coming from,
		 * anyways? It's... oh, wait, it's way down below, in the content length parser. What?!
		 *
		 * Ok, so something strange is going on. If I turn verbose output back on, no more segfaults. If I move one of
		 * the printfs back outside again, no more segfaults either. Or, not always. Now it only segfaults *sometimes*.
		 * Yay, non-determinism! Maybe this somehow messes with the IO stuff a tiny bit? What if I fflush stdout or even
		 * the network socket? Nope, no difference. Check Wireshark... nope, server doesn't care either way, and the
		 * requests going out are identical. Everything is the same network-wise.
		 *
		 * So I took a look at the actual segfault location in the code to try and figure it out. It's the weird
		 * copy-buffer-to-temp line, so strtok doesn't eat my http data when I don't want it to. Maybe the content
		 * length is getting garbled? ...nope. All is well. Maybe I'm overrunni- HOLY **** THAT'S IT
		 *
		 * I had forgotten that strcpy can cause buffer overflows if you don't do it right. I didn't think it would be
		 * an issue because I was copying between buffers of the same size, but I guess it mattered; once I switched
		 * over to strncpy, everything stopped segfaulting (and presumably the CNN.com servers breathed a sigh of relief
		 * when I stopped hammering them with HTTP requests so I could test my non-deterministic bug).
		 *
		 * Bug fixed, right? Yup. Problem solved? Nope. I have no idea how that bug ever got revealed. Best I figure is
		 * that the buffer was overflowing by 1 byte, but how that's related to a pair of printfs *not* executing is beyond
		 * me. I've been writing C/C++ for about 5 years now, and this is just about the only code that has ever made me say
		 * "WHAT THE F***?!" out loud. I know those printfs should probably be `cout`'s and not printfs, but
		 * honestly, I'm too afraid to touch them. It took way too long to find and fix the first time through.
		 */
	}
	timeval startTime;
	gettimeofday(&startTime, nullptr);
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
		gettimeofday(&endTime, nullptr);
		int ms = ((endTime.tv_sec - startTime.tv_sec) * 1000) + ((endTime.tv_usec - startTime.tv_usec) / 1000);
		cerr << "RTT: " << ms << "ms" << endl; //Makes it easier to filter out data
	}

	//Extract the content length from the response, if there is one.
	int contentLength = 0;
	char tempBuf[MAX_BUF_SIZE];
	if (strstr(buf, "Content-Length: ") != nullptr) {
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
	//Resolve URL using magic
	addrinfo hints;
	addrinfo* result;
	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(hostname, NULL, &hints, &result) != 0){
		perror("Failed to resolve host");
		exit(1);
	}

	sockaddr_in addr;
	if (result != nullptr)
		addr = *(sockaddr_in*)result->ai_addr;
	freeaddrinfo(result);
	return addr;
}
