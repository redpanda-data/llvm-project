.. title:: clang-tidy - misc-no-forgotten-usages

misc-no-forgotten-usages
========================

Allows enforcing variables for a specific type are used even if clang would not normally warn that they are unused (because for example they cause side effects).

Motivating example:

.. code-block:: cpp

   void release_lock();

   struct units {
      ~units() {
        release_lock();
      }
   };

   future<units> async_get_lock();

   future<> foo() {
      auto u = async_get_lock(); // missing co_await!
     // do something, but you don't have the lock!
      // u is not marked unused by clang normally
     // adding an unused variable check for future allows forcing the future to be used somehow
     // if you really don't need the future you're forced to use std::ignore = future();
   }

