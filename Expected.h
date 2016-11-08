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

namespace {

static constexpr enum class expected_value_tag_type { } expected_value_tag{ };
static constexpr enum class expected_error_tag_type { } expected_error_tag{ };

template <class T, class E>
union expected_constexpr_storage {
    typedef T value_type;
    typedef E error_type;
    char dummy;
    value_type val;
    error_type err;
    constexpr expected_constexpr_storage() : dummy() { }
    constexpr expected_constexpr_storage(expected_value_tag_type) : val() { }
    constexpr expected_constexpr_storage(expected_error_tag_type) : err() { }
    constexpr expected_constexpr_storage(expected_value_tag_type, const value_type& v) : val(v) { }
    constexpr expected_constexpr_storage(expected_error_tag_type, const error_type& e) : err(e) { }
    ~expected_constexpr_storage() = default;
};

template <class T, class E>
union expected_storage {
    typedef T value_type;
    typedef E error_type;
    char dummy;
    value_type val;
    error_type err;
    constexpr expected_storage() : dummy() { }
    constexpr expected_storage(expected_value_tag_type) : val() { }
    constexpr expected_storage(expected_error_tag_type) : err() { }
    constexpr expected_storage(expected_value_tag_type, const value_type& val) : val(val) { }
    constexpr expected_storage(expected_error_tag_type, const error_type& err) : err(err) { }
    ~expected_storage() { }
};

template <class E>
union expected_constexpr_storage<void, E> {
    typedef void value_type;
    typedef E error_type;
    char dummy;
    error_type err;
    constexpr expected_constexpr_storage() : dummy() { }
    constexpr expected_constexpr_storage(expected_value_tag_type) : dummy() { }
    constexpr expected_constexpr_storage(expected_error_tag_type) : err() { }
    constexpr expected_constexpr_storage(expected_error_tag_type, const error_type& e) : err(e) { }
    ~expected_constexpr_storage() = default;
};

template <class E>
union expected_storage<void, E> {
    typedef void value_type;
    typedef E error_type;
    char dummy;
    error_type err;
    constexpr expected_storage() : dummy() { }
    constexpr expected_storage(expected_value_tag_type) : dummy() { }
    constexpr expected_storage(expected_error_tag_type) : err() { }
    constexpr expected_storage(expected_error_tag_type, const error_type& err) : err(err) { }
    ~expected_storage() { }
};

template <class T, class E>
struct expected_constexpr_base {
    typedef T value_type;
    typedef E error_type;
    expected_constexpr_storage<value_type, error_type> s;
    bool has;
    constexpr expected_constexpr_base() : s(), has(true) { }
    constexpr expected_constexpr_base(expected_value_tag_type tag) : s(tag), has(true) { }
    constexpr expected_constexpr_base(expected_error_tag_type tag) : s(tag), has(false) { }
    constexpr expected_constexpr_base(expected_value_tag_type tag, const value_type& val) : s(tag, val), has(true) { }
    constexpr expected_constexpr_base(expected_error_tag_type tag, const error_type& err) : s(tag, err), has(false) { }
    ~expected_constexpr_base() = default;
};

template <class T, class E>
struct expected_base {
    typedef T value_type;
    typedef E error_type;
    expected_storage<value_type, error_type> s;
    bool has;
    constexpr expected_base() : s(), has(true) { }
    constexpr expected_base(expected_value_tag_type tag) : s(tag), has(true) { }
    constexpr expected_base(expected_error_tag_type tag) : s(tag), has(false) { }
    constexpr expected_base(expected_value_tag_type tag, const value_type& val) : s(tag, val), has(true) { }
    constexpr expected_base(expected_error_tag_type tag, const error_type& err) : s(tag, err), has(false) { }
    expected_base(const expected_base& o)
    : has(o.has)
    {
        if (has)
            ::new (&s.val) value_type(o.s.val);
        else
            ::new (&s.err) error_type(o.s.err);
    }
    expected_base(const expected_base&& o)
    : has(o.has)
    {
        if (has)
            ::new (&s.val) value_type(std::move(o.s.val));
        else
            ::new (&s.err) error_type(std::move(o.s.err));
    }
    ~expected_base()
    {
        if (has)
            s.val.value_type::~value_type();
        else
            s.err.error_type::~error_type();
    }
};

template <class E>
struct expected_constexpr_base<void, E> {
    typedef void value_type;
    typedef E error_type;
    expected_constexpr_storage<value_type, error_type> s;
    bool has;
    constexpr expected_constexpr_base() : s(), has(true) { }
    constexpr expected_constexpr_base(expected_value_tag_type tag) : s(tag), has(true) { }
    constexpr expected_constexpr_base(expected_error_tag_type tag) : s(tag), has(false) { }
    constexpr expected_constexpr_base(expected_error_tag_type tag, const error_type& err) : s(tag, err), has(false) { }
    ~expected_constexpr_base() = default;
};

template <class E>
struct expected_base<void, E> {
    typedef void value_type;
    typedef E error_type;
    expected_storage<value_type, error_type> s;
    bool has;
    constexpr expected_base() : s(), has(true) { }
    constexpr expected_base(expected_value_tag_type tag) : s(tag), has(true) { }
    constexpr expected_base(expected_error_tag_type tag) : s(tag), has(false) { }
    constexpr expected_base(expected_error_tag_type tag, const error_type& err) : s(tag, err), has(false) { }
    expected_base(const expected_base& o)
    : has(o.has)
    {
        if (!has)
            ::new (&s.err) error_type(o.s.err);
    }
    expected_base(const expected_base&& o)
    : has(o.has)
    {
        if (!has)
            ::new (&s.err) error_type(std::move(o.s.err));
    }
    ~expected_base()
    {
        if (!has)
            s.err.error_type::~error_type();
    }
};

template <class T, class E>
using expected_base_select = typename std::conditional<
    ((std::is_void<T>::value || std::is_trivially_destructible<T>::value)
     && std::is_trivially_destructible<E>::value),
    expected_constexpr_base<typename std::remove_const<T>::type, typename std::remove_const<E>::type>,
    expected_base<typename std::remove_const<T>::type, typename std::remove_const<E>::type>
>::type;

 } // anonymous namespace

