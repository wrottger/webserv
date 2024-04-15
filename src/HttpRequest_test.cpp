#include "HttpRequest.hpp"
#include "HttpError.hpp"
#include "utest.h"

UTEST(HttpRequest, setup) {
  HttpRequest r;
  ASSERT_FALSE(r.isComplete());
}

UTEST(HttpRequest, null)
{
  HttpRequest r;
  r.parseLine(NULL);
  r.parseLine(NULL);
  r.parseLine("G");
  ASSERT_FALSE(r.isComplete());
}

UTEST(HttpRequest, size) {
  HttpRequest r;
  size_t size = r.parseLine("GET / HTTP/1.1\r\nhost: localhost\r\n\r\n");
  size_t expected = 33;
  ASSERT_EQ(expected, size);
}

UTEST(HttpRequest, wrongWhitespaces)
{
  HttpRequest r;
  ASSERT_EXCEPTION(r.parseLine("GET /HTTP/1.1\r\nhost: localhost\r\n\r\n"), HttpError);
  HttpRequest r2;
  ASSERT_EXCEPTION(r2.parseLine("GET / HTTP/1.1\r\nhost :localhost\r\n\r\n"), HttpError);
  HttpRequest r3;
  ASSERT_EXCEPTION(r3.parseLine("GET / HTTP/1.1\r\nhost: localhost \r\n\r\n"), HttpError);
}

UTEST(HttpRequest, nlLineneding)
{
  HttpRequest r;
  r.parseLine("GET / HTTP/1.1\nhost: localhost\r\n\r\n");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/", r.getTarget().c_str());
  ASSERT_STREQ("HTTP/1.1", r.getVersion().c_str());
  ASSERT_STREQ("localhost", r.getHeader("host").c_str());
}

UTEST(HttpRequest, parseAtOnce) {
  HttpRequest r;
  size_t size = r.parseLine("GET / HTTP/1.1\r\nhost: localhost\r\n\r\n");
  size_t expected = 33;
  ASSERT_EQ(expected, size);
  ASSERT_TRUE(r.isComplete());
}

UTEST(HttpRequest, charByChar) {
  HttpRequest r;
  r.parseLine("G");
  r.parseLine("E");
  r.parseLine("T");
  r.parseLine(" ");
  r.parseLine("/");
  r.parseLine(" ");
  r.parseLine("H");
  r.parseLine("T");
  r.parseLine("T");
  r.parseLine("P");
  r.parseLine("/");
  r.parseLine("1");
  r.parseLine(".");
  r.parseLine("1");
  r.parseLine("\r");
  r.parseLine("\n");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/", r.getTarget().c_str());
  ASSERT_STREQ("HTTP/1.1", r.getVersion().c_str());
  ASSERT_FALSE(r.isComplete());
}

UTEST(HttpRequest, minimal) {
  HttpRequest r;
  r.parseLine("GET / HTTP/1.1\r\nhost: localhost\r\n\r\n");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/", r.getTarget().c_str());
  ASSERT_STREQ("HTTP/1.1", r.getVersion().c_str());
  ASSERT_STREQ("localhost", r.getHeader("host").c_str());
}

UTEST(HttpRequest, missingHost) {
  HttpRequest r;
  ASSERT_EXCEPTION(r.parseLine("GET / HTTP/1.1\r\n\r\n"), HttpError);
}

UTEST(HttpRequest, CRLFtooEarly) {
  HttpRequest r;
  ASSERT_EXCEPTION(r.parseLine("GET\r\n/ HTTP/1.0"), HttpError);
}

UTEST(HttpRequest, noCRLF) {
  HttpRequest r;
  ASSERT_EXCEPTION(r.parseLine("GET / HTTP/1.0 hostname: localhost"), HttpError);
}

UTEST(HttpRequest, postRequest)
{
  HttpRequest r;
  r.parseLine("POST / HTTP/1.1\r\nhost: localhost\r\ncontent-length: 5\r\n\r\n12345");
  ASSERT_STREQ("POST", r.getMethod().c_str());
  ASSERT_STREQ("12345", r.getBody().c_str());
}
