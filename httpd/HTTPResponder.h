#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
using std::thread;
using std::string;
using std::cout;
using std::endl;

class HTTPResponder {
public:
	HTTPResponder(int connection, sockaddr_in clientAddr, bool verbose = false);
	~HTTPResponder();
	void run();
	bool join();
	void forcejoin();

private:
	void respond();

	int connection;
	sockaddr_in clientAddr;
	thread* t;
	bool verbose;
	bool finished; //because thread.joinable() doesn't work like I want it to work.
};
