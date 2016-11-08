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

// WTF::Expected is based on std::expected, as described here: http://wg21.link/p0323r1

#ifndef Expected_h
#define Expected_h

#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace WTF {

// The specification expects to throw. This implementation doesn't support exceptions.
void unexpected_fail() { abort(); }

// Part of <optional>, used in <expected>.
struct nullopt_t {
    constexpr nullopt_t(int) { }
};
constexpr nullopt_t nullopt{ 42 };


template <class E>
class unexpected_type {
public:
    unexpected_type() = delete;
    constexpr explicit unexpected_type(const E& e) : val(e) { }
    constexpr explicit unexpected_type(E&& e) : val(std::move(e)) { }
    constexpr const E& value() const { return val; }
    constexpr E& value() { return val; }

private:
    E val;
};

template <class E> constexpr bool operator==(const unexpected_type<E>& lhs, const unexpected_type<E>& rhs) { return lhs.value() == rhs.value(); }
template <class E> constexpr bool operator!=(const unexpected_type<E>& lhs, const unexpected_type<E>& rhs) { return lhs.value() != rhs.value(); }
template <class E> constexpr bool operator<(const unexpected_type<E>& lhs, const unexpected_type<E>& rhs) { return lhs.value() < rhs.value(); }
template <class E> constexpr bool operator>(const unexpected_type<E>& lhs, const unexpected_type<E>& rhs) { return lhs.value() > rhs.value(); }
template <class E> constexpr bool operator<=(const unexpected_type<E>& lhs, const unexpected_type<E>& rhs) { return lhs.value() <= rhs.value(); }
template <class E> constexpr bool operator>=(const unexpected_type<E>& lhs, const unexpected_type<E>& rhs) { return lhs.value() >= rhs.value(); }

template <class E> constexpr unexpected_type<std::decay_t<E>> make_unexpected(E&& v) { return unexpected_type<typename std::decay<E>::type>(std::forward<E>(v)); }

struct unexpect_t { unexpect_t() = delete; };
constexpr unexpect_t unexpect { };

template <class T, class E>
class expected {
public:
    typedef T value_type;
    typedef E error_type;
    template <class U> struct rebind { using type = expected<U, error_type>; };

    constexpr expected() : val(), has(true) { }
    expected(const expected&) = default;
    expected(expected&&) = default;
    constexpr expected(const value_type& e) : val(e), has(true) { }
    constexpr expected(value_type&& e) : val(std::move(e)), has(true) { }
    //template <class... Args> constexpr explicit expected(in_place_t, Args&&...);
    //template <class U, class... Args> constexpr explicit expected(in_place_t, std::initializer_list<U>, Args&&...);
    constexpr expected(unexpected_type<E> const& u) : err(u.value()), has(false) { }
    template <class Err> constexpr expected(unexpected_type<Err> const& u) : err(u.value()), has(false) { }
    //template <class... Args> constexpr explicit expected(unexpect_t, Args&&...);
    //template <class U, class... Args> constexpr explicit expected(unexpect_t, std::initializer_list<U>, Args&&...);

    ~expected() = default;

    expected& operator=(const expected& e) { type(e).swap(*this); return *this; }
    expected& operator=(expected&& e) { type(std::move(e)).swap(*this); return *this; }
    template <class U> expected& operator=(U&& u) { type(std::move(u)).swap(*this); return *this; }
    expected& operator=(const unexpected_type<E>& u) { type(u).swap(*this); return *this; }
    expected& operator=(unexpected_type<E>&& u) { type(std::move(u)).swap(*this); return *this; }
    //template <class... Args> void emplace(Args&&...);
    //template <class U, class... Args> void emplace(std::initializer_list<U>, Args&&...);

    void swap(expected& o) {
      using std::swap;
      if (has && o.has) {
        swap(val, o.val);
      } else if (has && !o.has) {
        error_type e(std::move(o.err));
        new (&o.val) value_type(std::move(val));
        new (&err) error_type(e);
        swap(has, o.has);
      } else if (!has && o.has) {
        value_type v(std::move(o.val));
        new (&o.err) error_type(std::move(err));
        new (&val) value_type(v);
        swap(has, o.has);
      } else {
        swap(err, o.err);
      }
    }

