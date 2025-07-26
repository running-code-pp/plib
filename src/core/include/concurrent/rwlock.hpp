#ifndef PLIB_CORE_CONCURRENT_RWLOCK_HPP
#define PLIB_CORE_CONCURRENT_RWLOCK_HPP

#include <shared_mutex>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <string>

namespace plib::core::concurrent{

    template <typename T>
    class LockedRW
    {
      public:
      explicit LockedRW(T obj = T()) : data_(std::move(obj)) {}
    
      // 读操作，返回func返回的类型
      template <typename Func>
      auto access_read(Func func) const
      {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return func(data_);
      }
    
      // 写操作，返回func返回的类型
      template <typename Func>
      auto access_write(Func func)
      {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        return func(data_);
      }
    
      private:
      T data_;
      mutable std::shared_mutex mutex_;
    };
    

}
#endif // PLIB_CORE_CONCURRENT_RWLOCK_HPP