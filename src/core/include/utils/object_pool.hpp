#ifndef PLIB_CORE_UTILS_OBJECT_POOL_HPP
#define PLIB_CORE_UTILS_OBJECT_POOL_HPP

#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <cassert>
#include <cstddef>

namespace plib::core::utils
{
   /**
    take from taskflow
    线程安全的对象池

    空闲链表：空闲的对象槽连接而成，每个槽的前sizeof(T*)储存了下一个空闲对象槽的地址
    */

   /**
    * @param T: 对象类型
    * @param S: 每个块的大小，默认64kb
    */
   template <typename T, size_t S = 65536>
   class ObjectPool
   {
      // 每个对象槽的大小，确保能容纳下指针
      constexpr static size_t SlotSize = (std::max)(sizeof(T *), sizeof(T));

      // 每个块能容纳的对象数
      constexpr static size_t SlotNumPerBlock = S / SlotSize;

      // 每个块的桶的数量
      constexpr static size_t BinNumPerBlock = 4;

      // 本地堆中的桶数量
      constexpr static size_t localBinNum = BinNumPerBlock + 1;

      // 每个桶的对象槽数量
      constexpr static size_t SlotNumPerBin = (SlotNumPerBlock + BinNumPerBlock - 1) / BinNumPerBlock;

      // 每个桶的空闲对象槽数量
      constexpr static size_t FreeSlotNumPerBin = SlotNumPerBin;

      // 缩容系数
      constexpr static size_t ShrinkFactor = 4;

      // 保证块大小必须为2的幂
      static_assert((S & (S - 1)) == 0, "Block size must be power of 2");

      // 保证每个块至少容纳128个对象槽
      static_assert(SlotNumPerBlock >= 128, "Per Block must be able to hold at least 128 slots");

      struct BlockList
      {
         BlockList *prev;
         BlockList *next;
      };

      struct GlobalHeap
      {
         std::mutex mutex; // 全局堆互斥锁
         BlockList *head;  // 块链表表头
      };

      struct LocalHeap
      {
         std::mutex mutex;             // 本地对互斥锁
         BlockList lists[localBinNum]; // localBinNum条块链表
         size_t used{0};               // 已分配的对象数量
         size_t total{0};              // 总对象数量
      };

      struct Block
      {
         std::atomic<LocalHeap *> heap; // 所属的本地堆，若为空则属于全局堆
         BlockList list_node;           // 在块链表中对应的节点
         size_t index;                  // 已分配对象槽索引
         size_t usedNum;                // 已分配的对象数量
         T *top;                        // 空闲链表表头
         char data[S];                  // 块数据区
      };

   public:
      /**
       * @brief 构造对象池
       * @param thread_num 预期线程数量，实际可以比这个多
       */
      explicit ObjectPool(unsigned int thread_num = std::thread::hardware_concurrency());

      /**
       * @brief 析构对象池,释放所有分配的内存
       */
      ~ObjectPool();

      /**
       * @brief 分配一个对象
       * @tparam Args 构造函数参数类型
       * @param args 构造函数参数
       * @return 指向分配对象的指针
       */
      template <typename... Args>
      T *animate(Args &&...args);

      /**
       * @brief 回收一个对象
       * @param obj 指向要回收的对象的指针
       */
      void recycle(T *obj);

      // 获取每个本地堆的bin数
      size_t num_bins_per_local_heap() const;
      // 获取每个bin的对象数
      size_t num_objects_per_bin() const;
      // 获取每个Block的对象数
      size_t num_objects_per_block() const;
      // 获取可用对象数
      size_t num_available_objects() const;
      // 获取已分配对象数
      size_t num_allocated_objects() const;
      // 获取总容量
      size_t capacity() const;
      // 获取本地堆数量
      size_t num_local_heaps() const;
      // 获取全局堆数量
      size_t num_global_heaps() const;
      // 获取总堆数量
      size_t num_heaps() const;
      // 获取空块阈值
      float emptiness_threshold() const;

