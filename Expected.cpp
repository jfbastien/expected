/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include <wtf/Expected.h>

#include <cstdio>
#include <string>
#include <unordered_map>

namespace TestWebKitAPI {

constexpr const char* oops = "oops";
constexpr const char* foof = "foof";

TEST(WTF_Expected, unexpected_type)
{
    {
        auto u = unexpected_type<int>(42);
        EXPECT_EQ(u.value(), 42);
        constexpr auto c = make_unexpected(42);
        EXPECT_EQ(c.value(), 42);
        EXPECT_TRUE(u == c);
        EXPECT_FALSE(u != c);
        EXPECT_FALSE(u < c);
        EXPECT_FALSE(u > c);
        EXPECT_TRUE(u <= c);
        EXPECT_TRUE(u >= c);
    }
    {
        auto c = make_unexpected(oops);
        EXPECT_EQ(c.value(), oops);
    }
    {
        auto s = make_unexpected(std::string(oops));
        EXPECT_EQ(s.value(), oops);
    }
    {
        constexpr auto s0 = make_unexpected(oops);
        constexpr auto s1(s0);
        EXPECT_TRUE(s0 == s1);
    }
}

TEST(WTF_Expected, expected)
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
        auto e = E();
        EXPECT_TRUE(e.has_value());
        EXPECT_EQ(e.value(), 0);
        EXPECT_EQ(e.value_or(3.14), 0);
    }
    {
        constexpr E e;
        EXPECT_TRUE(e.has_value());
        EXPECT_EQ(e.value(), 0);
        EXPECT_EQ(e.value_or(3.14), 0);
    }
    {
        auto e = E(42);
        EXPECT_TRUE(e.has_value());
        EXPECT_EQ(e.value(), 42);
        EXPECT_EQ(e.value_or(3.14), 42);
        const auto e2(e);
        EXPECT_TRUE(e2.has_value());
        EXPECT_EQ(e2.value(), 42);
        EXPECT_EQ(e2.value_or(3.14), 42);
        E e3;
        e3 = e2;
        EXPECT_TRUE(e3.has_value());
        EXPECT_EQ(e3.value(), 42);
        EXPECT_EQ(e3.value_or(3.14), 42);
        const E e4 = e2;
        EXPECT_TRUE(e4.has_value());
        EXPECT_EQ(e4.value(), 42);
        EXPECT_EQ(e4.value_or(3.14), 42);
    }
    {
        constexpr E c(42);
        EXPECT_TRUE(c.has_value());
        EXPECT_EQ(c.value(), 42);
        EXPECT_EQ(c.value_or(3.14), 42);
        constexpr const auto c2(c);
        EXPECT_TRUE(c2.has_value());
        EXPECT_EQ(c2.value(), 42);
        EXPECT_EQ(c2.value_or(3.14), 42);
    }
    {
        auto u = E(make_unexpected(oops));
        EXPECT_FALSE(u.has_value());
        EXPECT_EQ(u.error(), oops);
        EXPECT_EQ(u.get_unexpected().value(), oops);
        EXPECT_EQ(u.value_or(3.14), 3);
    }
    {
        auto uv = EV(make_unexpected(oops));
        EXPECT_FALSE(uv.has_value());
        EXPECT_EQ(uv.error(), oops);
        EXPECT_EQ(uv.get_unexpected().value(), oops);
        EXPECT_EQ(uv.value_or(3.14), 3);
    }
    {
        E e = make_unexpected(oops);
        EXPECT_FALSE(e.has_value());
        EXPECT_EQ(e.error(), oops);
        EXPECT_EQ(e.get_unexpected().value(), oops);
        EXPECT_EQ(e.value_or(3.14), 3);
    }
    {
        auto e = make_expected_from_error<int, const char*>(oops);
        EXPECT_FALSE(e.has_value());
        EXPECT_EQ(e.error(), oops);
        EXPECT_EQ(e.get_unexpected().value(), oops);
        EXPECT_EQ(e.value_or(3.14), 3);
    }
    {
        auto e = make_expected_from_error<int, const void*>(oops);
        EXPECT_FALSE(e.has_value());
        EXPECT_EQ(e.error(), oops);
        EXPECT_EQ(e.get_unexpected().value(), oops);
        EXPECT_EQ(e.value_or(3.14), 3);
    }
    {
        auto e = FooChar(42);
        EXPECT_EQ(e->v, 42);
        EXPECT_EQ((*e).v, 42);
    }
    {
        auto e0 = E(42);
        auto e1 = E(1024);
        swap(e0, e1);
        EXPECT_EQ(e0.value(), 1024);
        EXPECT_EQ(e1.value(), 42);
    }
    {
        auto e0 = E(make_unexpected(oops));
        auto e1 = E(make_unexpected(foof));
        swap(e0, e1);
        EXPECT_EQ(e0.error(), foof);
        EXPECT_EQ(e1.error(), oops);
    }
    {
        FooChar c(foo(42));
        EXPECT_EQ(c->v, 42);
        EXPECT_EQ((*c).v, 42);
    }
    {
        FooString s(foo(42));
        EXPECT_EQ(s->v, 42);
        EXPECT_EQ((*s).v, 42);
        const char* message = "very long failure string, for very bad failure cases";
        FooString e0(make_unexpected<std::string>(message));
        FooString e1(make_unexpected<std::string>(message));
        FooString e2(make_unexpected<std::string>(std::string()));
        EXPECT_EQ(e0.error(), std::string(message));
        EXPECT_TRUE(e0 == e1);
        EXPECT_TRUE(e0 != e2);
        FooString* e4 = new FooString(make_unexpected<std::string>(message));
        FooString* e5 = new FooString(*e4);
        EXPECT_TRUE(e0 == *e4);
        delete e4;
        EXPECT_TRUE(e0 == *e5);
        delete e5;
    }
}