    constexpr const T* operator->() const { return &val; }
    T* operator->() { return &val; }
    constexpr const T& operator*() const & { return val; }
    T& operator*() & { return val; }
    constexpr const T&& operator*() const && { return std::move(val); }
    constexpr T&& operator*() && { return std::move(val); }
    constexpr explicit operator bool() const { return has; }
    constexpr bool has_value() const { return has; }
    constexpr const T& value() const & { return has ? val : (unexpected_fail(), val); }
    constexpr T& value() & { return has ? val : (unexpected_fail(), val); }
    constexpr const T&& value() const && { return has ? val : (unexpected_fail(), val); }
    constexpr T&& value() && { return has ? val : (unexpected_fail(), val); }
    constexpr const E& error() const & { return !has ? err : (unexpected_fail(), err); }
    E& error() & { return !has ? err : (unexpected_fail(), err); }
    constexpr E&& error() && { return !has ? err : (unexpected_fail(), err); }
    constexpr const E&& error() const && { return !has ? err : (unexpected_fail(), err); }
    constexpr unexpected_type<E> get_unexpected() const { return unexpected_type<E>(err); }
    template <class U> constexpr T value_or(U&& u) const & { return has ? **this : static_cast<value_type>(std::forward<U>(u)); }
    template <class U> T value_or(U&& u) && { return has ? std::move(**this) : static_cast<value_type>(std::forward<U>(u)); }

private:
    typedef expected<value_type, error_type> type;
    union {
        value_type val;
        error_type err;
    };
    bool has;
};

template <class T, class E> constexpr bool operator==(const expected<T, E>& x, const expected<T, E>& y) { return bool(x) == bool(y) && (x ? x.value() == y.value() : x.error() == y.error()); }
template <class T, class E> constexpr bool operator!=(const expected<T, E>& x, const expected<T, E>& y) { return !(x == y); }
template <class T, class E> constexpr bool operator<(const expected<T, E>& x, const expected<T, E>& y) { return (!bool(x) && bool(y)) ? false : ((bool(x) && !bool(y)) ? true : ((bool(x) && bool(y)) ? x.value() < y.value() : x.error() < y.error())); }
template <class T, class E> constexpr bool operator>(const expected<T, E>& x, const expected<T, E>& y) { return !(x == y) && !(x < y); }
template <class T, class E> constexpr bool operator<=(const expected<T, E>& x, const expected<T, E>& y) { return (x == y) || (x < y); }
template <class T, class E> constexpr bool operator>=(const expected<T, E>& x, const expected<T, E>& y) { return (x == y) || (x > y); }

template <class T, class E> constexpr bool operator==(const expected<T, E>& x, const T& y) { return x == expected<T, E>(y); }
template <class T, class E> constexpr bool operator==(const T& x, const expected<T, E>& y) { return expected<T, E>(x) == y; }
template <class T, class E> constexpr bool operator!=(const expected<T, E>& x, const T& y) { return x != expected<T, E>(y); }
template <class T, class E> constexpr bool operator!=(const T& x, const expected<T, E>& y) { return expected<T, E>(x) != y; }
template <class T, class E> constexpr bool operator<(const expected<T, E>& x, const T& y) { return x < expected<T, E>(y); }
template <class T, class E> constexpr bool operator<(const T& x, const expected<T, E>& y) { return expected<T, E>(x) < y; }
template <class T, class E> constexpr bool operator<=(const expected<T, E>& x, const T& y) { return x <= expected<T, E>(y); }
template <class T, class E> constexpr bool operator<=(const T& x, const expected<T, E>& y) { return expected<T, E>(x) <= y; }
template <class T, class E> constexpr bool operator>(const expected<T, E>& x, const T& y) { return x > expected<T, E>(y); }
template <class T, class E> constexpr bool operator>(const T& x, const expected<T, E>& y) { return expected<T, E>(x) > y; }
template <class T, class E> constexpr bool operator>=(const expected<T, E>& x, const T& y) { return x >= expected<T, E>(y); }
template <class T, class E> constexpr bool operator>=(const T& x, const expected<T, E>& y) { return expected<T, E>(x) >= y; }

template <class T, class E> constexpr bool operator==(const expected<T, E>& x, const unexpected_type<E>& y) { return x == expected<T, E>(y); }
template <class T, class E> constexpr bool operator==(const unexpected_type<E>& x, const expected<T, E>& y) { return expected<T, E>(x) == y; }
template <class T, class E> constexpr bool operator!=(const expected<T, E>& x, const unexpected_type<E>& y) { return x != expected<T, E>(y); }
template <class T, class E> constexpr bool operator!=(const unexpected_type<E>& x, const expected<T, E>& y) { return expected<T, E>(x) != y; }
template <class T, class E> constexpr bool operator<(const expected<T, E>& x, const unexpected_type<E>& y) { return x < expected<T, E>(y); }  
template <class T, class E> constexpr bool operator<(const unexpected_type<E>& x, const expected<T, E>& y) {  return expected<T, E>(x) < y; }  
template <class T, class E> constexpr bool operator<=(const expected<T, E>& x, const unexpected_type<E>& y) { return x <= expected<T, E>(y); }
template <class T, class E> constexpr bool operator<=(const unexpected_type<E>& x, const expected<T, E>& y) { return expected<T, E>(x) <= y; }
template <class T, class E> constexpr bool operator>(const expected<T, E>& x, const unexpected_type<E>& y) { return x > expected<T, E>(y); }  
template <class T, class E> constexpr bool operator>(const unexpected_type<E>& x, const expected<T, E>& y) { return expected<T, E>(x) > y; }  
template <class T, class E> constexpr bool operator>=(const expected<T, E>& x, const unexpected_type<E>& y) { return x >= expected<T, E>(y); }
template <class T, class E> constexpr bool operator>=(const unexpected_type<E>& x, const expected<T, E>& y) { return expected<T, E>(x) >= y; }

