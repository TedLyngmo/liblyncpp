#pragma once

namespace lyn {
namespace alg {
    // -------------------------------------------------------------------------
    template<class ForwardIt, class U>
    constexpr ForwardIt unstable_remove(ForwardIt first, ForwardIt last, const T& value) {
        return unstable_remove_if(first, last, [&value](auto& v) { return v == value; });
    }
    // -------------------------------------------------------------------------
    template<class ForwardIt, class UnaryPredicate>
    constexpr ForwardIt unstable_remove_if(ForwardIt first, ForwardIt last, UnaryPredicate p) {
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
    }
    // -------------------------------------------------------------------------
    // Erases all elements that compare equal to value
    template<class T, class Alloc, class U>
    [[maybe_unused]] constexpr typename std::vector<T, Alloc>::size_type unstable_erase(std::vector<T, Alloc>& c,
                                                                                        const U& value) {
        return unstable_erase_if(c, [&value](auto& v) { return v == value; });
    }
    // -------------------------------------------------------------------------
    // Erases all elements that satisfy the predicate pred
    template<class T, class Alloc, class Pred>
    [[maybe_unused]] constexpr typename std::vector<T, Alloc>::size_type unstable_erase_if(std::vector<T, Alloc>& c,
                                                                                           Pred pred) {
        using value_type = typename std::vector<T, Alloc>::value_type;
        using size_type = typename std::vector<T, Alloc>::size_type;

        auto p = unstable_remove_if(c.begin(), c.end(), pred);
        auto count = static_cast<size_type>(std::distance(p, c.end()));
        if constexpr(std::is_default_construcible_v<value_type>) {
            c.resize(c.size() - count);
        } else {
            c.erase(p, c.end());
        }

        return count;
    }
    // -------------------------------------------------------------------------
} // namespace alg
} // namespace lyn
