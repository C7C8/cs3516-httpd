#include "HTTPHeader.h"
#include <iostream>
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
	size_t methodEnd = header.find(' ', 0);
	cout << "First space at " << methodEnd << endl;
	hMethod = header.substr(0, methodEnd);
	cout << "Extracted method \"" << hMethod << "\"" << endl;

	//We don't care about the HTTP version
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
