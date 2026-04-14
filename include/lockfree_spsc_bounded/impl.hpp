#ifndef LOCKFREE_SPSC_BOUNDED_IMPL_CT
#define LOCKFREE_SPSC_BOUNDED_IMPL_CT

#include "defs.hpp"

// template <typename T, size_t Capacity>
// using queue = tsfqueue::__impl::lockfree_spsc_bounded<T, Capacity>;

template <typename T, size_t Capacity>
void queue<T, Capacity>::wait_and_push(T value) {}

template <typename T, size_t Capacity>
bool tsfqueue::__impl::lockfree_spsc_bounded<T, Capacity>::try_push(T value)
{
    return emplace_back(std::move(value));
}

template <typename T, size_t Capacity>
bool queue<T, Capacity>::try_pop(T &value) {}

template <typename T, size_t Capacity>
void queue<T, Capacity>::wait_and_pop(T &value) {}

template <typename T, size_t Capacity>
bool queue<T, Capacity>::peek(T &value) {}

template <typename T, size_t Capacity>
template <typename... Args>
bool tsfqueue::__impl::lockfree_spsc_bounded<T, Capacity>::emplace_back(Args &&...args) {
  size_t tail_cur = tail.load(std::memory_order_acquire);
  size_t next_tail = (tail_cur + 1) % capacity;
  if (next_tail == head_cache) {
    head_cache = head.load(std::memory_order_acquire);
    if (next_tail == head_cache) {
      return false;
    }
  }
  arr[tail_cur] = T(std::forward<Args>(args)...);
  tail.store(next_tail, std::memory_order_release);
  return true;
}
template <typename T, size_t Capacity>
bool tsfqueue::__impl::lockfree_spsc_bounded<T, Capacity>::empty()
{
    return (head.load(std::memory_order_relaxed) ==tail.load(std::memory_order_relaxed));
}

#endif

// 1. Add static asserts
// 2. Add emplace_back using perfect forwarding and variadic templates (you
// can use this in push then)
// 3. Add size() function
// 4. Any more suggestions ??