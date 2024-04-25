#include "HttpHeader.hpp"
#include "HttpError.hpp"
#include <iostream>
#include "utest.h"

UTEST(HttpHeader, setup) {
  HttpHeader r;
  ASSERT_FALSE(r.isComplete());
}

UTEST(HttpHeader, identifyMethod)
{
  HttpHeader r;
  r.parseBuffer("GET ");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  HttpHeader r2;
  r2.parseBuffer("POST ");
  ASSERT_STREQ("POST", r2.getMethod().c_str());
  HttpHeader r3;
  r3.parseBuffer("PUT ");
  ASSERT_STREQ("PUT", r3.getMethod().c_str());
}

UTEST(HttpHeader, minimal)
{
  HttpHeader r;
  ASSERT_EQ(r.parseBuffer("GET / HTTP/1.1\r\nhost: localhost\r\n\r\n"), (size_t)35);
  EXPECT_STREQ_MSG("GET", r.getMethod().c_str(), r.getMethod().c_str());
  EXPECT_STREQ_MSG("/", r.getPath().c_str(), r.getPath().c_str());
  EXPECT_STREQ_MSG("localhost", r.getHeader("host").c_str(), r.getHeader("host").c_str());
}

UTEST(HttpHeader, null)
{
  HttpHeader r;
  r.parseBuffer(NULL);
  r.parseBuffer(NULL);
  r.parseBuffer("G");
  ASSERT_FALSE(r.isComplete());
}

UTEST(HttpHeader, size) {
  HttpHeader r;
  size_t size = r.parseBuffer("POST / HTTP/1.1\r\nhost: localhost\r\n\r\n");
  size_t expected = 36;
  ASSERT_EQ(expected, size);
  ASSERT_TRUE(r.isComplete());
}

UTEST(HttpHeader, longerPath)
{
  HttpHeader r;
  r.parseBuffer("GET /rooot/user/ratntant/index.html ");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/rooot/user/ratntant/index.html", r.getPath().c_str());
}

UTEST(HttpHeader, localhost)
{
  HttpHeader r;
  r.parseBuffer("GET http://localhost.com/ HTTP/1.1\r\nHost: localhost\r\n\r\n");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/", r.getPath().c_str());
  ASSERT_STREQ("localhost", r.getHeader("host").c_str());
  ASSERT_TRUE(r.isComplete());
}

UTEST(HttpHeader, wrongWhitespaces)
{
  HttpHeader r;
  r.parseBuffer("GET /HTTP/1.1\r\nhost: localhost\r\n\r\n");
  EXPECT_EQ(400, r.getError().code());
  HttpHeader r2;
  r2.parseBuffer("GET / HTTP/1.1\r\nhost :localhost\r\n\r\n");
  EXPECT_EQ(400, r2.getError().code());
  HttpHeader r3;
  r3.parseBuffer("GET / HTTP/1.1\r\nhost: localhost \r\n\r\n");
  EXPECT_EQ(400, r3.getError().code());
}

UTEST(HttpHeader, nlLineneding)
{
  HttpHeader r;
  r.parseBuffer("GET / HTTP/1.1\nhost: localhost\r\n\r\n");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/", r.getPath().c_str());
  ASSERT_TRUE(r.isComplete());
  ASSERT_STREQ("localhost", r.getHeader("host").c_str());
}

UTEST(HttpHeader, parseAtOnce) {
  HttpHeader r;
  size_t size = r.parseBuffer("GET / HTTP/1.1\r\nhost: localhost\r\n\r\n");
  size_t expected = 35;
  ASSERT_EQ(expected, size);
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/", r.getPath().c_str());
  ASSERT_STREQ("localhost", r.getHeader("host").c_str());
  ASSERT_TRUE(r.isComplete());
}

UTEST(HttpHeader, wrongVersion){
  HttpHeader r;
  r.parseBuffer("GET / HTTP/1.0\r\nhost: localhost\r\n\r\n");
  ASSERT_EQ(505, r.getError().code());
}

UTEST(HttpHeader, absolutePath)
{
  HttpHeader r;
  r.parseBuffer("GET http://httpbin.org/index.html?a=1#title HTTP/1.1\r\nhost: localhost\r\n\r\n");
  ASSERT_STREQ("GET", r.getMethod().c_str());
  ASSERT_STREQ("/index.html", r.getPath().c_str());
  ASSERT_STREQ("localhost", r.getHeader("host").c_str());
  ASSERT_STREQ("a=1", r.getQuery().c_str());
  ASSERT_TRUE(r.isComplete());
}

UTEST(HttpHeader, charByChar) {
  HttpHeader r;
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

UTEST(HttpHeader, missingHost) {
  HttpHeader r;
  r.parseBuffer("GET / HTTP/1.1\r\n\r\n");
  ASSERT_EQ(400, r.getError().code());
}

UTEST(HttpHeader, CRLFtooEarly) {
  HttpHeader r;
  r.parseBuffer("GET\r\n/ HTTP/1.0");
  ASSERT_EQ(400, r.getError().code());
}

UTEST(HttpHeader, noCRLF) {
  HttpHeader r;
  r.parseBuffer("GET / HTTP/1.1 hostname: localhost");
  ASSERT_EQ(400, r.getError().code());
}

UTEST(HttpHeader, postRequest)
{
  HttpHeader r;
  r.parseBuffer("POST / HTTP/1.1\r\nhost: localhost\r\ncontent-length: 5\r\n\r\n12345");
  ASSERT_STREQ("POST", r.getMethod().c_str());
  ASSERT_STREQ("/", r.getPath().c_str());
  ASSERT_STREQ("localhost", r.getHeader("host").c_str());
  ASSERT_STREQ("5", r.getHeader("content-length").c_str());
}

UTEST(HttpHeader, percentDecode)
{
  HttpHeader r;
  r.parseBuffer("GET /%20 HTTP/1.1\r\nhost: localhost\r\n\r\n");
  ASSERT_STREQ("/ ", r.getPath().c_str());
}
