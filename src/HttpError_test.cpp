#include "HttpError.hpp"
#include "utest.h"

UTEST(HttpError, minimal) {
  HttpError error(404, "Not Found");
  ASSERT_STREQ("Not Found", error.what());
  int code = error.code();
  ASSERT_EQ(404, code);
}