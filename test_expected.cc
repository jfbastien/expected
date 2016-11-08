#include "Expected.h"
#include <cstdio>
#include <string>

const char* oops = "oops";
unsigned errors = 0;

#define ASSERT_EQ(A, B) do {                                \
      if ((A) != (B)) {                                     \
          fprintf(stderr, "Failed: %s != %s\n", #A, #B);    \
          ++errors;                                         \
      }                                                     \
  } while (0)

void test_unexpected_type() {
    {
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
    }
    {
        auto c = make_unexpected(oops);
        ASSERT_EQ(c.value(), oops);
    }
    {
        auto s = make_unexpected(std::string(oops));
        ASSERT_EQ(s.value(), oops);
    }
}

void test_expected() {
    typedef expected<int, const char*> E;
    typedef expected<int, const void*> EV;
    struct foo { foo(int v) : bar(v) {} int bar; };
    typedef expected<foo, const char*> F;
    {
        auto e = E(42);
        ASSERT_EQ(e.has_value(), true);
        ASSERT_EQ(e.value(), 42);
        ASSERT_EQ(e.value_or(3.14), 42);
        const auto e2(e);
        ASSERT_EQ(e2.has_value(), true);
        ASSERT_EQ(e2.value(), 42);
        ASSERT_EQ(e2.value_or(3.14), 42);
        E e3;
        e3 = e2;
        ASSERT_EQ(e3.has_value(), true);
        ASSERT_EQ(e3.value(), 42);
        ASSERT_EQ(e3.value_or(3.14), 42);
        const E e4 = e2;
        ASSERT_EQ(e4.has_value(), true);
        ASSERT_EQ(e4.value(), 42);
        ASSERT_EQ(e4.value_or(3.14), 42);
    }
    {
        constexpr E c(42);
        ASSERT_EQ(c.has_value(), true);
        ASSERT_EQ(c.value(), 42);
        ASSERT_EQ(c.value_or(3.14), 42);
        constexpr const auto c2(c);
        ASSERT_EQ(c2.has_value(), true);
        ASSERT_EQ(c2.value(), 42);
        ASSERT_EQ(c2.value_or(3.14), 42);
    }
    {
        auto u = E(make_unexpected(oops));
        ASSERT_EQ(u.has_value(), false);
        ASSERT_EQ(u.error(), oops);
        ASSERT_EQ(u.get_unexpected().value(), oops);
        ASSERT_EQ(u.value_or(3.14), 3);
    }
    {
        auto uv = EV(make_unexpected(oops));
        ASSERT_EQ(uv.has_value(), false);
        ASSERT_EQ(uv.error(), oops);
        ASSERT_EQ(uv.get_unexpected().value(), oops);
        ASSERT_EQ(uv.value_or(3.14), 3);
    }
    {
        E e = make_unexpected(oops);
        ASSERT_EQ(e.has_value(), false);
        ASSERT_EQ(e.error(), oops);
        ASSERT_EQ(e.get_unexpected().value(), oops);
        ASSERT_EQ(e.value_or(3.14), 3);
    }
    {
        auto e = F(42);
        ASSERT_EQ(e->bar, 42);
        ASSERT_EQ((*e).bar, 42);
    }
    /*{
      constexpr F c(foo(42));
      ASSERT_EQ(c->bar, 42);
      ASSERT_EQ((*c).bar, 42);
      }*/
    // FIXME also test non-trivial value, non-trivial error.
}

int main() {
    test_unexpected_type();
    test_expected();

    return errors != 0;
}
