#include <string>
#include <sstream>
#include <ctime>
using std::stringstream;
using std::string;
using std::time;

/**
 * HTTP status codes. This list used to be much longer, then I trimmed it down to the ones actualyl used here.
 */
enum HTTP_STATUS_CODE {
	OK				= 200,
	FORBIDDEN		= 403,
	NOT_FOUND		= 404,
	SERVER_ERROR	= 500,
	NOT_IMPL		= 501
};

/**
 * Class for constructing or parsing HTTP headers.
 */
class HTTPHeader {
public:
	HTTPHeader();
	HTTPHeader(string header);
	string construct(long long dataSize = 0);
	void parse(string header);

	/** Functions for various HTTP header fields.**/
	string method();
	void method(string method);
	string filename();
	void status(HTTP_STATUS_CODE code);
	void statusStr(string stat);
	HTTP_STATUS_CODE status();
	string host();
	string accept();
	void connection(bool keepalive);
	bool connection();
	void contentType(string mimetype);
	string contentType();
	bool doNotTrack();
	void date(tm date);
	tm date();
	string userAgent();
	void server(string server);
	string server();
	time_t ifUnmodifiedSince();

private:
	bool parsedHeader;
	string hMethod;
	string hFilename;
	HTTP_STATUS_CODE hStatus;
	string hStatusStr;
	string hHost;
	string hAccept;
	bool keepalive;
	string hContentType;
	bool hDoNotTrack;
	tm hDate;
	string hUserAgent;
	string hServerName;
	time_t hUnmodifiedSince;
};