   private:
      const size_t _headpMask;                                 // 用于线程哈希的掩码
      GlobalHeap _globalHeap;                                  // 全局堆
      std::vector<LocalHeap> _lheaps;                          // 线程局部堆
      LocalHeap &_this_heap();                                 // 获取当前线程对应的局部堆(根据threadId哈希)
      constexpr unsigned int _next_pow2(unsigned int n) const; // 获取大于等于n的最小2的幂
      // 计算成员在类中的偏移
      template <class P, class Q>
      constexpr size_t _offset_in_class(const Q P::*member) const;
      // 通过成员指针和成员地址获取父类指针
      template <class P, class Q>
      constexpr P *_parent_class_of(Q *, const Q P::*member) const;

      template <class P, class Q>
      constexpr P *_parent_class_of(const Q *, const Q P::*member) const;

      // 通过BlockList节点获取Block指针
      constexpr Block *_block_of(BlockList *node);
      constexpr Block *_block_of(const BlockList *node) const;

      // 计算对象数对应的bin编号
      size_t _bin(size_t) const;

      // 从block分配一个对象
      T *_allocate(Block *);
      // 回收一个对象到Block
      void _deallocate(Block *, T *);
      // 初始化链表头
      void _blocklist_init_head(BlockList *);
      // 在链表中插入节点
      void _blocklist_add_impl(BlockList *, BlockList *, BlockList *);
      // 插入到表头后
      void _blocklist_push_front(BlockList *, BlockList *);
      // 插入到表尾前
      void _blocklist_push_back(BlockList *, BlockList *);
      // 删除链表节点
      void _blocklist_del_impl(BlockList *, BlockList *);
      void _blocklist_del(BlockList *);
      // 替换链表节点
      void _blocklist_replace(BlockList *, BlockList *);
      // 节点移动到链表头
      void _blocklist_move_front(BlockList *, BlockList *);
      // 节点移动到链表尾
      void _blocklist_move_back(BlockList *, BlockList *);
      // 判断是否为链表第一个节点
      bool _blocklist_is_first(const BlockList *, const BlockList *);
      // 判断是否为链表最后一个节点
      bool _blocklist_is_last(const BlockList *, const BlockList *);
      // 判断链表是否为空
      bool _blocklist_is_empty(const BlockList *);
      // 判断链表是否只有一个节点
      bool _blocklist_is_singular(const BlockList *);

      // 遍历链表所有Block（可安全删除）
      template <typename C>
      void _for_each_block_safe(BlockList *, C &&);

      // 遍历链表所有Block
      template <typename C>
      void _for_each_block(BlockList *, C &&);
   };

   template <typename T, size_t S>
   ObjectPool<T, S>::ObjectPool(unsigned int thread_num)
       : _headpMask(_next_pow2((thread_num + 1) << 1) - 1), _globalHeap{std::mutex(), nullptr}, _lheaps(_headpMask + 1)
   {
      // 初始化全局对和局部堆
      _blocklist_init_head(&_globalHeap.head);
      for (auto &lh : _lheaps)
         for (size_t i = 0; i < localBinNum; ++i)
            _blocklist_init_head(&lh.lists[i]);
   }

   template <typename T, size_t S>
   ObjectPool<T, S>::~ObjectPool()
   {
      // 释放所有本地堆
      for (auto &lh : _lheaps)
      {
         _for_each_block_safe(&lh.lists[localBinNum - 1], [this](Block *b)
                              { delete b; });
      }

      // 释放全局堆
      _for_each_block_safe(&_globalHeap.head, [this](Block *b)
                           { delete b; });
   }

   template <typename T, size_t S>
   size_t ObjectPool<T, S>::num_bins_per_local_heap() const
   {
      return localBinNum;
   }

   template <typename T, size_t S>
   size_t ObjectPool<T, S>::num_objects_per_bin() const
   {
      return SlotNumPerBin;
   }

   template <typename T, size_t S>
   size_t ObjectPool<T, S>::num_objects_per_block() const
   {
      return SlotNumPerBlock;
   }

   template <typename T, size_t S>
   float ObjectPool<T, S>::emptiness_threshold() const
   {
      return 1.0f / BinNumPerBlock;
   }