template <class T, class E>
class expected : private expected_base_select<T, E> {
    typedef expected_base_select<T, E> base;

public:
    typedef typename base::value_type value_type;
    typedef typename base::error_type error_type;

private:
    typedef expected<value_type, error_type> type;

public:
    template <class U> struct rebind { using type = expected<U, error_type>; };

    constexpr expected() : base(expected_value_tag) { }
    expected(const expected&) = default;
    expected(expected&&) = default;
    constexpr expected(const value_type& e) : base(expected_value_tag, e) { }
    constexpr expected(value_type&& e) : base(expected_value_tag, std::move(e)) { }
    //template <class... Args> constexpr explicit expected(in_place_t, Args&&...);
    //template <class U, class... Args> constexpr explicit expected(in_place_t, std::initializer_list<U>, Args&&...);
    constexpr expected(unexpected_type<error_type> const& u) : base(expected_error_tag, u.value()) { }
    template <class Err> constexpr expected(unexpected_type<Err> const& u) : base(expected_error_tag, u.value()) { }
    //template <class... Args> constexpr explicit expected(unexpect_t, Args&&...);
    //template <class U, class... Args> constexpr explicit expected(unexpect_t, std::initializer_list<U>, Args&&...);

    ~expected() = default;

    expected& operator=(const expected& e) { type(e).swap(*this); return *this; }
    expected& operator=(expected&& e) { type(std::move(e)).swap(*this); return *this; }
    template <class U> expected& operator=(U&& u) { type(std::move(u)).swap(*this); return *this; }
    expected& operator=(const unexpected_type<error_type>& u) { type(u).swap(*this); return *this; }
    expected& operator=(unexpected_type<error_type>&& u) { type(std::move(u)).swap(*this); return *this; }
    //template <class... Args> void emplace(Args&&...);
    //template <class U, class... Args> void emplace(std::initializer_list<U>, Args&&...);

    void swap(expected& o) {
      using std::swap;
      if (base::has && o.has) {
        swap(base::s.val, o.s.val);
      } else if (base::has && !o.has) {
        error_type e(std::move(o.s.err));
        ::new (&o.s.val) value_type(std::move(base::s.val));
        ::new (&base::s.err) error_type(e);
        swap(base::has, o.has);
      } else if (!base::has && o.has) {
        value_type v(std::move(o.s.val));
        ::new (&o.s.err) error_type(std::move(base::s.err));
        ::new (&base::s.val) value_type(v);
        swap(base::has, o.has);
      } else {
        swap(base::s.err, o.s.err);
      }
    }

    constexpr const value_type* operator->() const { return &base::s.val; }
    value_type* operator->() { return &base::s.val; }
    constexpr const value_type& operator*() const & { return base::s.val; }
    value_type& operator*() & { return base::s.val; }
    constexpr const value_type&& operator*() const && { return std::move(base::s.val); }
    constexpr value_type&& operator*() && { return std::move(base::s.val); }
    constexpr explicit operator bool() const { return base::has; }
    constexpr bool has_value() const { return base::has; }
    constexpr const value_type& value() const & { return base::has ? base::s.val : (unexpected_fail(), base::s.val); }
    constexpr value_type& value() & { return base::has ? base::s.val : (unexpected_fail(), base::s.val); }
    constexpr const value_type&& value() const && { return base::has ? base::s.val : (unexpected_fail(), base::s.val); }
    constexpr value_type&& value() && { return base::has ? base::s.val : (unexpected_fail(), base::s.val); }
    constexpr const error_type& error() const & { return !base::has ? base::s.err : (unexpected_fail(), base::s.err); }
    error_type& error() & { return !base::has ? base::s.err : (unexpected_fail(), base::s.err); }
    constexpr error_type&& error() && { return !base::has ? base::s.err : (unexpected_fail(), base::s.err); }
    constexpr const error_type&& error() const && { return !base::has ? base::s.err : (unexpected_fail(), base::s.err); }
    constexpr unexpected_type<error_type> get_unexpected() const { return unexpected_type<error_type>(base::s.err); }
    template <class U> constexpr value_type value_or(U&& u) const & { return base::has ? **this : static_cast<value_type>(std::forward<U>(u)); }
    template <class U> value_type value_or(U&& u) && { return base::has ? std::move(**this) : static_cast<value_type>(std::forward<U>(u)); }
};

