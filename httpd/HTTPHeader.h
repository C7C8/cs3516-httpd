#include <string>
#include <sstream>
#include <ctime>
#include <tuple>
using std::stringstream;
using std::string;
using std::tuple;
using std::time;

/**
 * List of HTTP status codes. Will all of these be used? Nope. Am I purposefully going overboard here? Yup.
 */
enum HTTP_STATUS_CODE {
	CONTINUE		= 100,
	SWTCH_PROTO		= 101,
	PROCESSING		= 102,
	OK				= 200,
	CREATED			= 201,
	ACCEPTED		= 202,
	NON_AUTH_INFO	= 203,
	NO_CONTENT		= 204,
	RESET_CONTENT	= 205,
	PARTIAL_CONTENT	= 206,
	MULTISTAT		= 207,
	PREV_REPORT		= 208,
	IM_USED			= 226,
	MULTI_CHOICE	= 300,
	MOVED_PERM		= 301,
	FOUND			= 302,
	SEE_OTHER		= 303,
	NOT_MODIFIED	= 304,
	USE_PROXY		= 305, //not used by firefox
	TEMP_REDIRECT	= 307,
	PERM_REDIRECT 	= 308,
	BAD_REQUEST		= 400,
	UNAUTH			= 401,
	PAYMENT_REQ		= 402, //generally not used
	FORBIDDEN		= 403,
	NOT_FOUND		= 404,
	METH_NOT_ALLW	= 405,
	NOT_ACCEPT		= 406,
	PROX_AUTH_REQ	= 407,
	REQ_TIMEOUT		= 408,
	CONFLICT		= 409,
	GONE			= 410,
	LENGTH_REQ		= 411,
	PRECOND_FAIL	= 412,
	HUGE_PAYLOAD	= 413,
	HUGE_URI		= 414,
	UNSUP_MIME		= 415,
	RNG_UNSAT		= 416,
	EXPECT_FAIL		= 417,
	TEAPOT			= 418,
	MISDIRECT_REQ	= 421,
	UNPROC_ENT		= 422,
	LOCKED			= 423,
	FAILED_DEP		= 424,
	UPGRD_REQ		= 427,
	PRECOND_REQ		= 428,
	TOO_MANY_REQ	= 429,
	HUGE_HEADER		= 431,
	TAKEDOWN		= 451, //apparently this code is in reference to Fahrenheit 451. Neat!
	SERVER_ERROR	= 500,
	NOT_IMPL		= 501,
	BAD_GATEWAY		= 502,
	SERV_UNAVAIL	= 503,
	GATEWAY_TIMEOUT	= 504,
	HTTPV_UNSUP		= 505,
	VARIANT_NEGT	= 506,
	OUT_OF_SPACE	= 507,
	LOOP			= 508,
	NOT_EXTENDED	= 510,
	NET_AUTH_REQ	= 511
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
