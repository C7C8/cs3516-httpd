#include "HTTPResponder.h"
#include "HTTPHeader.h"

/**
 * Construct a new HTTP responder thread using a given connection and a client address.
 * This will allow for server multithreading.
 * @param connection Connection as returned by a call to accept().
 * @param clientAddr Address of connecting client, as populated by a call to accept()
 */
HTTPResponder::HTTPResponder(int connection, sockaddr_in clientAddr, bool verbose) {
	this->connection = connection;
	this->clientAddr = clientAddr;
	this->verbose = verbose;
	finished = false;
}

/**
 * Deconstruct the responder -- this joins the thread (if it is joinable), frees memory
 * allocated to the thread and the client address, and closes the connection if not closed
 * already.
 */
HTTPResponder::~HTTPResponder() {
	if (t != nullptr) {
		t->join();
		delete t;
	}
	close(connection);
}

/**
 * Run the thread (specifically by creating a new std::thread that calls into
 * private function respond())
 */
void HTTPResponder::run(){
	t = new thread(&HTTPResponder::respond, this);
}

/**
 * Join the thread if possible.
 * @return True if the thread was joined, false if the thread is still running and was thus
 * not joined.
 */
bool HTTPResponder::join(){
	if (t != nullptr && finished){
		if (verbose)
			cout << "Ending a finished thread" << endl;
		t->join();
		delete t;
		t = nullptr;
		return true;
	}
	if (verbose)
		cout << "Thread not ready to be joined yet" << endl;
	return false;
}

/**
 * Immediately join thread. This may result in a delay.
 */
void HTTPResponder::forcejoin() {
	if (verbose)
		cout << "Force joining a thread" << endl;
	if (t != nullptr){
		t->join();
		delete t;
		t = nullptr;
	}
}

/**
 * Responder loop, will respond to a client's requests. May or may not explode.
 */
void HTTPResponder::respond() {
	//Basic HTTP response loop: read from connection until we get a double \r\n. Parse the
	//client's request and respond appropriately.
	char buf[8192] = {};
	while (true){
		int rdsize = read(connection, buf, 8192);
		if (rdsize <= 0) {
			if (verbose)
				printf("TERMINATING CONNECTION: %d\n\n", rdsize);
			
			close(connection);
			finished = true; //I hate this hack
			return;
		}

		HTTPHeader clientHeader(buf), responseHeader;
		if (verbose){
			cout << "Client: " << clientHeader.method() << " " << clientHeader.filename() << ", then ";
			cout << (clientHeader.connection() ? "KEEPALIVE" : "CLOSE") << endl;
		}

		if (clientHeader.method() == "GET" || clientHeader.method() == "HEAD"){
			string filename, data;
			if (clientHeader.filename() == "/" || clientHeader.filename().empty())
				filename = "index.html";
			else
				filename = clientHeader.filename().substr(1, clientHeader.filename().size() - 1);

			if (verbose)
				cout << "Accessing file " << filename << endl;

			if (access(filename.c_str(), F_OK | R_OK) == -1){
				if (errno == EACCES){
					if (verbose)
						cout << "Invalid permissions on file" << endl;
					responseHeader.status(FORBIDDEN);
					responseHeader.statusStr("Forbidden");
				}
				else if (errno == ENOENT){
					if (verbose)
						cout << "File doesn't exist" << endl;
					responseHeader.status(NOT_FOUND);
					responseHeader.statusStr("Not found");
				}
				else {
					if (verbose)
						cout << "Other file-based error" << endl;
					responseHeader.status(SERVER_ERROR);
					responseHeader.statusStr("Internal server error");
				}
				string rsp = responseHeader.construct();
				write(connection, rsp.c_str(), rsp.size());
				write(connection, "Error", 6);
			}
			else {
				//Open a file, read all of it into memory (no, this isn't a good idea)
				//Nothing like some well mixed C and C++!
				FILE* file = fopen(filename.c_str(), "rb");
				fseek(file, 0, SEEK_END);
				long long size = ftell(file);
				rewind(file);
				char* fileBuf = (char*)malloc(size);
				//I hope your RAM is as big as this file is
				fread(fileBuf, sizeof(char), size, file);

				//Now construct our response
				responseHeader.status(OK);
				responseHeader.statusStr("OK");
				string rsp = responseHeader.construct(size);
				write(connection, rsp.c_str(), rsp.size());
				write(connection, fileBuf, size);
				free(fileBuf);
			}
		}
		else {
			//Method that we can't respond to
			if (verbose)
				cout << "CLIENT BAD METHOD" << endl;
			responseHeader.status(NOT_IMPL);
			responseHeader.statusStr("Not implemented");
			responseHeader.connection(false);
			string rsp = responseHeader.construct();
			write(connection, rsp.c_str(), rsp.size());
		}

		if (!clientHeader.connection()){
			if (verbose)
				cout << "TERMINATING CONNECTION: CLOSE" << endl;
			close(connection);
			finished = true;
			return;
		}
	}
}
