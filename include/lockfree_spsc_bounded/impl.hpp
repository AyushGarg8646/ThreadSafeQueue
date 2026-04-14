#ifndef LOCKFREE_SPSC_BOUNDED_IMPL_CT
#define LOCKFREE_SPSC_BOUNDED_IMPL_CT

#include "defs.hpp"

// template <typename T, size_t Capacity>
// using queue = tsfqueue::__impl::lockfree_spsc_bounded<T, Capacity>;

template <typename T, size_t Capacity>
void tsfqueue::__impl::lockfree_spsc_bounded<T, Capacity>::wait_and_push(T value) {
  size_t cur_tail = tail.load(std::memory_order_acquire);
  size_t next_tail = (cur_tail + 1) % capacity;

  while (next_tail == head_cache) {
    head_cache = head.load(std::memory_order_acquire); // busy wait
  }

  arr[cur_tail] = std::move(value);
  tail.store(next_tail, std::memory_order_release);
}

template <typename T, size_t Capacity>
bool tsfqueue::__impl::lockfree_spsc_bounded<T, Capacity>::peek(T &value) {
  size_t cur_head = head.load(std::memory_order_acquire);
  if (cur_head == tail_cache) {
    tail_cache = tail.load(std::memory_order_acquire);
    if (cur_head == tail_cache) {
      return false;
    }
  }
  value = arr[cur_head];
  return true;
}



template <typename T, size_t Capacity>
bool tsfqueue::__impl::lockfree_spsc_bounded<T, Capacity>::try_push(T value)
{
    return emplace_back(std::move(value));
}

template <typename T, size_t Capacity>
bool tsfqueue::__impl::lockfree_spsc_bounded<T, Capacity>::try_pop(T &value)
{
    size_t current_head = head.load(std::memory_order_acquire);
    if (tail_cache == current_head)
    {
        tail_cache = tail.load(std::memory_order_acquire);
        if (tail_cache == current_head) //false if the queue is still empty after updating the tail cache
            return false;
    }

    value = std::move(arr[current_head]);
    head.store((current_head + 1) % capacity, std::memory_order_release);
    return true;
}

template <typename T, size_t Capacity>
void tsfqueue::__impl::lockfree_spsc_bounded<T, Capacity>::wait_and_pop(T &value)
{
    size_t current_head = head.load(std::memory_order_acquire);
    while (tail_cache == current_head)//waiting till the q is empty 
    {
        tail_cache = tail.load(std::memory_order_acquire); 
    }

    value = std::move(arr[current_head]);
    head.store((current_head + 1) % capacity, std::memory_order_release);
}


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
bool tsfqueue::__impl::lockfree_spsc_bounded<T, Capacity>::empty () const
{
    return (head.load(std::memory_order_relaxed) ==tail.load(std::memory_order_relaxed));
}

template <typename T, size_t Capacity>
size_t tsfqueue::__impl::lockfree_spsc_bounded<T, Capacity>::size() const {
  return (tail.load(std::memory_order_relaxed) -head.load(std::memory_order_relaxed) + capacity) %capacity;
  // again, since size is very frequently changing.
}


#endif

// 1. Add static asserts
// 2. Add emplace_back using perfect forwarding and variadic templates (you
// can use this in push then)
// 3. Add size() function
// 4. Any more suggestions ??