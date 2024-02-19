#pragma once

/*
 * lyn::initialize
 * This provides an alternative to using constructors taking an initializer_list
 * to avoid copying elements.
 *
 * This is work in progress!
 *
 * Copied from:
 * https://github.com/TedLyngmo/liblyncpp/blob/main/include/lyn/initialize.hpp
 * "This is free and unencumbered software released into the public domain."
 */

#include <type_traits>
#include <utility>
//----------------------------------------------------------------------------------
namespace lyn {
template<class, class = void>
struct can_reserve : std::false_type {};

template<class T>
struct can_reserve<T, std::void_t<decltype(std::declval<T&>().reserve(std::declval<std::size_t>()))>> :
    std::true_type {};

template<class T>
inline constexpr bool can_reserve_v = can_reserve<T>::value;
//----------------------------------------------------------------------------------
template<class, class, class = void>
struct can_emplace_back : std::false_type {};

template<class C, class T>
struct can_emplace_back<C, T, std::void_t<decltype(std::declval<C&>().emplace_back(std::declval<T>()))>> :
    std::true_type {};

template<class C, class T>
inline constexpr bool can_emplace_back_v = can_emplace_back<C, T>::value;
//----------------------------------------------------------------------------------
template<class, class, class = void>
struct can_emplace : std::false_type {};

template<class C, class T>
struct can_emplace<C, T, std::void_t<decltype(std::declval<C&>().emplace(std::declval<T>()))>> : std::true_type {};

template<class C, class T>
inline constexpr bool can_emplace_v = can_emplace<C, T>::value;
//----------------------------------------------------------------------------------
template<class C, class T>
concept can_emplace_somehow = can_emplace_back_v<C, T> || can_emplace_v<C, T>;
//----------------------------------------------------------------------------------
// calls the member function C::reserve
template<class C>
    requires can_reserve_v<C>
void reserve(C& c, size_t size) {
    if constexpr(can_reserve_v<C>) {
        c.reserve(size);
    }
}
//----------------------------------------------------------------------------------
// calls the member function C::emplace_back or C::emplace
template<class C, class T>
    requires can_emplace_somehow<C, T>
void emplace_somehow(C& c, T&& value) {
    if constexpr(can_emplace_back_v<C, T>) {
        c.emplace_back(std::forward<T>(value));
    } else {
        c.emplace(std::forward<T>(value));
    }
}
//----------------------------------------------------------------------------------
template<class C, class... Args>
    requires(... && can_emplace_somehow<C, Args>)
C initialize(Args&&... args) {
    C res;
    if constexpr(can_reserve_v<C>) {
        reserve(res, sizeof...(Args));
    }
    (..., emplace_somehow(res, std::forward<Args>(args)));
    return res;
}
} // namespace lyn
