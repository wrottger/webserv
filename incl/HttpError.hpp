#ifndef HTTPERROR_HPP
#define HTTPERROR_HPP

#include <exception>
#include <string>

class HttpError : public std::exception {
 public:
  ~HttpError()  throw() {}
  HttpError(int code, const std::string& message)
      : _code(code), _message(message) {}

  const char* what() { return _message.c_str(); }

  int code() { return _code; }

 private:
  int _code;
  std::string _message;
};

#endif