   template <typename T, size_t S>
   size_t ObjectPool<T, S>::num_global_heaps() const
   {
      return 1;
   }

   template <typename T, size_t S>
   size_t ObjectPool<T, S>::num_local_heaps() const
   {
      return _lheaps.size();
   }

   template <typename T, size_t S>
   size_t ObjectPool<T, S>::num_heaps() const
   {
      return _lheaps.size() + 1;
   }

   template <typename T, size_t S>
   size_t ObjectPool<T, S>::capacity() const
   {
      size_t cap = 0;
      for (auto p = _globalHeap.head->next; p != nullptr; p = p->next)
         cap += SlotNumPerBlock;
      for (const auto &lh : _lheaps)
         cap += lh.total;
      return cap;
   }

   template <typename T, size_t S>
   size_t ObjectPool<T, S>::num_available_objects() const
   {
      size_t avail = 0;
      for (auto p = _globalHeap.head->next; p != nullptr; p = p->next)
      {
         avail += (SlotNumPerBlock - _block_of(p)->usedNum);
      }
      for (const auto &lh : _lheaps)
         avail += (lh.total - lh.used);
      return avail;
   }

   template <typename T, size_t S>
   size_t ObjectPool<T, S>::num_allocated_objects() const
   {
      size_t used = 0;
      for (auto p = _globalHeap.head->next; p != nullptr; p = p->next)
      {
         used += _block_of(p)->usedNum;
      }
      for (const auto &lh : _lheaps)
         used += lh.used;
      return used;
   }

   template <typename T, size_t S>
   size_t ObjectPool<T, S>::_bin(size_t objNum) const
   {
      return objNum == SlotNumPerBlock ? BinNumPerBlock : objNum / SlotNumPerBin;
   }

   template <typename T, size_t S>
   template <class P, class Q>
   constexpr size_t ObjectPool<T, S>::_offset_in_class(const Q P::*member) const
   {
      return reinterpret_cast<size_t>(&(reinterpret_cast<P *>(0)->*member));
   }

   template <typename T, size_t S>
   template <class P, class Q>
   constexpr P *ObjectPool<T, S>::_parent_class_of(Q *member_ptr, const Q P::*member) const
   {
      return reinterpret_cast<P *>(reinterpret_cast<char *>(member_ptr) - _offset_in_class(member));
   }

   template <typename T, size_t S>
   template <class P, class Q>
   constexpr P *ObjectPool<T, S>::_parent_class_of(const Q *member_ptr, const Q P::*member) const
   {
      return reinterpret_cast<P *>(reinterpret_cast<const char *>(member_ptr) - _offset_in_class(member));
   }

   template <typename T, size_t S>
   constexpr typename ObjectPool<T, S>::Block *ObjectPool<T, S>::_block_of(BlockList *node)
   {
      return _parent_class_of(node, &Block::list_node);
   }

   template <typename T, size_t S>
   constexpr typename ObjectPool<T, S>::Block *ObjectPool<T, S>::_block_of(const BlockList *node) const
   {
      return _parent_class_of(node, &Block::list_node);
   }

   template <typename T, size_t S>
   void ObjectPool<T, S>::_blocklist_init_head(BlockList *head)
   {
      head->next = head;
      head->prev = head;
   }

   template <typename T, size_t S>
   void ObjectPool<T, S>::_blocklist_add_impl(BlockList *cur_node, BlockList *prev, BlockList *next)
   {
      next->prev = cur_node;
      cur_node->next = next;
      cur_node->prev = prev;
      prev->next = cur_node;
   }

   template <typename T, size_t S>
   void ObjectPool<T, S>::_blocklist_push_front(BlockList *cur_node, BlockList *head)
   {
      _blocklist_add_impl(cur_node, head, head->next);
   }

   template <typename T, size_t S>
   void ObjectPool<T, S>::_blocklist_push_back(BlockList *cur_node, BlockList *head)
   {
      _blocklist_add_impl(cur_node, head->prev, head);
   }

   template <typename T, size_t S>
   void ObjectPool<T, S>::_blocklist_del_impl(BlockList *prev, BlockList *next)
   {
      next->prev = prev;
      prev->next = next;
   }

