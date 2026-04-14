#ifndef BLOCKING_MPMC_UNBOUNDED_IMPL
#define BLOCKING_MPMC_UNBOUNDED_IMPL

#include "defs.hpp"

// template <typename T>
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
tsfqueue::__impl::blocking_mpmc_unbounded<T>::node *tsfqueue::__impl::blocking_mpmc_unbounded<T>::get_tail()
{
    std::lock_guard<std::mutex> tail_lock(tail_mutex);
    return tail;
}


template <typename T>
std::unique_ptr<typename tsfqueue::__impl::blocking_mpmc_unbounded<T>::node> tsfqueue::__impl::blocking_mpmc_unbounded<T>::try_get()
{
    std::lock_guard<std::mutex> guard_head_mutex(head_mutex);
    if (size() > 0)
    {
        std::unique_ptr<node> out_node = std::move(head);
        head = std::move(out_node->next);

        std::lock_guard<std::mutex> guard_size_mutex(size_mutex);
        size_q--;

        return std::move(out_node); // gives ownership to the the caller function
    }

    return nullptr;
}

template <typename T>
bool tsfqueue::__impl::blocking_mpmc_unbounded<T>::try_pop(T &value)
{

    static_assert(std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>,
                  "T must be copy-assignable or move-assignable to be popped and placed into a reference.");

    std::unique_ptr<node> removed_node = try_get(); // gets deleted when it goes out of scope
    if (removed_node == nullptr)
    {
        return false;
    }
    else
    {
        value = *(removed_node->data); // copying the value to the reference passed in

        return true;
    }
}

template <typename T>
std::shared_ptr<T> tsfqueue::__impl::blocking_mpmc_unbounded<T>::try_pop()
{
    std::unique_ptr<node> removed_node = try_get(); //..
    if (removed_node == nullptr)
    {
        return nullptr;
    }
    else
    {
        return removed_node->data;
    }
}
template <typename T>
std::unique_ptr<typename tsfqueue::__impl::blocking_mpmc_unbounded<T>::node> tsfqueue::__impl::blocking_mpmc_unbounded<T>::wait_and_get() {
  // Locking the head mutex
  std::unique_lock<std::mutex> head_lock(head_mutex);

  // Waiting for the queue to not be empty
  cond.wait(head_lock, [this] { return !empty(); });

  // Extracting the head node and updating it
  std::unique_ptr<node> old_head = std::move(head);
  head = std::move(old_head->next);

  // Updating the queue size
  {
    std::lock_guard<std::mutex> size_lock(size_mutex);
    size_q--;
  }

  return std::move(old_head);
}

template <typename T> void tsfqueue::__impl::blocking_mpmc_unbounded<T>::wait_and_pop(T &val) {
  static_assert(std::is_copy_assignable_v<T> || std::is_move_assignable_v<T>,
                "T must be copy-assignable or move-assignable to be popped "
                "into a reference.");

  // Obtain the front_node with the help of wait_and_get()
  std::unique_ptr<node> front_node = wait_and_get();

  // Move the data into value
  val = *front_node->data; // Removed std::move() from here due to dangling
                        
}

template <typename T> std::shared_ptr<T> tsfqueue::__impl::blocking_mpmc_unbounded<T>::wait_and_pop() {
  // Obtain the front_node with the help of wait_and_get()
  std::unique_ptr<node> front_node = wait_and_get();

  // Return the shared pointer of the data
  return front_node->data;
}

template <typename T>
bool tsfqueue::__impl::blocking_mpmc_unbounded<T>::empty() {}

#endif

// 1. Add static asserts
// 2. Add emplace_back using perfect forwarding and variadic templates (you
// can use this in push then)
// 3. Add size() function
// 4. Any more suggestions ??