/*
 * C++17 multi iterator - work in progress - do not use
 */

#pragma once

#include <cstddef>
#include <iterator>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace lyn {
template<class... Ts>
class multi_iterator {
public:
    multi_iterator(Ts&... args) : its{{std::begin(args)...}, {std::end(args)...}} {}

    template <class IT>
    struct iterator {
        using ref_type = std::tuple<decltype(*std::begin(std::declval<Ts&>()))...>;

        template<class... Args>
        iterator(Args&&... args) : val{std::forward<Args>(args)...} {}
        //---------------------------------------------------------------------
        template<std::size_t... I>
        void inc(std::index_sequence<I...>) {
            (..., std::advance(std::get<I>(val), 1));
        }
        iterator& operator++() {
            inc(std::make_index_sequence<sizeof...(Ts)>());
            return *this;
        }
        //---------------------------------------------------------------------
        template<class R, std::size_t... I>
        bool equ(const R& rhs, std::index_sequence<I...>) const {
            return (... or (std::get<I>(val) == std::get<I>(rhs.val)));
        }
        template<class R>
        bool operator==(const R& rhs) const {
            return equ(rhs, std::make_index_sequence<sizeof...(Ts)>());
        }
        template<class R>
        bool operator!=(const R& rhs) const {
            return !(*this == rhs);
        }
        //---------------------------------------------------------------------
        template<std::size_t... I>
        void deref(std::index_sequence<I...>) {
            refs = std::make_unique<ref_type>(*std::get<I>(val)...);
        }
        ref_type& operator*() {
            deref(std::make_index_sequence<sizeof...(Ts)>());
            return *refs.get();
        }
        //---------------------------------------------------------------------
    private:
        std::tuple<decltype(std::begin(std::declval<Ts&>()))...> val;
        std::unique_ptr<ref_type> refs;
    };

    //---------------------------------------------------------------------
    template<size_t BE, std::size_t... I>
    auto creit(std::index_sequence<I...>) {
        return iterator{std::get<I>(std::get<BE>(its))...};
    }
    auto begin() { return creit<0>(std::make_index_sequence<sizeof...(Ts)>()); }
    auto end() { return creit<1>(std::make_index_sequence<sizeof...(Ts)>()); }

private:
    std::pair<std::tuple<decltype(std::begin(std::declval<Ts&>()))...>,
              std::tuple<decltype(std::end(std::declval<Ts&>()))...>>
        its;
};

template<class... Ts>
multi_iterator(Ts&&...) -> multi_iterator<std::remove_reference_t<Ts>...>;

} // namespace lyn