   template <typename T, size_t S>
   void ObjectPool<T, S>::_blocklist_del(BlockList *node)
   {
      _blocklist_del_impl(node->prev, node->next);
      node->next = nullptr;
      node->prev = nullptr;
   }

   template <typename T, size_t S>
   void ObjectPool<T, S>::_blocklist_replace(BlockList *old_node, BlockList *cur_node)
   {
      cur_node->next = old_node->next;
      cur_node->next->prev = cur_node;
      cur_node->prev = old_node->prev;
      cur_node->prev->next = cur_node;
      old_node->next = nullptr;
      old_node->prev = nullptr;
   }

   template <typename T, size_t S>
   void ObjectPool<T, S>::_blocklist_move_front(BlockList *cur_node, BlockList *head)
   {
      _blocklist_del_impl(cur_node->prev, cur_node->next);
      _blocklist_push_front(cur_node, head);
   }

   template <typename T, size_t S>
   void ObjectPool<T, S>::_blocklist_move_back(BlockList *cur_node, BlockList *head)
   {
      _blocklist_del_impl(cur_node->prev, cur_node->next);
      _blocklist_push_back(cur_node, head);
   }

   template <typename T, size_t S>
   bool ObjectPool<T, S>::_blocklist_is_first(const BlockList *cur_node, const BlockList *head)
   {
      return cur_node->prev == head;
   }

   template <typename T, size_t S>
   bool ObjectPool<T, S>::_blocklist_is_last(const BlockList *cur_node, const BlockList *head)
   {
      return cur_node->next == head;
   }

   template <typename T, size_t S>
   bool ObjectPool<T, S>::_blocklist_is_empty(const BlockList *head)
   {
      return head->next == head;
   }

   template <typename T, size_t S>
   bool ObjectPool<T, S>::_blocklist_is_singular(const BlockList *head)
   {
      return !_blocklist_is_empty(head) && (head->next == head->prev);
   }

   template <typename T, size_t S>
   template <typename F>
   void ObjectPool<T, S>::_for_each_block_safe(BlockList *head, F &&func)
   {
      Blocklist *p;
      Blocklist *t;
      for (p = head->next, t = p->next; p != head; p = t, t = p->next)
         c(_block_of(p));
   }

   template <typename T, size_t S>
   template <typename F>
   void ObjectPool<T, S>::_for_each_block(BlockList *head, F &&func)
   {
      Blocklist *p;
      for (p = head->next; p != head; p = p->next)
         c(_block_of(p));
   }

   template <typename T, size_t S>
   T *ObjectPool<T, S>::_allocate(Block *block)
   {
      if (block->top == nullptr)
      {
         // 如果空闲列表为空则直接从未分配区域分配
         return reinterpret_cast<T *>(block->data + block->index++ * SlotSize);
      }
      else
      {
         // 如果空闲列表不为空则直接从空闲列表分配
         T *obj = block->top;
         block->top = *reinterpret_cast<T **>(obj);
         return obj;
      }
   }

   template <typename T, size_t S>
   void ObjectPool<T, S>::_deallocate(Block *block, T *obj)
   {
      // 将对象回收到空闲列表
      *reinterpret_cast<T **>(obj) = block->top;
      block->top = obj;
   }