template <typename T, typename E> void swap(expected<T, E>& x, expected<T, E>& y) { x.swap(y); }

template <class T, class E = WTF::nullopt_t> constexpr expected<std::decay_t<T>, E> make_expected(T&& v)
{
    return expected<typename std::decay<T>::type, E>(std::forward<T>(v));
}
template <class T, class E> constexpr expected<T, std::decay_t<E>> make_expected_from_error(E&& e) { return expected<T, std::decay_t<E>>(make_unexpected(e)); }
template <class T, class E, class U> constexpr expected<T, E> make_expected_from_error(U&& u) { return expected<T, E>(make_unexpected(E{std::forward<U>(u)})); }
//template <class F, class E = WTF::nullopt_t> constexpr expected<typename std::result_of<F>::type, E> make_expected_from_call(F f);

template <class E>
class expected<void, E> {
public:
    typedef void value_type;
    typedef E error_type;
    template <class U> struct rebind { typedef expected<U, error_type> type; };

    constexpr expected() : has(true) { }
    expected(const expected&) = default;
    expected(expected&&) = default;
    //constexpr explicit expected(in_place_t);
    constexpr expected(unexpected_type<E> const& u) : err(u.value()), has(false) { }
    template <class Err> constexpr expected(unexpected_type<Err> const& u) : err(u.value()), has(false) { }

    ~expected() = default;

    expected& operator=(const expected& e) { type(e).swap(*this); return *this; }
    expected& operator=(expected&& e) { type(std::move(e)).swap(*this); return *this; }
    expected& operator=(const unexpected_type<E>& u) { type(u).swap(*this); return *this; } // Not in the current paper.
    expected& operator=(unexpected_type<E>&& u) { type(std::move(u)).swap(*this); return *this; } // Not in the current paper.
    //void emplace();

    void swap(expected& o) {
      using std::swap;
      if (has && o.has) {
      } else if (has && !o.has) {
        error_type e(std::move(o.err));
        new (&err) error_type(e);
        swap(has, o.has);
      } else if (!has && o.has) {
        new (&o.err) error_type(std::move(err));
        swap(has, o.has);
      } else {
        swap(err, o.err);
      }
    }

    constexpr explicit operator bool() const { return has; }
    constexpr bool has_value() const { return has; }
    void value() const { if (!has) unexpected_fail(); }
    constexpr const E& error() const & { return !has ? err : (unexpected_fail(), err); }
    E& error() & { return !has ? err : (unexpected_fail(), err); } // Not in the current paper.
    constexpr E&& error() && { return !has ? err : (unexpected_fail(), err); }
    constexpr const E&& error() const && { return !has ? err : (unexpected_fail(), err); }  // Not in the current paper.
    //constexpr E& error() &;
    constexpr unexpected_type<E> get_unexpected() const { return unexpected_type<E>(err); }

private:
    typedef expected<value_type, error_type> type;
    union {
        unsigned char dummy;
        error_type err;
    };
    bool has;
};

expected<void, WTF::nullopt_t> make_expected() { return expected<void, WTF::nullopt_t>(); }

}

namespace std {

template <class T, class E> struct hash<WTF::expected<T, E>>
{
    typedef WTF::expected<T, E> argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& e) const { return e ? hash<typename argument_type::value_type>{}(e.value()) : hash<typename argument_type::error_type>{}(e.error()); }
};

template <class E> struct hash<WTF::expected<void, E>>
{
    typedef WTF::expected<void, E> argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& e) const { return e ? 0 : hash<typename argument_type::error_type>{}(e.error()); }
};

}

using WTF::nullopt;
using WTF::unexpected_type;
using WTF::make_unexpected;
using WTF::unexpect;
using WTF::expected;
using WTF::make_expected;
using WTF::make_expected_from_error;

#endif
