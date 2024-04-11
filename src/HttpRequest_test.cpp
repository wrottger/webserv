#include "HttpRequest.hpp"
#include "HttpError.hpp"
#include "utest.h"

UTEST(Parsing, minimal) {
  HttpRequest r;
  r.parseLine("GET / HTTP/1.1\r\n");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/", r.getTarget().c_str());
  ASSERT_STREQ("HTTP/1.1", r.getVersion().c_str());
  ASSERT_STREQ("localhost", r.getHeader("host").c_str());
}

UTEST(Parsing, missingHost) {
  HttpRequest r;
  r.parseLine("GET / HTTP/1.1\r\n\r\n");
  EXPECT_FALSE(r.isComplete());
}

UTEST(Parsing, missingCRLF) {
  HttpRequest r;
  ASSERT_EXCEPTION(r.parseLine("GET\r\n/ HTTP/1.0"), HttpError);
}
