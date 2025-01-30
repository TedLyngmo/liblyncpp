#pragma once
#include <algorithm>
#include <iterator>
#include <string>
#include <utility>
#include <type_traits>
#include <vector>

namespace lyn {
namespace alg {
    // -------------------------------------------------------------------------
    template<class ForwardIt, class T>
    constexpr ForwardIt unstable_remove(ForwardIt first, ForwardIt last, const T& value) {
        return unstable_remove_if(first, last, [&value](auto& v) { return value == v; });
    }
    // -------------------------------------------------------------------------
    template<class ForwardIt, class UnaryPredicate>
    constexpr ForwardIt unstable_remove_if(ForwardIt first, ForwardIt last, UnaryPredicate&& p) {
        if constexpr (std::is_base_of_v<std::bidirectional_iterator_tag, typename std::iterator_traits<ForwardIt>::iterator_category>) {
            for(; first != last; ++first) {
                if(p(*first)) { // found one that should be removed

                    // find a "last" that should NOT be removed
                    while(true) {
                        if(--last == first) return last;
                        if(not p(*last)) break; // should not be removed
                    }
                    *first = std::move(*last); // move last to first
                }
            }
            return last;
        } else {
            return std::remove_if(first, last, std::forward<UnaryPredicate>(p));
        }
    }
    // -------------------------------------------------------------------------
    // Erase-Remove idiom algorithms
    namespace detail {
        template<class C, class Pred>
        constexpr auto unstable_erase_if_impl(C& c, Pred&& pred) {
            using value_type = typename C::value_type;
            using size_type = typename C::size_type;

            auto p = unstable_remove_if(c.begin(), c.end(), std::forward<Pred>(pred));
            auto count = static_cast<size_type>(std::distance(p, c.end()));
            if constexpr(std::is_default_constructible_v<value_type>) {
                c.resize(c.size() - count);
            } else {
                c.erase(p, c.end());
            }

            return count;
        }
    } // namespace detail
    // -------------------------------------------------------------------------
    // Erases all elements that compare equal to value
    template<class T, class Alloc, class U>
    [[maybe_unused]] constexpr typename std::vector<T, Alloc>::size_type unstable_erase(std::vector<T, Alloc>& c,
                                                                                        const U& value) {
        return unstable_erase_if(c, [&value](const T& v) { return value == v; });
    }
    // -------------------------------------------------------------------------
    // Erases all elements that satisfy the predicate pred
    template<class T, class Alloc, class Pred>
    [[maybe_unused]] constexpr typename std::vector<T, Alloc>::size_type unstable_erase_if(std::vector<T, Alloc>& c,
                                                                                           Pred pred) {
        return detail::unstable_erase_if_impl(c, std::forward<Pred>(pred));
    }
    // -------------------------------------------------------------------------
    template<class CharT, class Traits, class Alloc, class U>
    [[maybe_unused]] constexpr typename std::basic_string<CharT, Traits, Alloc>::size_type unstable_erase(
        std::basic_string<CharT, Traits, Alloc>& c, const U& value) {
        return unstable_erase_if(c, [&value](CharT v) { return value == v; });
    }
    // -------------------------------------------------------------------------
    // Erases all elements that satisfy the predicate pred
    template<class CharT, class Traits, class Alloc, class Pred>
    [[maybe_unused]] constexpr typename std::basic_string<CharT, Traits, Alloc>::size_type unstable_erase_if(
        std::basic_string<CharT, Traits, Alloc>& c, Pred pred) {
        return detail::unstable_erase_if_impl(c, std::forward<Pred>(pred));
    }
    // -------------------------------------------------------------------------
} // namespace alg
} // namespace lyn
