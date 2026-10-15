# RSI 产物结构

本文描述 RVM 编译产物 `.rsi` 的二进制布局，对应当前版本 **RSI 1.9.0**。

1.8.0 起携带静态槽位表；**1.9.0 起整数字段使用变长编码，不再是固定原生宽度**，
字符串改由路径表去重，`ArgType` 由 4 字节收窄为 1 字节。

---

## 编码约定

1.9.0 的全部字段都由以下四种编码构成。

| 记号 | 含义 | 说明 |
|---|---|---|
| `u8` | 1 字节无符号 | `bool` / `uint8_t` 字段 |
| `varint` | 无符号 LEB128 | 7 位一组、低位在前，最高位为续读标志 |
| `svarint` | zigzag + LEB128 | 有符号数；`-1` 编码为 1 字节 |
| `string` | `varint len` + `len` 字节 | `len == 0` 时不写内容 |
| `idx+1` | `varint(v + 1)` | 把 `0xFFFFFFFF` 无效哨兵压成 1 字节的 0 |

**变长编码与字节序无关**；只有头部那几个 `uint16` 仍是小端。

`writeSVarint` / `readSVarint` 采用 zigzag：

```
写: u = (uint64)value;  writeVarint((u << 1) ^ (uint64)(value >> 63))
读: u = readVarint();   (int64)((u >> 1) ^ (~(u & 1) + 1))
```

有符号数**必须**用 `svarint`。`end_pointer` 初值是 `-1`，用 `varint` 直接转
`uint64_t` 会占满 10 字节且读回类型极易出错。

`idx+1` 依赖 `uint32_t` 的无符号回绕（回绕在 C++ 中有定义行为）。参与运算的
类型必须是 `uint32_t`，不能被提升成 `int` 或 `uint64_t`。

`string` 的长度上限为 `2^28`，超过即抛异常——这是把损坏文件变成明确诊断、
而不是 `bad_alloc` 的手段。

---

## 总体布局

```
┌──────────────────────────────────────────┐
│ RVMSerialHeader            固定 12 字节   │
├──────────────────────────────────────────┤
│ 链接扩展表                 变长           │
├──────────────────────────────────────────┤
│ 静态槽位表                 变长           │
├──────────────────────────────────────────┤
│ 路径表（1.9.0 新增）        变长           │
├──────────────────────────────────────────┤
│ InsSet（递归）             变长           │
└──────────────────────────────────────────┘
```

顺序由 `serializeExecutableInsToBinaryFile` 决定。**两个「必须在指令流之前」的约束：**

1. **静态槽位表** —— 指令里写死的帧下标要靠它先建好运行期帧表才有落点。
2. **路径表** —— 反序列化 `Pos` 时就要能按下标取回路径。

路径表只有在整个指令流写完后才完整，所以指令流先写进内存缓冲区，最后整体落盘：

```cpp
utils::PathTable paths;              // 局部变量，非全局状态（见下）
std::ostringstream body;
ins_set->serialize(body, profile, paths);

base::rvm_serial_header.serialize(binary_file);
env::serializeLinkedExtensions(binary_file);
getLastSlotTable().serialize(binary_file);
paths.serialize(binary_file);
binary_file << body.rdbuf();
```

> **路径表为什么显式传参而不是全局/线程局部状态**
>
> `ra_parser` 在解析主文件的循环里，遇到 `LINK` + `--precomp-link` 会**递归**调用
> `serializeExecutableInsToBinaryFile`（见 `LINK` 分支）。若用全局表，内层调用开头
> 的 `clear()` 会把主文件已累积的路径清掉，写进主文件的路径表就被链接文件的内容污染。
> 显式传参让嵌套调用各持一张表，天然隔离。

---

## 1. RVMSerialHeader（12 字节，`#pragma pack(push,1)`）

| 偏移 | 大小 | 字段 | 说明 |
|---|---|---|---|
| 0 | 4 | `magic[4]` | 固定 `52 53 49 1A`（`'R','S','I',0x1A`） |
| 4 | 2 | `major_ver` | 主版本，当前 `1` |
| 6 | 2 | `minor_ver` | 次版本，当前 `9` |
| 8 | 2 | `patch_ver` | 修订号，当前 `0` |
| 10 | 2 | `profile` | 序列化档位 |

