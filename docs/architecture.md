# 架构设计 — Memory Pool + KV Store

> 本文是项目的总体架构说明，描述**目标架构**与**当前状态**。术语、代码、命令保留英文原文。

## 1. 目标与范围

单进程、从零实现的 key-value 存储，构建在一个自研内存分配器之上，使用 C++23。

项目采用**渐进式演进**：每个阶段都独立可运行，旧阶段不推倒重来——只在稳定的接口背后替换内部实现。

定位轨迹：**学习载体 → 面试 / 作品项目 → 具备生产能力**。因此"可扩展性"是一等约束，但它通过**接口边界**来表达，而不是通过过早的架构设计（premature abstraction）。

## 2. 分层架构

自底向上三层，每层只依赖下一层暴露的**接口**，不依赖其具体实现：

| 层 | 职责 | 接口 | 实现的演进 |
|---|---|---|---|
| **Server**（阶段 5） | 网络收发、协议解析 | —（待定） | epoll + Reactor + RESP |
| **KVStore** | 数据怎么存、查、删、淘汰 | `KVStore` | Hash → +LRU → +WAL → LSM-tree |
| **Allocator** | 内存怎么分配、回收 | `Allocator`（概念） | System → FixedPool → FreeList → Slab |

核心思想：**底层实现可热替换，上层不受影响**。例如分配器从 `FixedPool` 升级到 `Slab`，KVStore 的代码一行都不用动。这就是"可扩展性不妥协"的落地方式。

## 3. 核心接口

> 以下为**目标接口设计**。当前代码仅落地了 `FixedPool` 的具体接口（见 §4）。抽象基类会在引入**第二个实现**、确实需要运行时替换时再加入——在此之前保持具体类型，避免不必要的虚函数开销与抽象。这本身是渐进式策略的一部分。

### 3.1 Allocator（内存分配接口）

```cpp
class Allocator {
public:
    virtual ~Allocator() = default;
    virtual void* allocate(std::size_t size) = 0;
    virtual void  deallocate(void* ptr) = 0;
};
```

实现演进：`SystemAllocator`（直接 malloc）→ `FixedPool`（固定大小）→ `FreeListAllocator`（可变大小）→ `SlabAllocator`（多档固定大小）。

**运行时多态 vs 编译期多态**：分配器位于热路径，虚函数有调用开销。策略是"先虚接口、后模板"——教学期用抽象基类（清晰、易替换），等需要压榨性能时再用模板 / CRTP（编译期多态）替换。这个权衡本身也是面试可讲的点。

### 3.2 KVStore（存储引擎接口）

```cpp
class KVStore {
public:
    virtual ~KVStore() = default;
    virtual bool get(std::string_view key, std::string& out) = 0;
    virtual void put(std::string_view key, std::string_view val) = 0;
    virtual bool del(std::string_view key) = 0;
};
```

实现演进：`HashKVStore`（内存哈希表）→ 加 LRU 淘汰 → 加 WAL 持久化 → LSM-tree。

## 4. 当前组件：固定大小内存池（FixedPool）

接口见 `include/mempool/fixed_pool.h`。

预分配一整块内存，切成等大的 block，把所有空闲 block 用一条 **intrusive free list** 串起来。`allocate()` / `deallocate()` 都是 O(1)，且不产生外部碎片。适用于**同样大小的小对象高频生灭**——正是 KV entry 节点的形态。

```
预分配一大块，切成等大的 block（这里 4 个）：

  pool_
   │
   ▼
  ┌────────┬────────┬────────┬────────┐
  │ block0 │ block1 │ block2 │ block3 │
  └────────┴────────┴────────┴────────┘

空闲块用"自己的内存"存 next 指针，串成 free list（零额外开销）：

  free_list_
   │
   ▼
  block0 ─next─► block1 ─next─► block2 ─next─► block3 ─► nullptr

allocate():   摘表头              deallocate(p):   插回表头
  p = free_list_;                   p->next = free_list_;
  free_list_ = p->next;             free_list_ = p;
  return p;
```

一个块空闲时，它的内容没人用，于是借它前 `sizeof(pointer)` 字节存指向下一个空闲块的指针——一条链表零额外内存就把所有空闲块串起来了。由此引出两条约束：**块大小必须 ≥ 一个指针的大小**，且必须满足对齐。

## 5. 演进路线

| 阶段 | 组件 | 替换 / 新增的实现 | 状态 |
|---|---|---|---|
| 0 | 项目骨架 | CMake + ASan + 测试框架 | ✅ 完成 |
| 1a | `FixedPool` 实现 | 构造 / allocate / deallocate / 析构 | 🚧 进行中（接口已定，实现待写） |
| 1b | `HashKVStore` | 哈希表，用 FixedPool 管 entry 节点 | 📋 计划 |
| 2 | `SlabAllocator` | 多档固定大小池 | 📋 计划 |
| 3 | 并发安全 | 分段锁（sharding） | 📋 计划 |
| 4 | 持久化 | WAL 预写日志 + 快照 | 📋 计划 |
| 5 | 网络层 | epoll + Reactor + RESP 协议 | 📋 计划 |

## 6. 设计原则

1. **接口边界不妥协，实现可替换**——上层依赖接口不依赖实现；"重构"退化为"替换某层实现"，代价可控。
2. **渐进式交付**——每个阶段都能编译、能跑、有测试；不追求一步到位。
3. **永远不写临时代码**——no TODO / FIXME / HACK / 占位 / 硬编码绕过。现有结构挡路就重构，不绕过。
4. **Sanitizer 默认开启**——AddressSanitizer 常驻，内存 bug 在第一时间暴露。
5. **先虚接口、后模板**——抽象优先清晰，性能压榨留到热路径确有需要时。

## 7. 构建与测试

```bash
cmake -B build -S .                       # 配置（默认 Debug + ASan）
cmake --build build                       # 构建
ctest --test-dir build --output-on-failure  # 跑测试

cmake -B build -S . -DMEMPOOL_ASAN=OFF    # 关闭 ASan（性能基准测试时）
```

## 8. 目录结构

```
mem_pool_kv/
├── CMakeLists.txt          # 顶层构建 + ASan 开关 + 集中编译选项
├── CLAUDE.md               # 给 Claude Code 的项目指引
├── include/mempool/        # 公开头文件
│   └── fixed_pool.h        # FixedPool 接口
├── src/                    # 实现（当前为空，实现落于此）
├── tests/
│   ├── CMakeLists.txt
│   ├── test_framework.h    # 零依赖断言宏
│   └── test_smoke.cpp      # 冒烟测试
└── docs/
    ├── README.md
    └── architecture.md     # 本文件
```

## 9. 当前状态

阶段 0 完成（骨架可构建、可测试）。下一步：阶段 1a，结对实现 `FixedPool` 的四个方法并补测试。详见 §5。
