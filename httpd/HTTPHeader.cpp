#include "HTTPHeader.h"
using namespace std;

HTTPHeader::HTTPHeader() {
	parsedHeader = false;
	hStatus = OK;
	keepalive = false;
	hServerName = "crmyers-server-1.1";
	time_t now = time(0);
	hDate = *gmtime(&now);
}

/**
 * Using set parameters, construct an HTTP header to which the provided data (if any) is
 * appended to.
 * @return Constructed header as a string.
 */
string HTTPHeader::construct(string data){
	stringstream headerstr;
	char datestr[1000] = {'\0'};
	strftime(datestr, 1000, "%a, %d %b %Y %H:%M:%S %Z", &hDate);

	headerstr << "HTTP/1.1 " << hStatus << "\r\n";
	headerstr << "Connection: " << (keepalive ? "keep-alive" : "close") << "\r\n";
	headerstr << "Server: " << hServerName << "\r\n";
	headerstr << "Content-Length: " << data.size() << "\r\n";
	headerstr << "Date: " << datestr << "\r\n";
	headerstr << "\r\n" << data;

	return headerstr.str();
}

/**
 * Parse a recieved HTTP header. Not all fields will be extracted from it, just the
 * ones that a client can send and that this class has the ability to store.
 * @param header String containing the HTTP header sent by the client.
 */
void HTTPHeader::parse(string header) {
	parsedHeader = true;

	//Method: always the first word of the header.
	const size_t methodEnd = header.find(' ', 0);
	hMethod = header.substr(0, methodEnd);

	//Get filename
	const size_t httpVStart = header.find("HTTP", methodEnd);
	hFilename = header.substr(methodEnd + 1, httpVStart - 5); //1 for add space, 5 for HTTP-space.

	//Header line parsed. Extract the extra headers. This is done on a line-by-line basis, so for
	//that we need a loop and a whole lot of ifs. Should be fun!
	istringstream header_s(header);
	string line;
	getline(header_s, line); // strip off first line, we don't care about it
	while (getline(header_s, line)){
		if (line == "\r")
			break;
		const size_t dataStart = line.find(' ', 0);
		const string headerLine = line.substr(0, dataStart - 1);

		if (headerLine == "Host")
			hHost = line.substr(dataStart + 1, line.size() - dataStart - 2);
		if (headerLine == "Accept")
			hAccept = line.substr(dataStart + 1, line.size() - dataStart - 2);
		if (headerLine == "Connection")
			keepalive = (line.substr(dataStart + 1, line.size() - dataStart - 2) == "keep-alive");
		if (headerLine == "DNT")
			hDoNotTrack = (line.substr(dataStart + 1, line.size() - dataStart - 2) == "1");
		/*if (headerLine == "Date"){
		 * Hahaha, you thought I was going to implement date parsing? Or even try? There's a
		 * function for it somewhere, but I don't care to learn it!
		 * }
		 */
		if (headerLine == "User-Agent")
			hUserAgent = line.substr(dataStart + 1, line.size() - dataStart - 2);
	}
}

HTTPHeader::HTTPHeader(string header) {
	parse(header);
}

string HTTPHeader::method(){
	return hMethod;
}

void HTTPHeader::method(string method) {
	if (!parsedHeader)
		hMethod = method;
}

string HTTPHeader::filename(){
	return hFilename;
}

void HTTPHeader::status(HTTP_STATUS_CODE code){
	//No need to check for parsed data, this field is never filled by the client.
	hStatus = code;
}

HTTP_STATUS_CODE HTTPHeader::status(){
	return hStatus;
}

void HTTPHeader::connection(bool keepAlive){
	if (!parsedHeader)
		this->keepalive = keepAlive;
}

string HTTPHeader::host() {
	return hHost;
}

string HTTPHeader::accept(){
	return hAccept;
}

bool HTTPHeader::connection() {
	return keepalive;
}

void HTTPHeader::contentType(string mimetype) {
	if (!parsedHeader)
		hContentType = mimetype;
}

string HTTPHeader::contentType() {
	return hContentType;
}

bool HTTPHeader::doNotTrack() {
	return hDoNotTrack;
}

tm HTTPHeader::date(){
	return hDate;
}

void HTTPHeader::date(tm date) {
	if (!parsedHeader)
		hDate = date;
}

string HTTPHeader::userAgent() {
	return hUserAgent;
}

void HTTPHeader::server(string server) {
	//No need to check for parsed data, the client will never send this
	hServerName = server;
}

string HTTPHeader::server() {
	return hServerName;
}

time_t HTTPHeader::ifUnmodifiedSince() {
	return hUnmodifiedSince;
}
