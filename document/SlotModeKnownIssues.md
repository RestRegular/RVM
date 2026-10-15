# 槽位模式的两处既有缺陷

记录于对旧系统做依赖分析、并用 RioKit 打通 Rio→RA→RVM 测试链之后。
两处缺陷**均非静态槽位改造引入**（用改造前的二进制同样复现），但都是槽位模式的正确性问题。

用 RioKit 编译可复现的程序：

```
D:\ClionProjects\RioKit\cmake-build-debug\RioKit.exe compile --target X.rio --o X.ra
RVM.exe --run --target X.ra --slot-mode      # 失败
RVM.exe --run --target X.ra --legacy-mode    # 正确
```

---

## 缺陷 1：递归调用的局部值被内层覆盖

### 现象

`fib` 这类**双递归**在槽位模式失败：

| 用例 | slot 模式 | legacy 模式 |
|---|---|---|
| `addOne(5)` 非递归 | 6 ✅ | 6 |
| `countdown1(1)` 单递归深度 1 | 0 ✅ | 0 |
| `countdown1(3)` 单递归深度 3 | 0 ✅ | 0 |
| `fib(2)` | ❌ `MemoryError: tv409_v1121` | 1 |
| `fib(5)` | ❌ | 5 |

`fib` 生成的 RA（关键部分）：

```
1689: ALLOT: tv408_v1120
1691: ADD:   n_v1116, tv408, tv408     ← 算 n-1
1693: IVOK:  fib, tv408, tv409         ← 递归①，结果写 tv409
1695: ALLOT: tv410_v1122
1696: ADD:   n_v1116, tv410, tv410     ← 算 n-2
1698: IVOK:  fib, tv410, tv411         ← 递归②
1700: ADD:   tv409, tv411, tv412       ← 用递归①的结果 ← 此处报 tv409 不存在
```

### 根因

**帧下标是解析期固定在 `InsSet` 和每个 `Arg` 上的，运行期帧数组直接按这个下标索引。**

```cpp
// ra_core.cpp
readSlot(arg.getFrameIndex(), arg.getSlotIndex())
    → frames[frameIndex]->getSlot(slot)          // 直接索引，无调用层概念
```

`RVM_RuntimeMemory::currentFrameIndex` 那个栈**只用于诊断输出**，不参与槽位解析
（已核对全部引用点）。

因此**递归的每次调用共享同一个帧实例**：进入 `fib` 时 `resetSlots()` 清空整个帧，
把调用方（外层同一帧）刚算出的 `tv409` 一起抹掉。

### 试过但**不成立**的两个修法

1. **去掉 `resetSlots()`**
   崩溃消失，但结果是错的：`fib(2)=0`、`fib(5)=-15`（应为 1、5）。
   因为第 2 次递归仍会覆盖第 1 次的结果槽位——**问题不在清空，在于帧被复用**。

2. **递归时快照/恢复帧槽位**（`enterCall`/`leaveCall` + RAII 守卫）
   `fib` 仍失败，且把原本正常的 `countdown1(3)` 也弄坏了。

### 正确的修法（未实施）

需要**每次函数调用一个帧实例**：

- 在 `InsSet::execute` 进入**帧根**作用域（`frameRootScope`）时，
  **新建一个 `RVM_RuntimeScope`** 而不是复用解析期那一份；
- 槽位解析改为「解析期帧下标 → 当前调用的帧实例」，即
  `frames[parseFrameIndex]` 变成 `activeInstanceOf(parseFrameIndex)`；
- 退出作用域时释放该实例。

影响面：`readSlot` / `writeSlot` 的语义、`InsSet::execute` 的帧 push/pop、
以及解析期帧表与运行期实例的对应关系。这是个**架构级改动**，
不是局部修补——两次局部尝试均已证伪。

---

## 缺陷 2：通过函数变量调用（lambda）失败

### 现象

`ds.iter(list, (item, index, _, _) -> {...})` 这类高阶调用在槽位模式失败：

```
IVOK: handler_v275, tv115_v315, i_v310, iterLength_v278, tv121_v327, tv122_v328
MemoryError: Nonexistent Space Name: 'tv122_v328'
```

| 用例 | slot 模式 | legacy 模式 |
|---|---|---|
| 列表遍历 `ds.iter` | 前几个元素正确后报错 | 10 0 20 1 30 2 40 3 ✅ |
| 嵌套遍历 | ❌ | 1 2 3 4 ✅ |
| 遍历累加 | ❌ | 100 ✅ |

被调函数是**存在槽位里的函数对象**（lambda），不是按名字解析的符号。

### 推测（未验证到底）

错误信息里的名字 `tv122_v328` 是 `ALLOT` 出来的临时槽位，
却走到了旧系统的按名查找路径（`data_space_pool.findDataByNameNoLock`），
说明**参数的槽位解析在"被调函数是槽位里的函数对象"这条路径上没有被正确挂载地址**。

与缺陷 1 很可能**同源**：都是"调用时帧/槽位上下文没有正确切换"。

### 为什么这个更值得优先修

槽位模式是默认路径，`ds.iter` 是高阶函数/回调的基础。
`test_1.ra` 里 `iter_v273` 被调用 14 次——说明现有测试覆盖了这个模式，
但它用的是直接定义的函数名，而不是经由参数传入的 lambda。

---

## 附：验证基础设施

- **RioKit**：`D:\ClionProjects\RioKit\cmake-build-debug\RioKit.exe compile --target X.rio --o X.ra`
  Rio 语法参考 `D:\ClionProjects\RCC\test\rio\*.rio` 与 `D:\ClionProjects\RCC\release\Lib\builtin\*.rio`
- 可复现程序：`fibiso.rio`（缺陷 1）、`itertest.rio`（缺陷 2）、`recmin.rio`（缺陷 1 分级）
- 对照方法：用 `git stash` 构建改动前后的两个二进制，对同一输入比较输出
