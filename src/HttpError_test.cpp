#include "HttpError.hpp"
#include "utest.h"

UTEST(HttpError, minimal)
{
  HttpError error(404, "Not Found");
  ASSERT_STREQ("Not Found", error.what());
  int code = error.code();
  ASSERT_EQ(404, code);
}

UTEST(HttpError, throwing)
{
  HttpError error(404, "Not Found");
  try {
    throw error;
  } catch (HttpError& e) {
    ASSERT_STREQ("Not Found", e.what());
    int code = e.code();
    ASSERT_EQ(404, code);
  }
}

UTEST(HttpError, empty)
{
  HttpError error(0, NULL);
  ASSERT_STREQ("", error.what());
  int code = error.code();
  ASSERT_EQ(404, code);
}
