#include "Expected.h"
#include <cstdio>
#include <string>
#include <unordered_map>

constexpr const char* oops = "oops";
constexpr const char* foof = "foof";
unsigned errors = 0;

#define ASSERT(A, B, CMP) do {                                  \
    if (!((A) CMP (B))) {                                       \
      fprintf(stderr, "Failed: !(%s %s %s)\n", #A, #CMP, #B);   \
      ++errors;                                                 \
    }                                                           \
  } while (0)

#define ASSERT_NOT(A, B, CMP) do {                              \
    if ((A) CMP (B)) {                                          \
      fprintf(stderr, "Failed: %s %s %s\n", #A, #CMP, #B);      \
      ++errors;                                                 \
    }                                                           \
  } while (0)

#define ASSERT_EQ(A, B) ASSERT(A, B, ==)
#define ASSERT_NE(A, B) ASSERT(A, B, !=)
#define ASSERT_LT(A, B) ASSERT(A, B, <)
#define ASSERT_GT(A, B) ASSERT(A, B, >)
#define ASSERT_LE(A, B) ASSERT(A, B, <=)
#define ASSERT_GE(A, B) ASSERT(A, B, >=)
#define ASSERT_NOT_EQ(A, B) ASSERT_NOT(A, B, ==)
#define ASSERT_NOT_NE(A, B) ASSERT_NOT(A, B, !=)
#define ASSERT_NOT_LT(A, B) ASSERT_NOT(A, B, <)
#define ASSERT_NOT_GT(A, B) ASSERT_NOT(A, B, >)
#define ASSERT_NOT_LE(A, B) ASSERT_NOT(A, B, <=)
#define ASSERT_NOT_GE(A, B) ASSERT_NOT(A, B, >=)

void test_unexpected_type()
{
    {
        auto u = unexpected_type<int>(42);
        ASSERT_EQ(u.value(), 42);
        constexpr auto c = make_unexpected(42);
        ASSERT_EQ(c.value(), 42);
        ASSERT_EQ(u, c);
        ASSERT_NOT_NE(u, c);
        ASSERT_NOT_LT(u, c);
        ASSERT_NOT_GT(u, c);
        ASSERT_LE(u, c);
        ASSERT_GE(u, c);
    }
    {
        auto c = make_unexpected(oops);
        ASSERT_EQ(c.value(), oops);
    }
    {
        auto s = make_unexpected(std::string(oops));
        ASSERT_EQ(s.value(), oops);
    }
    {
        constexpr auto s0 = make_unexpected(oops);
        constexpr auto s1(s0);
        ASSERT_EQ(s0, s1);
    }
}

void test_expected()
{
    typedef expected<int, const char*> E;
    typedef expected<int, const void*> EV;
    struct foo {
      int v;
      foo(int v) : v(v) { }
      ~foo() { }
      bool operator==(const foo& y) const { return v == y.v; }
    };
    typedef expected<foo, const char*> FooChar;
    typedef expected<foo, std::string> FooString;
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
        auto e = make_expected_from_error<int, const char*>(oops);
        ASSERT_EQ(e.has_value(), false);
        ASSERT_EQ(e.error(), oops);
        ASSERT_EQ(e.get_unexpected().value(), oops);
        ASSERT_EQ(e.value_or(3.14), 3);
    }
    {
        auto e = make_expected_from_error<int, const void*>(oops);
        ASSERT_EQ(e.has_value(), false);
        ASSERT_EQ(e.error(), oops);
        ASSERT_EQ(e.get_unexpected().value(), oops);
        ASSERT_EQ(e.value_or(3.14), 3);
    }
    {
        auto e = FooChar(42);
        ASSERT_EQ(e->v, 42);
        ASSERT_EQ((*e).v, 42);
    }
    {
        auto e0 = E(42);
        auto e1 = E(1024);
        swap(e0, e1);
        ASSERT_EQ(e0.value(), 1024);
        ASSERT_EQ(e1.value(), 42);
    }
    {
        auto e0 = E(make_unexpected(oops));
        auto e1 = E(make_unexpected(foof));
        swap(e0, e1);
        ASSERT_EQ(e0.error(), foof);
        ASSERT_EQ(e1.error(), oops);
    }
    {
        FooChar c(foo(42));
        ASSERT_EQ(c->v, 42);
        ASSERT_EQ((*c).v, 42);
    }
    {
        FooString s(foo(42));
        ASSERT_EQ(s->v, 42);
        ASSERT_EQ((*s).v, 42);
        const char* message = "very long failure string, for very bad failure cases";
        FooString e0(make_unexpected<std::string>(message));
        FooString e1(make_unexpected<std::string>(message));
        FooString e2(make_unexpected<std::string>(std::string()));
        ASSERT_EQ(e0.error(), std::string(message));
        ASSERT_EQ(e0, e1);
        ASSERT_NE(e0, e2);
        FooString* e4 = new FooString(make_unexpected<std::string>(message));
        FooString* e5 = new FooString(*e4);
        ASSERT_EQ(e0, *e4);
        delete e4;
        ASSERT_EQ(e0, *e5);
        delete e5;
    }
}