TEST(WTF_Expected, expected_void)
{
    typedef expected<void, const char*> E;
    typedef expected<void, const void*> EV;
    typedef expected<void, std::string> String;
    {
        auto e = E();
        EXPECT_TRUE(e.has_value());
        const auto e2(e);
        EXPECT_TRUE(e2.has_value());
        EXPECT_TRUE(e == e2);
        E e3;
        e3 = e2;
        EXPECT_TRUE(e3.has_value());
        EXPECT_TRUE(e == e3);
    }
    {
        constexpr E c;
        EXPECT_TRUE(c.has_value());
        constexpr const auto c2(c);
        EXPECT_TRUE(c2.has_value());
        EXPECT_TRUE(c == c2);
    }
    {
        auto u = E(make_unexpected(oops));
        EXPECT_FALSE(u.has_value());
        EXPECT_EQ(u.error(), oops);
        EXPECT_EQ(u.get_unexpected().value(), oops);
    }
    {
        auto uv = EV(make_unexpected(oops));
        EXPECT_FALSE(uv.has_value());
        EXPECT_EQ(uv.error(), oops);
        EXPECT_EQ(uv.get_unexpected().value(), oops);
    }
    {
        E e = make_unexpected(oops);
        EXPECT_FALSE(e.has_value());
        EXPECT_EQ(e.error(), oops);
        EXPECT_EQ(e.get_unexpected().value(), oops);
    }
    {
        auto e = make_expected_from_error<void, const char*>(oops);
        EXPECT_FALSE(e.has_value());
        EXPECT_EQ(e.error(), oops);
        EXPECT_EQ(e.get_unexpected().value(), oops);
    }
    {
        auto e = make_expected_from_error<void, const void*>(oops);
        EXPECT_FALSE(e.has_value());
        EXPECT_EQ(e.error(), oops);
        EXPECT_EQ(e.get_unexpected().value(), oops);
    }
    {
        auto e0 = E();
        auto e1 = E();
        swap(e0, e1);
        EXPECT_TRUE(e0 == e1);
    }
    {
        auto e0 = E(make_unexpected(oops));
        auto e1 = E(make_unexpected(foof));
        swap(e0, e1);
        EXPECT_EQ(e0.error(), foof);
        EXPECT_EQ(e1.error(), oops);
    }
    {
        const char* message = "very long failure string, for very bad failure cases";
        String e0(make_unexpected<std::string>(message));
        String e1(make_unexpected<std::string>(message));
        String e2(make_unexpected<std::string>(std::string()));
        EXPECT_EQ(e0.error(), std::string(message));
        EXPECT_TRUE(e0 == e1);
        EXPECT_TRUE(e0 != e2);
        String* e4 = new String(make_unexpected<std::string>(message));
        String* e5 = new String(*e4);
        EXPECT_TRUE(e0 == *e4);
        delete e4;
        EXPECT_TRUE(e0 == *e5);
        delete e5;
    }
}