结构体带 `static_assert(sizeof(RVMSerialHeader) == 12)`。

**`Minified` 档位不写这个头**（见 `RVMSerialHeader::serialize` 的
`profile < Minified` 判断）。因此运行 Minified 产物时必须同时给
`--comp-level Minified`，否则反序列化侧会去读不存在的头：
`validate()` 拿到垃圾 magic，报 `Invalid file format!`。

### 档位语义

| 值 | 档位 | 语义 |
|---|---|---|
| 0 | `Debug` | 完整信息：源码、符号表、行号 |
| 1 | `Testing` | 移除源码，保留符号表与行号 |
| 2 | `Release` | 仅指令流 + 必要元数据 |
| 3 | `Minified` | **无头部**、无版本/元数据 |

代码里一律用 `profile < X` 判断，含义是「**比 X 更详细**」：

| 判断 | 成立的档位 |
|---|---|
| `profile < Testing` | 仅 `Debug` |
| `profile < Release` | `Debug`、`Testing` |
| 无条件写 | 全部四个档位 |

### 版本校验

`RVMSerialHeader::validate` 要求**逐段完全一致**，任一不同即拒绝：

```
Unsupported version
RVM version: ...
RSI File Version: ...
Recompile the source with this RVM version to produce a compatible RSI.
```

> 原实现只在「文件版本**高于** RVM 版本」时报错，低版本文件会一路通过校验，
> 然后在反序列化阶段以 `std::bad_alloc` 崩掉。1.8.0 已改为逐段比较。

---

## 2. 链接扩展表

```
varint   extension_count
重复 extension_count 次：
    string   name
```

只存扩展**名字**。名字指向的文件在运行期按需重新加载（源码里标了 `// FixMe`）。

---

## 3. 静态槽位表

```
varint   frame_count
重复 frame_count 次：
    varint   slot_span              该帧槽位规模（最大槽位号 + 1，至少 1）
    idx+1    parent_frame_index     词法父帧下标；无父帧时为 0xFFFFFFFF → 编码为 0
    u8       is_frame_root          1 = 帧根（词法链的终点）

varint   root_frame_index
varint   slot_name_count
重复 slot_name_count 次：
    string   name
```

**帧下标就是帧的身份。** 它与写在各 `InsSet` / `Arg` 里的 `frame_index` 一一对应，
所以反序列化时必须**按原下标顺序**重建，不能重新编号。

`root_slot_names[i]` 是根帧第 `i` 号槽位里的名字。这些名字（`tp-*`、`true`、`false`、
`null`、`SN`、`SE`、`SS` 等）在**编译期**就分配好了槽位，但它们的**值**来自
`RVM_Memory` 的全局作用域，所以反序列化时要按这份名单把全局数据复制进根帧槽位。

帧表为空时**直接抛错**，不静默降级：

```
The RSI file carries no static slot table.
Frame count read: 0
Recompile the source with this RVM version to produce a compatible RSI.
```

---

## 4. 路径表（1.9.0 新增）

```
varint   path_count
重复 path_count 次：
    string   path
```

`Pos.filepath` 与 `InsSet.scope_leader_file` 不再内联字符串，改存这张表的下标。
同一个文件路径在每条指令、每个参数里都会重复出现，内联是 1.8.0 最大的单项开销：
`test_2.ra` 里 `arg.pos` 一项就占 31.0%（3774 字节），改用下标后降到 6.8%（222 字节）。

写入端用 `intern` 去重；**读取端直接按顺序 `push_back`，不建索引**——
文件里的顺序就是下标，若表内出现重复项，用 `intern` 会把下标压歪、与写入端不一致。

---

## 5. InsSet（递归）

