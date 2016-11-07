#include "Expected.h"
#include <cstdio>
#include <string>

unsigned errors = 0;

#define ASSERT_EQ(A, B) do {                            \
    if ((A) != (B)) {                                   \
      fprintf(stderr, "Failed: %s != %s\n", #A, #B);    \
      ++errors;                                         \
    }                                                   \
  } while (0)

void test_unexpected_type() {
  auto u = unexpected_type<int>(42);
  ASSERT_EQ(u.value(), 42);
  constexpr auto c = make_unexpected(42);
  ASSERT_EQ(c.value(), 42);
  ASSERT_EQ(u, c);
  ASSERT_EQ(u != c, false);
  ASSERT_EQ(u < c, false);
  ASSERT_EQ(u > c, false);
  ASSERT_EQ(u <= c, true);
  ASSERT_EQ(u >= c, true);
  auto s = make_unexpected(std::string("oops"));
  ASSERT_EQ(s.value(), "oops");
}

int main() {
  test_unexpected_type();

  return errors != 0;
}
