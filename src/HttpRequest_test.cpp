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
  ASSERT_EQ(r.parseBuffer("GET / HTTP/1.1\r\nhost: localhost\r\n\r\n"), (size_t)35);
  EXPECT_STREQ_MSG("GET", r.getMethod().c_str(), r.getMethod().c_str());
  EXPECT_STREQ_MSG("/", r.getPath().c_str(), r.getPath().c_str());
  EXPECT_STREQ_MSG("localhost", r.getHeader("host").c_str(), r.getHeader("host").c_str());
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
  size_t size = r.parseBuffer("POST / HTTP/1.1\r\nhost: localhost\r\n\r\n");
  size_t expected = 36;
  ASSERT_EQ(expected, size);
  ASSERT_TRUE(r.isComplete());
}

UTEST(HttpRequest, longerPath)
{
  HttpRequest r;
  r.parseBuffer("GET /rooot/user/ratntant/index.html ");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/rooot/user/ratntant/index.html", r.getPath().c_str());
}

UTEST(HttpRequest, localhost)
{
  HttpRequest r;
  r.parseBuffer("GET http://localhost.com/ HTTP/1.1\r\nhost: localhost\r\n\r\n");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/", r.getPath().c_str());
  ASSERT_STREQ("localhost", r.getHeader("host").c_str());
  ASSERT_TRUE(r.isComplete());
}

UTEST(HttpRequest, wrongWhitespaces)
{
  HttpRequest r;
  r.parseBuffer("GET /HTTP/1.1\r\nhost: localhost\r\n\r\n");
  EXPECT_EQ(400, r.getError().code());
  HttpRequest r2;
  r2.parseBuffer("GET / HTTP/1.1\r\nhost :localhost\r\n\r\n");
  EXPECT_EQ(400, r2.getError().code());
  HttpRequest r3;
  r3.parseBuffer("GET / HTTP/1.1\r\nhost: localhost \r\n\r\n");
  EXPECT_EQ(400, r3.getError().code());
}

UTEST(HttpRequest, nlLineneding)
{
  HttpRequest r;
  r.parseBuffer("GET / HTTP/1.1\nhost: localhost\r\n\r\n");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/", r.getPath().c_str());
  ASSERT_TRUE(r.isComplete());
  ASSERT_STREQ("localhost", r.getHeader("host").c_str());
}

UTEST(HttpRequest, parseAtOnce) {
  HttpRequest r;
  size_t size = r.parseBuffer("GET / HTTP/1.1\r\nhost: localhost\r\n\r\n");
  size_t expected = 35;
  ASSERT_EQ(expected, size);
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/", r.getPath().c_str());
  ASSERT_STREQ("localhost", r.getHeader("host").c_str());
  ASSERT_TRUE(r.isComplete());
}

UTEST(HttpRequest, wrongVersion){
  HttpRequest r;
  r.parseBuffer("GET / HTTP/1.0\r\nhost: localhost\r\n\r\n");
  ASSERT_EQ(505, r.getError().code());
}

UTEST(HttpRequest, absolutePath)
{
  HttpRequest r;
  r.parseBuffer("GET http://httpbin.org/index.html?a=1#title HTTP/1.1\r\nhost: localhost\r\n\r\n");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/index.html", r.getPath().c_str());
  ASSERT_STREQ("localhost", r.getHeader("host").c_str());
  ASSERT_STREQ("a=1", r.getQuery().c_str());
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
  r.parseBuffer("h");
  r.parseBuffer("o");
  r.parseBuffer("s");
  r.parseBuffer("t");
  r.parseBuffer(":");
  r.parseBuffer(" ");
  r.parseBuffer("l");
  r.parseBuffer("o");
  r.parseBuffer("c");
  r.parseBuffer("a");
  r.parseBuffer("l");
  r.parseBuffer("h");
  r.parseBuffer("o");
  r.parseBuffer("s");
  r.parseBuffer("t");
  r.parseBuffer("\r");
  r.parseBuffer("\n");
  r.parseBuffer("\r");
  r.parseBuffer("\n");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/", r.getPath().c_str());
}

UTEST(HttpRequest, missingHost) {
  HttpRequest r;
  r.parseBuffer("GET / HTTP/1.1\r\n\r\n");
  ASSERT_EQ(400, r.getError().code());
}

UTEST(HttpRequest, CRLFtooEarly) {
  HttpRequest r;
  r.parseBuffer("GET\r\n/ HTTP/1.0");
  ASSERT_EQ(400, r.getError().code());
}

UTEST(HttpRequest, noCRLF) {
  HttpRequest r;
  r.parseBuffer("GET / HTTP/1.1 hostname: localhost");
  ASSERT_EQ(400, r.getError().code());
}

UTEST(HttpRequest, postRequest)
{
  HttpRequest r;
  r.parseBuffer("POST / HTTP/1.1\r\nhost: localhost\r\ncontent-length: 5\r\n\r\n12345");
  ASSERT_STREQ("POST", r.getMethod().c_str());
}

UTEST(HttpRequest, percentDecode)
{
  HttpRequest r;
  r.parseBuffer("GET /%20 HTTP/1.1\r\nhost: localhost\r\n\r\n");
  ASSERT_STREQ("/ ", r.getPath().c_str());
}
