#ifndef HTTPERROR_HPP
#define HTTPERROR_HPP

#include <exception>
#include <string>

class HttpError : public std::exception {
 public:
  ~HttpError()  throw() {}
  HttpError() : _code(0), _message("") {}
  HttpError(int code, const std::string& message)
      : _code(code), _message(message) {}

  const char* what() const throw () { return _message.c_str(); }

  int code() const { return _code; }
  std::string message() const { return _message; }

 private:
  int _code;
  std::string _message;
};

#endif