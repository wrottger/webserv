#include <iostream>
#include <map>
#include <sys/socket.h>
#include "Utils.hpp"
#include "Logger.hpp"


class CgiResponse {
	private:
		enum State {
			NO_RESPONSE,
			DOCUMENT_RESPONSE,
			LOCAL_REDIRECT,
			CLIENT_REDIRECT,
			CLIENT_REDIRECT_WITH_BODY,
			FINISHED
		};

		State _state;
		const std::string &_cgiBuffer;
		bool _isResponseBodyPresent;
		std::map<std::string, std::string> _responseHeaders;
		size_t _responseHeaderSize;
		size_t _responseBodySize;
		std::string _responseHeader;
		size_t _headerBytesSend;
		size_t _bodyBytesSend;
		const int &_fd;
		bool _headerSent;
		bool _bodySent;
		bool _isInternalRedirect;

		bool isValidStatusCode(const std::string &statusCode) const;
		bool isValidContentType(const std::string &line) const;
		int addHeaderField(const std::string &line);
		bool isHeaderFieldPresent(const std::string &key) const;
		int parseResponse();
		int parseHeader();
		int setState();
		bool isUrlPath(const std::string &path) const;
		bool isLocalPath(const std::string &path) const;
		int sendHeader();
		int sendBody();
		std::string createResponseHeader();

		CgiResponse();
		CgiResponse(const CgiResponse &other);
		CgiResponse &operator=(const CgiResponse &other);

	public:
		CgiResponse(const std::string &cgiBuffer, const int &fd);
		~CgiResponse();
		int sendResponse();
		bool isFinished() const;
		bool isInternalRedirect() const;
		std::string getInternalRedirectLocation() const;
};