```
varint   frame_index           本指令集所属帧下标
u8       is_frame_root
u8       is_delayed_release_scope
svarint  end_pointer           初值 -1，必须有符号

if profile < Testing:          Debug
    string   scope_prefix
    string   scope_leader

if profile < Release:          Debug / Testing
    string   scope_leader_pos
    varint   scope_leader_file_id   路径表下标

varint   ins_count
Ins × ins_count

varint   label_count
重复 label_count 次：
    string   key
    svarint  value                指令下标，允许 -1（无匹配）
```

`InsSet::is_delayed_release_scope` 会被写盘、也会被读回成员，但**运行期无人读取**它——
`InsSet::execute` 里那个同名判断用的是 `Ins` 上的字段（见第 6 节），不是这个。
`InsSet` 上的这份属于冗余字段（既有行为，未改动）。

`scope_name_stack` 的序列化代码已被注释掉，故不在布局中。

**标签表必须按 key 排序后写出。** `set_labels` 是 `std::unordered_map`，其迭代顺序
取决于哈希与 rehash 历史，而两条构建路径的插入顺序并不相同：

- 解析期：先由 `END:` 登记 `end_pointer` 标签，再按 `SET:` 出现顺序；
- 反序列化：严格按文件里的顺序读入。

直接迭代会让「`.ra → .rsi`」与「`.rsi → .rsi2`」写出不同字节序（文件大小相同、
只是顺序不同），往返测试因此失败。排序后两条路径产出完全一致的字节流。

---

## 6. Ins

```
Pos      pos                      见 6.1
varint   ri_id                    RI 编号
varint   args_count
Arg × args_count
u8       has_scope_ins_set        等于 ri->hasScope
if has_scope_ins_set:
    InsSet（递归）
if profile < Testing:             Debug
    string   raw_code
u8       is_delayed_release_scope
string   ext
```

`ri_id` 反序列化时在 `insMap` 里查回 `RI*`，查不到报 `Invalid RI id: N`。

`Ins::is_delayed_release_scope` **运行时确实在用**：`ri_end` 据此决定是
`clear()`（延迟释放的块作用域，让下一轮迭代从干净状态开始）还是 `releaseScopeNoLock()`。
解析期由所属 `InsSet` 的对应字段写入。

### 6.1 Pos

```
if profile < Release:          Debug / Testing / Release
    svarint  line
    svarint  column
    varint   filepath_id       路径表下标
```

`Minified` 档位下 `pos` **完全不写**，反序列化后保持默认值——这也是该档位下
路径表可能为空（仅 1 字节）的原因，属正常，不需特殊处理。

---

## 7. Arg

```
Pos      pos                      同 6.1（受 profile 控制）
u8       type                     ArgType，固定 1 字节，无视 profile
string   value
idx+1    frame_index              INVALID_SLOT(0xFFFFFFFF) + 1 回绕为 0
idx+1    slot_index
```

`ArgType` 由 `utils::serializeArgType` 写出，**不随 profile 变化**（与 `Pos`、
`raw_code` 不同）。新增枚举值时必须仍能装进 1 字节，`ra_utils.h` 里有
`static_assert(static_cast<int>(ArgType::keyword) < 256)` 把关。

`frame_index` / `slot_index` 是静态槽位架构的核心。解析期把每个参数的名字换算成
「帧下标 + 帧内槽位号」写在这里，运行期只做 `frames[idx]->slots[n]` 两次数组索引，
没有任何名字或字符串查找。未解析到槽位时写 `0xFFFFFFFF`，运行期取到它会报槽位模型错误。

这两个字段在**所有档位**下都写。若放进 `profile < Release` 之类的分支，
`Release` 产物就会缺地址而无法走新架构——而 `Release` 恰恰是生产档位。

---

## 8. 实测分区

对 `test_2.ra` 的 Debug 产物（**3272 字节**）逐字段走查，消耗全部字节、无剩余无越界。

### 顶层分区

| 区段 | 字节 | 占比 |
|---|---|---|
| 头 `RVMSerialHeader` | 12 | 0.4% |
| 链接扩展表（`count=0`） | 1 | 0.0% |
| 静态槽位表 | 204 | 6.2% |
| 路径表 | 37 | 1.1% |
| `InsSet` | 3018 | 92.2% |

