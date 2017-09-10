#include "HTTPResponder.h"
#include "HTTPHeader.h"

/**
 * Construct a new HTTP responder thread using a given connection and a client address.
 * This will allow for server multithreading.
 * @param connection Connection as returned by a call to accept().
 * @param clientAddr Address of connecting client, as populated by a call to accept()
 */
HTTPResponder::HTTPResponder(int connection, sockaddr_in *clientAddr, bool verbose) {
	this->connection = connection;
	this->clientAddr = clientAddr;
	this->verbose = verbose;
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

/**
 * Responder loop, will respond to a client's requests. May or may not explode.
 */
void HTTPResponder::respond() {
	//Basic HTTP response loop: read from connection until we get a double \r\n. Parse the
	//client's request and respond appropriately.
	char buf[8192] = {};
	while (true){
		int rdsize = read(connection, buf, 8192);
		if (rdsize <= 0){
			break;
		}

		HTTPHeader clientHeader(buf), responseHeader;
		if (verbose){
			cout << "Got a " << clientHeader.method() << " from a client, something about ";
			cout << clientHeader.filename() << endl;
			cout << (clientHeader.connection() ? "Keeping " : "Closing ") << "connection at end of session" << endl;
		}

		if (clientHeader.method() == "GET" || clientHeader.method() == "HEAD"){
			string filename, data;
			if (clientHeader.filename() == "/" || clientHeader.filename().empty())
				filename = "index.html";
			else
				filename = clientHeader.filename().substr(1, clientHeader.filename().size() - 1);

			if (verbose)
				cout << "Attempting to access file \"" << filename << "\" for client" << endl;

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
						cout << "Something bad happened that I didn't expect" << endl;
					responseHeader.status(SERVER_ERROR);
					responseHeader.statusStr("Internal server error");
				}
				string rsp = responseHeader.construct();
				write(connection, rsp.c_str(), rsp.size());
				close(connection);
				return;
			}
			else {
				//Open a file, read all of it into memory (no, this isn't a good idea)
				//Nothing like some well mixed C and C++!
				FILE* file = fopen(filename.c_str(), "rb");
				fseek(file, 0, SEEK_END);
				long long size = ftell(file);
				rewind(file);
				char* fileBuf = (char*)malloc(size);
				fread(fileBuf, sizeof(char), size, file);

				//Now construct our response
				responseHeader.status(OK);
				responseHeader.statusStr("OK");
				string rsp;
				if (clientHeader.method() == "GET")
					rsp = responseHeader.construct(fileBuf);
				else
					rsp = responseHeader.construct();
				free(fileBuf);

				write(connection, rsp.c_str(), rsp.size());
				close(connection);
				return;
			}
		}
		else {
			//We don't support that kind of method here. Make sure the client is aware of the problem
			responseHeader.status(NOT_IMPL);
			responseHeader.statusStr("Not implemented");
			responseHeader.connection(false);
			string rsp = responseHeader.construct();
			write(connection, rsp.c_str(), rsp.size());
			//if you cause me an error, I will end your connection whether you want it ended or not.
			close(connection);
			return;
		}

		if (!clientHeader.connection()){
			close(connection);
			return;
		}
	}
}
