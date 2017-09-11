#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <signal.h>
#include <sys/stat.h>
#include "cmdline.h"
#include "HTTPHeader.h"
#include "HTTPResponder.h"
using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char* argv[]){
	gengetopt_args_info args;
	cmdline_parser(argc, argv, &args);

	//Daemonize me
	if (args.daemon_given){
		cout << "Forking to background, run with -k to kill" << endl;
		if (daemon(1, 1) != 0){
			perror("Failed to fork into background");
			exit(1);
		}

		int pidf = open("/tmp/httpd-server-pid", O_WRONLY | O_CREAT);
		fchmod(pidf, S_IRUSR | S_IWUSR);
		string pidstr = std::to_string(getpid());
		cout << "Writing PID " << pidstr << " to file" << endl;
		write(pidf, pidstr.c_str(), pidstr.size());
		close(pidf);
	}

	if (args.kill_given){
		int pidf = open("/tmp/httpd-server-pid", O_RDONLY);
		if (pidf < 0){
			cerr << "Failed to open PID file, is a server running?" << endl;
			exit(1);
		}
		char buf[16] = {'\0'};
		read(pidf, buf, 16);
		close(pidf);
		remove("/tmp/httpd-server-pid");
		cout << "Killing PID " << buf << endl;
		if (kill(atoi(buf), SIGTERM) != 0){
			perror("Failed kill daemon");
			exit(1);
		}
		exit(0);
	}


	//Thread count checker
	if (!args.threads_given || args.threads_arg < 1){
		args.threads_arg = thread::hardware_concurrency();
		if (args.verbose_given) {
			cout << "Valid maximum thread count not supplied, setting to autodetected value " << args.threads_arg << endl;
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
	listen(netsocket, 0);
	HTTPResponder** threads = (HTTPResponder**)malloc(sizeof(HTTPResponder*)*args.threads_arg);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
	while (true){
		//Main server loop. Accept connections as they come up, spin up new threads as needed.

		socklen_t socklen; //?
		sockaddr_in clientAddr;
		int connection = accept(netsocket, (sockaddr*)&clientAddr, &socklen);
		if (args.verbose_given) {
			cout << "ACCEPTED CLIENT CONNECTION (";
			cout << HTTPResponder::getCount() << "/" << args.threads_arg << " currently active threads)" << endl;
		}

		//Find a slot for the thread to go in first, but first make sure one exists.
		int slot = 0;
		if (HTTPResponder::getCount() >= args.threads_arg && args.verbose_given)
			cout << "OUT OF THREADS, there may be a delay" << endl;
		for (int i = 0;; i++){
			if (i >= args.threads_arg)
				i = 0;

			if (threads[i] == nullptr){
				slot = i;
				break;
			}
			usleep(100000); //Check every 100 ms for a new thread to open up
		}
		HTTPResponder* responder = new HTTPResponder(connection, clientAddr, threads + slot, (bool)args.verbose_given);
		threads[slot] = responder;
		responder->run();
		//No more worrying over the responder again, the thing will delete itself whenever it's
		//finished its job. That can't possibly end badly, can it?
	}
	free(threads);
	close(netsocket);
#pragma clang diagnostic pop
}