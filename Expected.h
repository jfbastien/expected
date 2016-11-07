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

namespace WTF {

template <class E>
class unexpected_type {
public:
    unexpected_type() = delete;
    constexpr explicit unexpected_type(const E&);
    constexpr explicit unexpected_type(E&&);
    constexpr const E& value() const;
    constexpr E& value();

private:
    E val;
};

template <class E> constexpr bool operator==(const unexpected_type<E>&, const unexpected_type<E>&);
template <class E> constexpr bool operator!=(const unexpected_type<E>&, const unexpected_type<E>&);
template <class E> constexpr bool operator<(const unexpected_type<E>&, const unexpected_type<E>&);
template <class E> constexpr bool operator>(const unexpected_type<E>&, const unexpected_type<E>&);
template <class E> constexpr bool operator<=(const unexpected_type<E>&, const unexpected_type<E>&);
template <class E> constexpr bool operator>=(const unexpected_type<E>&, const unexpected_type<E>&);

template <class E> constexpr unexpected_type<decay_t<E>> make_unexpected(E&& v);

struct unexpect_t { unexpect_t() = delete; };
constexpr unexpect_t unexpect { };

template <class T, class E> constexpr bool operator==(const expected<T, E>&, const expected<T, E>&);
template <class T, class E> constexpr bool operator!=(const expected<T, E>&, const expected<T, E>&);
template <class T, class E> constexpr bool operator<(const expected<T, E>&, const expected<T, E>&);
template <class T, class E> constexpr bool operator>(const expected<T, E>&, const expected<T, E>&);
template <class T, class E> constexpr bool operator<=(const expected<T, E>&, const expected<T, E>&);
template <class T, class E> constexpr bool operator>=(const expected<T, E>&, const expected<T, E>&);

template <class T, class E> constexpr bool operator==(const expected<T, E>&, const T&);
template <class T, class E> constexpr bool operator==(const T&, const expected<T, E>&);
template <class T, class E> constexpr bool operator!=(const expected<T, E>&, const T&);
template <class T, class E> constexpr bool operator!=(const T&, const expected<T, E>&);
template <class T, class E> constexpr bool operator<(const expected<T, E>&, const T&);
template <class T, class E> constexpr bool operator<(const T&, const expected<T, E>&);
template <class T, class E> constexpr bool operator<=(const expected<T, E>&, const T&);
template <class T, class E> constexpr bool operator<=(const T&, const expected<T, E>&);
template <class T, class E> constexpr bool operator>(const expected<T, E>&, const T&);
template <class T, class E> constexpr bool operator>(const T&, const expected<T, E>&);
template <class T, class E> constexpr bool operator>=(const expected<T, E>&, const T&);
template <class T, class E> constexpr bool operator>=(const T&, const expected<T, E>&);

template <class T, class E> constexpr bool operator==(const expected<T, E>&, const unexpected_type<E>&);
template <class T, class E> constexpr bool operator==(const unexpected_type<E>&, const expected<T, E>&);
template <class T, class E> constexpr bool operator!=(const expected<T, E>&, const unexpected_type<E>&);
template <class T, class E> constexpr bool operator!=(const unexpected_type<E>&, const expected<T, E>&);
template <class T, class E> constexpr bool operator<(const expected<T, E>&, const unexpected_type<E>&);
template <class T, class E> constexpr bool operator<(const unexpected_type<E>&, const expected<T, E>&);
template <class T, class E> constexpr bool operator<=(const expected<T, E>&, const unexpected_type<E>&);
template <class T, class E> constexpr bool operator<=(const unexpected_type<E>&, const expected<T, E>&);
template <class T, class E> constexpr bool operator>(const expected<T, E>&, const unexpected_type<E>&);
template <class T, class E> constexpr bool operator>(const unexpected_type<E>&, const expected<T, E>&);
template <class T, class E> constexpr bool operator>=(const expected<T, E>&, const unexpected_type<E>&);
template <class T, class E> constexpr bool operator>=(const unexpected_type<E>&, const expected<T, E>&);

void swap(expected<T, E>&, expected<T, E>&);

template <class T> constexpr expected<decay_t<T>> make_expected(T&& v);
expected<void> make_expected();
template <class T, class E> constexpr expected<T, decay_t<E>> make_expected_from_error(E&& e);
template <class T, class E, class U> constexpr expected<T, E> make_expected_from_error(U&& u);
template <class F> constexpr expected<typename result_type<F>::type> make_expected_from_call(F f);

template <class T, class E> struct hash<expected<T, E>>;
template <class E> struct hash<expected<void, E>>;

template <class T, class E>
class expected {
public:
    typedef T value_type;
    typedef E error_type;
    template <class U> struct rebind { using type = expected<U, error_type>; };

    constexpr expected();
    expected(const expected&);
    expected(expected&&);
    constexpr expected(const T&);
    constexpr expected(T&&);
    template <class... Args> constexpr explicit expected(in_place_t, Args&&...);
    template <class U, class... Args> constexpr explicit expected(in_place_t, initializer_list<U>, Args&&...);
    constexpr expected(unexpected_type<E> const&);
    template <class Err> constexpr expected(unexpected_type<Err> const&);
    template <class... Args> constexpr explicit expected(unexpect_t, Args&&...);
    template <class U, class... Args> constexpr explicit expected(unexpect_t, initializer_list<U>, Args&&...);

    ~expected();

    expected& operator=(const expected&);
    expected& operator=(expected&&);
    template <class U> expected& operator=(U&&);
    expected& operator=(const unexpected_type<E>&);
    expected& operator=(unexpected_type<E>&&);
    template <class... Args> void emplace(Args&&...);
    template <class U, class... Args> void emplace(initializer_list<U>, Args&&...);

    void swap(expected&);

    constexpr const T* operator->() const;
    T* operator->();
    constexpr const T& operator*() const &;
    T& operator*() &;
    constexpr const T&& operator*() const &&;
    constexpr T&& operator*() &&;
    constexpr explicit operator bool() const;
    constexpr bool has_value() const;
    constexpr const T& value() const &;
    constexpr T& value() &;
    constexpr const T&& value() const &&;
    constexpr T&& value() &&;
    constexpr const E& error() const &;
    E& error() &;
    constexpr E&& error() &&;
    constexpr const E&& error() const &&;
    constexpr unexpected_type<E> get_unexpected() const;
    template <class U> constexpr T value_or(U&&) const &;
    template <class U> T value_or(U&&) &&;

private:
    bool has_value;
    union {
        value_type val;
        error_type err;
    };
};

template <class E>
class expected<void, E> {
public:
    typedef void value_type;
    typedef E error_type;
    template <class U> struct rebind { typedef expected<U, error_type> type; };

    constexpr expected();
    expected(const expected&);
    expected(expected&&);
    constexpr explicit expected(in_place_t);
    constexpr expected(unexpected_type<E> const&);
    template <class Err> constexpr expected(unexpected_type<Err> const&);

    ~expected();

    expected& operator=(const expected&);
    expected& operator=(expected&&);
    void emplace();

    void swap(expected&);

    constexpr explicit operator bool() const;
    constexpr bool has_value() const;
    void value() const;
    constexpr const E& error() const &;
    constexpr E& error() &;
    constexpr E&& error() &&;
    constexpr unexpected_type<E> get_unexpected() const;

private:
    bool has_value;
    union {
        unsigned char dummy;
        error_type err;
    };
};

}

#endif
