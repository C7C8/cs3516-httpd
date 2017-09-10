
#include <netinet/in.h>

class HTTPResponderThread {
public:
	HTTPResponderThread(int connection, sockaddr_in* clientAddr);
};
