#ifndef BLOCKING_MPMC_UNBOUNDED_IMPL
#define BLOCKING_MPMC_UNBOUNDED_IMPL

#include "defs.hpp"

template <typename T>
// using queue = tsfqueue::__impl::blocking_mpmc_unbounded<T>;

template <typename T>
void tsfqueue::__impl::blocking_mpmc_unbounded<T>::push(T value)
{
    static_assert(std::is_copy_constructible_v<T> ||
                      std::is_move_constructible_v<T>,
                  "T must be copyable or movable to be pushed into the queue.");
    // Created the shared pointer of "value" 
    std::shared_ptr<T> shared_ptr_for_value =
        std::make_shared<T>(std::move(value));
    
    std::unique_ptr<node> new_tail_unqptr = std::make_unique<node>();

   
    

    // Lock on Producer Thread
    std::lock_guard<std::mutex> tail_mutex_lock(tail_mutex);

    tail->data = std::move(shared_ptr_for_value);
    tail->next = std::move(new_tail_unqptr);

    // Now we move to tail to its next, which is actual tail
    tail = tail->next.get();

    {
        // Increase size
        std::lock_guard<std::mutex> guard_size_mutex(size_mutex);
        size_q++;
    } 
    

    // Notify any waiting threads in "wait_and_pop" to wake up and pop.
    cond.notify_one();

    return;
}

template <typename T>
queue<T>::node *tsfqueue::__impl::blocking_mpmc_unbounded<T>::get_tail() {
    std::lock_guard<std::mutex> tail_lock(tail_mutex);
    return tail;
}

template <typename T>
std::unique_ptr<queue<T>::node> queue<T>::wait_and_get() {}

template <typename T>
std::unique_ptr<queue<T>::node> queue<T>::try_get() {}

template <typename T>
void queue<T>::wait_and_pop(T &value) {}

template <typename T>
std::shared_ptr<T> queue<T>::wait_and_pop() {}

template <typename T>
bool queue<T>::try_pop(T &value) {}

template <typename T>
std::shared_ptr<T> queue<T>::try_pop() {}

template <typename T>
bool queue<T>::empty() {}

#endif

// 1. Add static asserts
// 2. Add emplace_back using perfect forwarding and variadic templates (you
// can use this in push then)
// 3. Add size() function
// 4. Any more suggestions ??