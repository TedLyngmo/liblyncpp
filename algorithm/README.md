# lyn::alg

Algorithms defined in header `lyn/algorithm.hpp`.

#### `lyn::alg::unstable_remove, lyn::alg::unstable_remove_if`

Removes all elements satisfying specific criteria from the range `[first, last)` and returns a past-the-end iterator for the new end of the range.
Removing is done by means of move assigning the elements to be removed in the range that should be kept from elements that should be kept in the range that should be removed. Relative order of the elements that remain is **not** preserved. The physical size of the container is unchanged. All iterators but `end()` are invalidated.

```cpp
template<class ForwardIt, class U>
constexpr ForwardIt
unstable_remove(ForwardIt first, ForwardIt last, const T& value);
```
Removes all elements that are equal to `value`, using `operator==` to compare them.

---
```cpp
template<class ForwardIt, class UnaryPredicate>
constexpr ForwardIt
unstable_remove_if(ForwardIt first, ForwardIt last, UnaryPredicate p);
```
Removes all elements for which predicate `p` returns `true`.

---
#### `lyn::alg::unstable_erase, lyn::alg::unstable_erase_if (std::vector)`
```cpp
template<class T, class Alloc, class U>
[[maybe_unused]] constexpr typename std::vector<T,Alloc>::size_type
unstable_erase(std::vector<T,Alloc>& c, const U& value);
```
Erases all elements that compare equal to `value`.

---
```cpp
template<class T, class Alloc, class Pred>
[[maybe_unused]] constexpr typename std::vector<T,Alloc>::size_type
unstable_erase_if(std::vector<T,Alloc>& c, Pred pred);
```
Erases all elements that satisfy the predicate `pred`.
