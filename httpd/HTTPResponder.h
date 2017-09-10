#include <thread>
#include <unistd.h>
#include <netinet/in.h>
using std::thread;

class HTTPResponder {
public:
	HTTPResponder(int connection, sockaddr_in* clientAddr);
	~HTTPResponder();
	void run();
	void join();

private:
	void respond();

	int connection;
	sockaddr_in* clientAddr;
	thread* t;
};