TEST(WTF_Expected, comparison)
{
    typedef expected<int, const char*> Ex;
    typedef expected<int, int> Er;

    // Two expected, no errors.
    EXPECT_TRUE(Ex(42) == Ex(42));
    EXPECT_TRUE(Ex(42) != Ex(1024));
    EXPECT_TRUE(Ex(42) < Ex(1024));
    EXPECT_TRUE(Ex(1024) > Ex(42));
    EXPECT_TRUE(Ex(42) <= Ex(42));
    EXPECT_TRUE(Ex(42) >= Ex(42));
    EXPECT_TRUE(Ex(42) <= Ex(1024));
    EXPECT_TRUE(Ex(1024) >= Ex(42));

    EXPECT_FALSE(Ex(42) == Ex(1024));
    EXPECT_FALSE(Ex(42) != Ex(42));
    EXPECT_FALSE(Ex(1024) < Ex(42));
    EXPECT_FALSE(Ex(42) > Ex(1024));
    EXPECT_FALSE(Ex(1024) < Ex(42));
    EXPECT_FALSE(Ex(42) >= Ex(1024));

    // Two expected, half errors.
    EXPECT_FALSE(Ex(42) == Ex(make_unexpected(oops)));
    EXPECT_TRUE(Ex(42) != Ex(make_unexpected(oops)));
    EXPECT_TRUE(Ex(42) < Ex(make_unexpected(oops)));
    EXPECT_FALSE(Ex(42) > Ex(make_unexpected(oops)));
    EXPECT_TRUE(Ex(42) <= Ex(make_unexpected(oops)));
    EXPECT_FALSE(Ex(42) >= Ex(make_unexpected(oops)));

    EXPECT_FALSE(Ex(make_unexpected(oops)) == Ex(42));
    EXPECT_TRUE(Ex(make_unexpected(oops)) != Ex(42));
    EXPECT_FALSE(Ex(make_unexpected(oops)) < Ex(42));
    EXPECT_TRUE(Ex(make_unexpected(oops)) > Ex(42));
    EXPECT_FALSE(Ex(make_unexpected(oops)) <= Ex(42));
    EXPECT_TRUE(Ex(make_unexpected(oops)) >= Ex(42));

    // Two expected, all errors.
    EXPECT_TRUE(Er(42) == Er(42));
    EXPECT_TRUE(Er(42) != Er(1024));
    EXPECT_TRUE(Er(42) < Er(1024));
    EXPECT_TRUE(Er(1024) > Er(42));
    EXPECT_TRUE(Er(42) <= Er(42));
    EXPECT_TRUE(Er(42) >= Er(42));
    EXPECT_TRUE(Er(42) <= Er(1024));
    EXPECT_TRUE(Er(1024) >= Er(42));

    EXPECT_FALSE(Er(42) == Er(1024));
    EXPECT_FALSE(Er(42) != Er(42));
    EXPECT_FALSE(Er(1024) < Er(42));
    EXPECT_FALSE(Er(42) > Er(1024));
    EXPECT_FALSE(Er(1024) <= Er(42));
    EXPECT_FALSE(Er(42) >= Er(1024));

    // One expected, one value.
    EXPECT_TRUE(Ex(42) == 42);
    EXPECT_TRUE(Ex(42) != 0);
    EXPECT_TRUE(Ex(42) < 1024);
    EXPECT_TRUE(Ex(1024) > 42);
    EXPECT_TRUE(Ex(42) <= 42);
    EXPECT_TRUE(Ex(42) >= 42);
    EXPECT_TRUE(Ex(42) <= 1024);
    EXPECT_TRUE(Ex(1024) >= 42);

    EXPECT_FALSE(Ex(42) == 0);
    EXPECT_FALSE(Ex(42) != 42);
    EXPECT_FALSE(Ex(1024) < 42);
    EXPECT_FALSE(Ex(42) > 1024);
    EXPECT_FALSE(Ex(1024) < 42);
    EXPECT_FALSE(Ex(42) >= 1024);

    EXPECT_TRUE(42 == Ex(42));
    EXPECT_TRUE(42 != Ex(1024));
    EXPECT_TRUE(42 < Ex(1024));
    EXPECT_TRUE(1024 > Ex(42));
    EXPECT_TRUE(42 <= Ex(42));
    EXPECT_TRUE(42 >= Ex(42));
    EXPECT_TRUE(42 <= Ex(1024));
    EXPECT_TRUE(1024 >= Ex(42));

    EXPECT_FALSE(42 == Ex(1024));
    EXPECT_FALSE(42 != Ex(42));
    EXPECT_FALSE(1024 < Ex(42));
    EXPECT_FALSE(42 > Ex(1024));
    EXPECT_FALSE(1024 <= Ex(42));
    EXPECT_FALSE(42 >= Ex(1024));

    // One expected, one unexpected.
    EXPECT_FALSE(Ex(42) == make_unexpected(oops));
    EXPECT_TRUE(Ex(42) != make_unexpected(oops));
    EXPECT_TRUE(Ex(42) < make_unexpected(oops));
    EXPECT_FALSE(Ex(42) > make_unexpected(oops));
    EXPECT_TRUE(Ex(42) <= make_unexpected(oops));
    EXPECT_FALSE(Ex(42) >= make_unexpected(oops));

    EXPECT_FALSE(make_unexpected(oops) == Ex(42));
    EXPECT_TRUE(make_unexpected(oops) != Ex(42));
    EXPECT_FALSE(make_unexpected(oops) < Ex(42));
    EXPECT_TRUE(make_unexpected(oops) > Ex(42));
    EXPECT_FALSE(make_unexpected(oops) <= Ex(42));
    EXPECT_TRUE(make_unexpected(oops) >= Ex(42));
}

TEST(WTF_Expected, hash)
{
    typedef expected<int, const char*> E;
    std::unordered_map<E, int> m;
    m.insert({ E(42), 42 });
    m.insert({ E(make_unexpected(oops)), 5 });
    m.insert({ E(1024), 1024 });
    m.insert({ E(make_unexpected(foof)), 0xf00f });
    EXPECT_EQ(m[E(42)], 42);
    EXPECT_EQ(m[E(1024)], 1024);
    EXPECT_EQ(m[E(make_unexpected(oops))], 5);
    EXPECT_EQ(m[E(make_unexpected(foof))], 0xf00f);
}

TEST(WTF_Expected, hash_void)
{
    typedef expected<void, const char*> E;
    std::unordered_map<E, int> m;
    m.insert({ E(), 42 });
    m.insert({ E(make_unexpected(oops)), 5 });
    m.insert({ E(make_unexpected(foof)), 0xf00f });
    EXPECT_EQ(m[E()], 42);
    EXPECT_EQ(m[E(make_unexpected(oops))], 5);
    EXPECT_EQ(m[E(make_unexpected(foof))], 0xf00f);
}

} // namespace TestWebkitAPI