槽位表：`frames=7`、`root_frame_index=0`、`root_slot_names=25`。
路径表：1 条（`D:\ClionProjects\RVM\test\test.ra`）。

### InsSet 内部（顶层，各段互斥）

| 区段 | 字节 | 占比 |
|---|---|---|
| `set.fixed`（帧下标+帧根+延迟+`end_pointer`） | 4 | 0.1% |
| `set.debugStr`（`scope_prefix`+`scope_leader`） | 45 | 1.4% |
| `set.relStr`（`scope_leader_pos`+路径下标） | 49 | 1.5% |
| `set.inscount` / `set.labelcount` | 各 1 | 0.0% |
| `ins.hdr`（`ri_id`+`args_count`） | 14 | 0.4% |
| `ins.hasscope` | 7 | 0.2% |
| `ins.rawcode` | 140 | 4.3% |
| `ins.delayed` / `ins.ext` | 各 7 | 0.2% |
| `arg.total`（顶层指令的 Arg） | 979 | 29.9% |
| `nested.total`（子指令集） | 3584 | — |

实例计数：`InsSet` 8 个、`Ins` 45 条、`Arg` 74 个、标签 8 条。

### Arg 内部（全文件 74 个累计）

| 区段 | 字节 | 占比 | 1.8.0 对比 |
|---|---|---|---|
| `arg.pos` | 222 | 6.8% | 3774 |
| `arg.type` | 74 | 2.3% | 296 |
| `arg.value` | 535 | 16.4% | 1053 |
| `arg.slotaddr` | 148 | 4.5% | 592 |
| 合计 | 979 | 29.9% | 5715 |

**每个 Arg 平均开销从 77.2 字节降到 13.2 字节。**

- `pos`：51.0 → 3.0 字节（`svarint` 行号/列号 + 路径下标）
- 槽位地址：8.0 → 2.0 字节（`idx+1`，无效哨兵只占 1 字节）
- `type`：4.0 → 1.0 字节

---

## 9. 体积对比

| 文件 | 1.8.0 | 1.9.0 | 变化 |
|---|---|---|---|
| `test.ra` | 4155 B | 1012 B | −75.6% |
| `test_2.ra` | 12169 B | 3272 B | −73.1% |
| `test_1.ra` | 482315 B | 140272 B | −70.9% |

各档位（`test_2.ra`）：Debug 3272 / Testing 2202 / Release 1409 / Minified 1397。

---

## 10. 验证工具

`--dump-rsi` + `--dump-rsi-to <path>` 读取 `.rsi`、反序列化后再序列化，
用于验证写入端与读取端对称：正确实现下产物与输入**逐字节相同**。

```bash
# 往返验证（Minified 无头，两次都要给 --comp-level）
rvm --run --target a.rsi --comp-level Debug --dump-rsi --dump-rsi-to b.rsi
cmp a.rsi b.rsi      # 应完全一致
```

已在四个档位 × 三个用例上验证 12/12 逐字节一致，且两种运行模式输出与直接运行
`.ra` 完全一致。

错误路径（截断、版本不符、magic 损坏）均给出明确诊断，未出现 `bad_alloc` 或崩溃。

---

## 11. 与运行期模式的对应

| 来源 | 槽位表从哪来 | 可用模式 |
|---|---|---|
| `.ra` | 解析期 `prepareRuntimeFrames` 建立 | `--slot-mode`（默认）/ `--legacy-mode` |
| `.rsi` 1.9.0 | 反序列化时由第 3 节重建 | 同上 |
| `.rsi` ≤ 1.8.0 | 无（且布局已变） | 版本校验直接拒绝 |

`--legacy-mode` 强制关掉静态槽位，走旧的动态作用域系统，用于回退与 A/B 对照。
旧系统完全不需要第 3 节与第 7 节的槽位字段——它对每个指令里的 `frame_index` /
`slot_index` 视而不见，仍按 `Arg::value` 里的名字查表。

**`Minified` 的额外要求**：该档位不写头，运行时必须显式给 `--comp-level Minified`。