template <class E>
class expected<void, E> : private expected_base_select<void, E> {
    typedef expected_base_select<void, E> base;

public:
    typedef typename base::value_type value_type;
    typedef typename base::error_type error_type;

private:
    typedef expected<value_type, error_type> type;

public:
    template <class U> struct rebind { typedef expected<U, error_type> type; };

    constexpr expected() : base(expected_value_tag) { }
    expected(const expected&) = default;
    expected(expected&&) = default;
    //constexpr explicit expected(in_place_t);
    constexpr expected(unexpected_type<E> const& u) : base(expected_error_tag, u.value()) { }
    template <class Err> constexpr expected(unexpected_type<Err> const& u) : base(expected_error_tag, u.value()) { }

    ~expected() = default;

    expected& operator=(const expected& e) { type(e).swap(*this); return *this; }
    expected& operator=(expected&& e) { type(std::move(e)).swap(*this); return *this; }
    expected& operator=(const unexpected_type<E>& u) { type(u).swap(*this); return *this; } // Not in the current paper.
    expected& operator=(unexpected_type<E>&& u) { type(std::move(u)).swap(*this); return *this; } // Not in the current paper.
    //void emplace();

    void swap(expected& o) {
      using std::swap;
      if (base::has && o.has) {
      } else if (base::has && !o.has) {
        error_type e(std::move(o.s.err));
        ::new (&base::s.err) error_type(e);
        swap(base::has, o.has);
      } else if (!base::has && o.has) {
        ::new (&o.s.err) error_type(std::move(base::s.err));
        swap(base::has, o.has);
      } else {
        swap(base::s.err, o.s.err);
      }
    }

    constexpr explicit operator bool() const { return base::has; }
    constexpr bool has_value() const { return base::has; }
    void value() const { if (!base::has) unexpected_fail(); }
    constexpr const E& error() const & { return !base::has ? base::s.err : (unexpected_fail(), base::s.err); }
    E& error() & { return !base::has ? base::s.err : (unexpected_fail(), base::s.err); } // Not in the current paper.
    constexpr E&& error() && { return !base::has ? base::s.err : (unexpected_fail(), base::s.err); }
    constexpr const E&& error() const && { return !base::has ? base::s.err : (unexpected_fail(), base::s.err); }  // Not in the current paper.
    //constexpr E& error() &;
    constexpr unexpected_type<E> get_unexpected() const { return unexpected_type<E>(base::s.err); }
};

template <class T, class E> constexpr bool operator==(const expected<T, E>& x, const expected<T, E>& y) { return bool(x) == bool(y) && (x ? x.value() == y.value() : x.error() == y.error()); }
template <class T, class E> constexpr bool operator!=(const expected<T, E>& x, const expected<T, E>& y) { return !(x == y); }
template <class T, class E> constexpr bool operator<(const expected<T, E>& x, const expected<T, E>& y) { return (!bool(x) && bool(y)) ? false : ((bool(x) && !bool(y)) ? true : ((bool(x) && bool(y)) ? x.value() < y.value() : x.error() < y.error())); }
template <class T, class E> constexpr bool operator>(const expected<T, E>& x, const expected<T, E>& y) { return !(x == y) && !(x < y); }
template <class T, class E> constexpr bool operator<=(const expected<T, E>& x, const expected<T, E>& y) { return (x == y) || (x < y); }
template <class T, class E> constexpr bool operator>=(const expected<T, E>& x, const expected<T, E>& y) { return (x == y) || (x > y); }

template <class E> constexpr bool operator==(const expected<void, E>& x, const expected<void, E>& y) { return bool(x) == bool(y) && (x ? true : x.error() == y.error()); } // Not in the current paper.
template <class E> constexpr bool operator<(const expected<void, E>& x, const expected<void, E>& y) { return (!bool(x) && bool(y)) ? false : ((bool(x) && !bool(y)) ? true : ((bool(x) && bool(y)) ? false : x.error() < y.error())); } // Not in the current paper.

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

expected<void, WTF::nullopt_t> make_expected() { return expected<void, WTF::nullopt_t>(); }

} // namespace WTF

namespace std {

template <class T, class E> struct hash<WTF::expected<T, E>>
{
    typedef WTF::expected<T, E> argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& e) const { return e ? hash<typename argument_type::value_type>{ }(e.value()) : hash<typename argument_type::error_type>{ }(e.error()); }
};

template <class E> struct hash<WTF::expected<void, E>>
{
    typedef WTF::expected<void, E> argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& e) const { return e ? 0 : hash<typename argument_type::error_type>{ }(e.error()); }
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
