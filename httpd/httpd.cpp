#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <queue>
#include <cstring>
#include "cmdline.h"
#include "HTTPHeader.h"
#include "HTTPResponder.h"
using std::cout;
using std::cerr;
using std::endl;
using std::queue;

int main(int argc, char* argv[]){
	gengetopt_args_info args;
	cmdline_parser(argc, argv, &args);

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

	cout << "Starting server with " << args.threads_arg << " threads on INADDR_ANY:" << args.port_arg << endl << endl;
	queue<HTTPResponder*> threadQueue;
	listen(netsocket, 0);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
	while (true){
		//Main server loop. Accept connections as they come up, spin up a new responder thread
		//to address each of them. That part of the program is currently unimplemented.
		socklen_t socklen; //?
		sockaddr_in clientAddr;
		int connection = accept(netsocket, (sockaddr*)&clientAddr, &socklen);
		if (args.verbose_given) {
			cout << "ACCEPTED CLIENT CONNECTION (";
			cout << threadQueue.size() << "/" << args.threads_arg << " active threads)" << endl;
		}

		//Put the new responder thread at the end of the queue. If there isn't enough room,
		//wait for the oldest thread to finish execution.
		HTTPResponder* responder = new HTTPResponder(connection, clientAddr, (bool)args.verbose_given);
		if (threadQueue.size() >= args.threads_arg){
			if (args.verbose_given)
				cout << "Waiting for oldest thread to complete execution" << endl;
			threadQueue.front()->forcejoin();
			delete threadQueue.front();
			threadQueue.pop();

			//...this is getting hacky now, but I'm really not in a condition variable mood right
			//now. Do some housekeeping to clean up old threads.
			queue<HTTPResponder*> temp;
			while (threadQueue.size() > 0){
				if (threadQueue.front()->join()) {
					if (args.verbose_given)
						cout << "Cleaned up thread" << endl;
					delete threadQueue.front();
					threadQueue.pop();
				}
				else {
					temp.push(threadQueue.front());
					threadQueue.pop();
				}
			}
			threadQueue = temp;

		}
		threadQueue.push(responder);
		responder->run();

		//Check the head of the queue to see if that thread is done or not. If so, kill!
		if (threadQueue.front()->join()){
			delete threadQueue.front();
			threadQueue.pop();
		}
	}
#pragma clang diagnostic pop
}