   template <typename T, size_t S>
   template <typename... Args>
   T *ObjectPool<T, S>::animate(Args &&...args)
   {
      // 获取当线程对应的局部堆
      LocalHeap &lh = _this_heap();
      Block *block{nullptr};
      lh.mutex.lock();
      // 从最满的桶往下找有空位的块
      int f = static_cast<int>(BinNumPerBlock - 1);
      for (; f >= 0; --f)
      {
         if (!_blocklist_is_empty(&lh.lists[f]))
         {
            block = _block_of(lh.lists[f]->next);
            break;
         }
      }

      if (f == -1)
      {
         _globalHeap.mutex.lock();
         // 在局部堆中没有找到可用的块，则从全局对中获取一个Block放到局部堆中
         if (!_blocklist_is_empty(_globalHeap.head))
         {
            block = _block_of(_globalHeap.head.next);
            assert(block->usedNum < SlotNumPerBlock && block->heap == nullptr);
            f = static_cast<int>(_bin(block->usedNum + 1));
            block->heap = &lh;
            _globalHeap.mutex.unlock();
            lh.used += block->usedNum;
            lh.total += SlotNumPerBlock;
         }
         // 全局堆中也没有可用块，新分配一个Block
         else
         {
            _globalHeap.mutex.unlock();
            f = 0;
            block = new Block();
            block->heap = &lh;
            block->index = 0;
            block->usedNum = 0;
            block->top = nullptr;
            _blocklist_push_front(&block->list_node, lh.lists[f]);
            lh.total += SlotNumPerBlock;
         }
      }
      ++lh.used;
      ++block->usedNum;
      T *obj = _allocate(block);
      int b = static_cast<int>(_bin(block->usedNum));
      if (b != f)
         _blocklist_move_front(&block->list_node, &lh.lists[b]);
      lh.mutex.unlock();
      new (obj) T(std::forward<Args>(args)...);
      obj->_object_pool_block = block;
      return obj;
   }

   template <typename T, size_t S>
   void ObjectPool<T, S>::recycle(T *obj)
   {
      // 获取对象所在的块
      Block *block = obj->_object_pool_block;
      // 析构
      obj->~T();
      // 由于block可能迁移到其他堆，需要循环重试
      bool sync = false;
      do
      {
         LocalHeap *lh = block->heap.load(std::memory_order_acquire);
         // 对象所在块全局堆
         if (lh == nullptr)
         {
            std::lock_guard<std::mutex> globalLock(_globalHeap.mutex);
            if (block->heap == lh)
            {
               sync = true;
               _deallocate(block, obj);
               block->usedNum = block->usedNum - 1;
            }
         }
         else
         {
            std::lock_guard<std::mutex> localLock(lh->mutex);
            if (block->heap == lh)
            {
               sync = true;
               size_t f = _bin(block->usedNum);
               _deallocate(block, obj);
               block->usedNum = block->usedNum - 1;
               lh->used = lh->used - 1;
               size_t b = _bin(block->usedNum);
               if (b != f)
                  _blocklist_move_front(&block->list_node, &lh->lists[b]);
               if ((lh->used + ShrinkFactor * SlotNumPerBlock < lh->total) &&
                   (lh->used < ((BinNumPerBlock - 1) * lh->total) / BinNumPerBlock))
               {
                  // 如果本地堆比较富余，那么将最空闲的块迁移到全局堆中
                  for (size_t i = 0; i < BinNumPerBlock; ++i)
                  {
                     if (!_blocklist_is_empty(&lh.lists[i]))
                     {
                        Block *b = _block_of(lh.lists[i]->next);
                        assert(lh->used > b->usedNum && lh->total >= SlotNumPerBlock);
                        lh->used -= b->usedNum;
                        lh->total -= SlotNumPerBlock;
                        b->heap = nullptr;
                        std::lock_guard<std::mutex> globalLock(_globalHeap.mutex);
                        _blocklist_move_front()(&b->list_node, &_globalHeap.head);
                        break;
                     }
                  }
               }
            }
         }
      } while (!sync);
   }
   // 获取当前线程对应的本地堆
   template <typename T, size_t S>
   typename ObjectPool<T, S>::LocalHeap &ObjectPool<T, S>::_this_heap()
   {
      // 不用thread_local，避免池多次创建销毁导致TLS悬挂
      // 用线程id哈希选本地堆
      return _lheaps[std::hash<std::thread::id>()(std::this_thread::get_id()) & _headpMask];
   }

   // 计算大于等于n的最小2的幂
   template <typename T, size_t S>
   constexpr unsigned ObjectPool<T, S>::_next_pow2(unsigned n) const
   {

      if (n == 0)
         return 1;
      n--;
      n |= n >> 1;
      n |= n >> 2;
      n |= n >> 4;
      n |= n >> 8;
      n |= n >> 16;
      n++;
      return n;
   }
}
#endif