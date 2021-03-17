#pragma once

#include <iterator>
#include <type_traits>

namespace lyn {

template<typename IntType>
class counting_iterator {
public:
    using value_type = IntType;
    using difference_type = std::make_signed_t<IntType>;
    using pointer = const IntType*;
    using reference = const IntType&;
    using iterator_category = std::random_access_iterator_tag;

    counting_iterator() : value(0) {}
    explicit counting_iterator(IntType init) : value(init) {}

    reference operator*() const { return value; }
    value_type operator[](difference_type i) const { return value + i; }
    difference_type operator-(const counting_iterator& rhs) const { return value - rhs.value; }

    counting_iterator& operator+=(difference_type x) {
        value += x;
        return *this;
    }
    counting_iterator& operator-=(difference_type x) {
        value -= x;
        return *this;
    }
    counting_iterator& operator++() {
        ++value;
        return *this;
    }
    counting_iterator& operator--() {
        --value;
        return *this;
    }
    counting_iterator operator++(int) {
        auto copy = *this;
        ++value;
        return copy;
    }
    counting_iterator operator--(int) {
        auto copy = *this;
        --value;
        return copy;
    }
    counting_iterator operator-(difference_type x) const { return {value - x}; }
    counting_iterator operator+(difference_type x) const { return {value + x}; }

    friend counting_iterator operator+(difference_type x, const counting_iterator it) { return {it.value + x}; }

    bool operator==(const counting_iterator& rhs) const { return value == rhs.value; }
    bool operator!=(const counting_iterator& rhs) const { return value != rhs.value; }
    bool operator<(const counting_iterator& rhs) const { return value < rhs.value; }
    bool operator>(const counting_iterator& rhs) const { return value > rhs.value; }
    bool operator<=(const counting_iterator& rhs) const { return value <= rhs.value; }
    bool operator>=(const counting_iterator& rhs) const { return value >= rhs.value; }

private:
    value_type value;
};

} // namespace lyn
