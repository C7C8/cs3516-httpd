#include "HTTPResponder.h"

/**
 * Construct a new HTTP responder thread using a given connection and a client address.
 * This will allow for server multithreading.
 * @param connection Connection as returned by a call to accept().
 * @param clientAddr Address of connecting client, as populated by a call to accept()
 */
HTTPResponder::HTTPResponder(int connection, sockaddr_in *clientAddr) {
	this->connection = connection;
	this->clientAddr = clientAddr;
}

/**
 * Deconstruct the responder -- this joins the thread (if it is joinable), frees memory
 * allocated to the thread and the client address, and closes the connection if not closed
 * already.
 */
HTTPResponder::~HTTPResponder() {
	if (t != nullptr) {
		if (t->joinable())
			t->join();
		delete t;
	}
	delete clientAddr;
	close(connection);
}

/**
 * Run the thread (specifically by creating a new std::thread that calls into
 * private function respond()
 */
void HTTPResponder::run(){
	t = new thread(&HTTPResponder::respond, this);
}

/**
 * Join the thread if possible.
 */
void HTTPResponder::join(){
	if (t != nullptr)
		t->join();
}

void HTTPResponder::respond() {

}