void test_comparisons()
{
    typedef expected<int, const char*> Ex;
    typedef expected<int, int> Er;

    // Two expected, no errors.
    ASSERT_EQ(Ex(42), Ex(42));
    ASSERT_NE(Ex(42), Ex(1024));
    ASSERT_LT(Ex(42), Ex(1024));
    ASSERT_GT(Ex(1024), Ex(42));
    ASSERT_LE(Ex(42), Ex(42));
    ASSERT_GE(Ex(42), Ex(42));
    ASSERT_LE(Ex(42), Ex(1024));
    ASSERT_GE(Ex(1024), Ex(42));

    ASSERT_NOT_EQ(Ex(42), Ex(1024));
    ASSERT_NOT_NE(Ex(42), Ex(42));
    ASSERT_NOT_LT(Ex(1024), Ex(42));
    ASSERT_NOT_GT(Ex(42), Ex(1024));
    ASSERT_NOT_LE(Ex(1024), Ex(42));
    ASSERT_NOT_GE(Ex(42), Ex(1024));

    // Two expected, half errors.
    ASSERT_NOT_EQ(Ex(42), Ex(make_unexpected(oops)));
    ASSERT_NE(Ex(42), Ex(make_unexpected(oops)));
    ASSERT_LT(Ex(42), Ex(make_unexpected(oops)));
    ASSERT_NOT_GT(Ex(42), Ex(make_unexpected(oops)));
    ASSERT_LE(Ex(42), Ex(make_unexpected(oops)));
    ASSERT_NOT_GE(Ex(42), Ex(make_unexpected(oops)));

    ASSERT_NOT_EQ(Ex(make_unexpected(oops)), Ex(42));
    ASSERT_NE(Ex(make_unexpected(oops)), Ex(42));
    ASSERT_NOT_LT(Ex(make_unexpected(oops)), Ex(42));
    ASSERT_GT(Ex(make_unexpected(oops)), Ex(42));
    ASSERT_NOT_LE(Ex(make_unexpected(oops)), Ex(42));
    ASSERT_GE(Ex(make_unexpected(oops)), Ex(42));

    // Two expected, all errors.
    ASSERT_EQ(Er(42), Er(42));
    ASSERT_NE(Er(42), Er(1024));
    ASSERT_LT(Er(42), Er(1024));
    ASSERT_GT(Er(1024), Er(42));
    ASSERT_LE(Er(42), Er(42));
    ASSERT_GE(Er(42), Er(42));
    ASSERT_LE(Er(42), Er(1024));
    ASSERT_GE(Er(1024), Er(42));

    ASSERT_NOT_EQ(Er(42), Er(1024));
    ASSERT_NOT_NE(Er(42), Er(42));
    ASSERT_NOT_LT(Er(1024), Er(42));
    ASSERT_NOT_GT(Er(42), Er(1024));
    ASSERT_NOT_LE(Er(1024), Er(42));
    ASSERT_NOT_GE(Er(42), Er(1024));

    // One expected, one value.
    ASSERT_EQ(Ex(42), 42);
    ASSERT_NE(Ex(42), 0);
    ASSERT_LT(Ex(42), 1024);
    ASSERT_GT(Ex(1024), 42);
    ASSERT_LE(Ex(42), 42);
    ASSERT_GE(Ex(42), 42);
    ASSERT_LE(Ex(42), 1024);
    ASSERT_GE(Ex(1024), 42);

    ASSERT_NOT_EQ(Ex(42), 0);
    ASSERT_NOT_NE(Ex(42), 42);
    ASSERT_NOT_LT(Ex(1024), 42);
    ASSERT_NOT_GT(Ex(42), 1024);
    ASSERT_NOT_LE(Ex(1024), 42);
    ASSERT_NOT_GE(Ex(42), 1024);

    ASSERT_EQ(42, Ex(42));
    ASSERT_NE(42, Ex(1024));
    ASSERT_LT(42, Ex(1024));
    ASSERT_GT(1024, Ex(42));
    ASSERT_LE(42, Ex(42));
    ASSERT_GE(42, Ex(42));
    ASSERT_LE(42, Ex(1024));
    ASSERT_GE(1024, Ex(42));

    ASSERT_NOT_EQ(42, Ex(1024));
    ASSERT_NOT_NE(42, Ex(42));
    ASSERT_NOT_LT(1024, Ex(42));
    ASSERT_NOT_GT(42, Ex(1024));
    ASSERT_NOT_LE(1024, Ex(42));
    ASSERT_NOT_GE(42, Ex(1024));

    // One expected, one unexpected.
    ASSERT_NOT_EQ(Ex(42), make_unexpected(oops));
    ASSERT_NE(Ex(42), make_unexpected(oops));
    ASSERT_LT(Ex(42), make_unexpected(oops));
    ASSERT_NOT_GT(Ex(42), make_unexpected(oops));
    ASSERT_LE(Ex(42), make_unexpected(oops));
    ASSERT_NOT_GE(Ex(42), make_unexpected(oops));

    ASSERT_NOT_EQ(make_unexpected(oops), Ex(42));
    ASSERT_NE(make_unexpected(oops), Ex(42));
    ASSERT_NOT_LT(make_unexpected(oops), Ex(42));
    ASSERT_GT(make_unexpected(oops), Ex(42));
    ASSERT_NOT_LE(make_unexpected(oops), Ex(42));
    ASSERT_GE(make_unexpected(oops), Ex(42));
}

void test_hash()
{
    typedef expected<int, const char*> E;
    std::unordered_map<E, int> m;
    m.insert({ E(42), 42 });
    m.insert({ E(make_unexpected(oops)), 5 });
    m.insert({ E(1024), 1024 });
    m.insert({ E(make_unexpected(foof)), 0xf00f });
    ASSERT_EQ(m[E(42)], 42);
    ASSERT_EQ(m[E(1024)], 1024);
    ASSERT_EQ(m[E(make_unexpected(oops))], 5);
    ASSERT_EQ(m[E(make_unexpected(foof))], 0xf00f);
}

int main()
{
    test_unexpected_type();
    test_expected();
    test_comparisons();
    test_hash();

    return errors != 0;
}
