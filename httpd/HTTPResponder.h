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
	HTTPResponder(int connection, sockaddr_in clientAddr, HTTPResponder** threadList, bool verbose = false);
	~HTTPResponder();
	void run();
	static int getCount() {return count;}

private:
	void respond();

	HTTPResponder** threadList;
	int connection;
	sockaddr_in clientAddr;
	thread t;
	bool verbose;
	static int count;
};
