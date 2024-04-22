#include "HttpRequest.hpp"
#include "HttpError.hpp"
#include <iostream>
#include "utest.h"

UTEST(HttpRequest, setup) {
  HttpRequest r;
  ASSERT_FALSE(r.isComplete());
}

UTEST(HttpRequest, identifyMethod)
{
  HttpRequest r;
  r.parseBuffer("GET ");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  HttpRequest r2;
  r2.parseBuffer("POST ");
  ASSERT_STREQ("POST", r2.getMethod().c_str());
  HttpRequest r3;
  r3.parseBuffer("PUT ");
  ASSERT_STREQ("PUT", r3.getMethod().c_str());
}

UTEST(HttpRequest, minimal)
{
  HttpRequest r;
  r.parseBuffer("GET / ");
  EXPECT_STREQ_MSG("GET", r.getMethod().c_str(), r.getMethod().c_str());
  EXPECT_STREQ_MSG("/", r.getPath().c_str(), r.getPath().c_str());
  //EXPECT_STREQ_MSG("localhost", r.getHeader("host").c_str(), r.getHeader("host").c_str());
}

UTEST(HttpRequest, null)
{
  HttpRequest r;
  r.parseBuffer(NULL);
  r.parseBuffer(NULL);
  r.parseBuffer("G");
  ASSERT_FALSE(r.isComplete());
}

UTEST(HttpRequest, size) {
  HttpRequest r;
  size_t size = r.parseBuffer("GET / HTTP/1.1\r\nhost: localhost\r\n\r\n");
  size_t expected = 41;
  ASSERT_EQ(expected, size);
}

UTEST(HttpRequest, wrongWhitespaces)
{
  HttpRequest r;
  ASSERT_EXCEPTION(r.parseBuffer("GET /HTTP/1.1\r\nhost: localhost\r\n\r\n"), HttpError);
  HttpRequest r2;
  ASSERT_EXCEPTION(r2.parseBuffer("GET / HTTP/1.1\r\nhost :localhost\r\n\r\n"), HttpError);
  HttpRequest r3;
  ASSERT_EXCEPTION(r3.parseBuffer("GET / HTTP/1.1\r\nhost: localhost \r\n\r\n"), HttpError);
}

UTEST(HttpRequest, nlLineneding)
{
  HttpRequest r;
  r.parseBuffer("GET / HTTP/1.1\nhost: localhost\r\n\r\n");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/", r.getPath().c_str());
  //ASSERT_STREQ("localhost", r.getHeader("host").c_str());
}

UTEST(HttpRequest, parseAtOnce) {
  HttpRequest r;
  size_t size = r.parseBuffer("GET / HTTP/1.1\r\nhost: localhost\r\n\r\n");
  size_t expected = 33;
  ASSERT_EQ(expected, size);
  ASSERT_TRUE(r.isComplete());
}

UTEST(HttpRequest, charByChar) {
  HttpRequest r;
  r.parseBuffer("G");
  r.parseBuffer("E");
  r.parseBuffer("T");
  r.parseBuffer(" ");
  r.parseBuffer("/");
  r.parseBuffer(" ");
  r.parseBuffer("H");
  r.parseBuffer("T");
  r.parseBuffer("T");
  r.parseBuffer("P");
  r.parseBuffer("/");
  r.parseBuffer("1");
  r.parseBuffer(".");
  r.parseBuffer("1");
  r.parseBuffer("\r");
  r.parseBuffer("\n");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/", r.getPath().c_str());
  ASSERT_FALSE(r.isComplete());
}

UTEST(HttpRequest, missingHost) {
  HttpRequest r;
  ASSERT_EXCEPTION(r.parseBuffer("GET / HTTP/1.1\r\n\r\n"), HttpError);
}

UTEST(HttpRequest, CRLFtooEarly) {
  HttpRequest r;
  ASSERT_EXCEPTION(r.parseBuffer("GET\r\n/ HTTP/1.0"), HttpError);
}

UTEST(HttpRequest, noCRLF) {
  HttpRequest r;
  ASSERT_EXCEPTION(r.parseBuffer("GET / HTTP/1.0 hostname: localhost"), HttpError);
}

UTEST(HttpRequest, postRequest)
{
  HttpRequest r;
  r.parseBuffer("POST / HTTP/1.1\r\nhost: localhost\r\ncontent-length: 5\r\n\r\n12345");
  ASSERT_STREQ("POST", r.getMethod().c_str());
  ASSERT_STREQ("12345", r.getBody().c_str());
}
