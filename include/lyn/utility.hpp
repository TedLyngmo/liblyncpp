#pragma once

#include <cstddef>
#include <tuple>
#include <utility>

namespace detail {
    template <class T, T... I, std::size_t... J>
    constexpr auto rev_helper(std::integer_sequence<T, I...>, std::index_sequence<J...>) {
        //  <decltype(I)...>  below needed to make it work in C++14
        return std::integer_sequence<T, std::get<sizeof...(J) - J - 1>(std::tuple<decltype(I)...>{I...})...>{};
    }
} // namespace detail

template <class T, T... I>
constexpr auto reverse_sequence(std::integer_sequence<T, I...> s) {
    return detail::rev_helper(s, std::make_index_sequence<sizeof...(I)>{});
}
