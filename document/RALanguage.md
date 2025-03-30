## RA语言规范

#### [@RestRegular](https://github.com/RestRegular) 2025/3/22 13:27:03

### 一、引言

#### 1.1 背景与目标

RA语言（Rio Assembly Language）是一种类汇编的指令式编程语言，专为 RVM（Rio Virtual Machine）虚拟机设计。其主要目标是为 RVM
提供一种高效、灵活且易于理解的低级语言，用于实现 RVM 的指令集。RA语言的设计灵感来源于传统汇编语言，但在语法结构和语义规则上进行了优化，以更好地适应
RVM 的运行环境。

通过 RA语言，开发者可以直接编写底层指令，控制 RVM 的内存空间和栈操作，从而实现高性能的应用程序开发。同时，RA语言也为高级语言编译器提供了中间表示层，使得其他语言可以通过编译生成
RA代码，进而运行在 RVM 上。

#### 1.2 设计原则

RA语言的设计遵循以下核心原则：

- **简洁性** ：语法结构尽量简单，降低学习成本，同时便于编译器和解释器的实现。
- **高效性** ：指令集设计贴近 RVM 的硬件抽象层，确保执行效率最大化。
- **可扩展性** ：支持未来对指令集的扩展，允许新增功能而不破坏现有代码的兼容性。
- **易用性** ：尽管是低级语言，但设计时提供了一定的抽象能力（如动态类型系统等），提升开发体验。

#### 1.3 应用场景

RA语言适用于以下场景：

- **底层开发** ：直接编写 RVM 的原生指令，用于实现操作系统内核、驱动程序或其他需要高性能的组件。
- **编译器后端** ：作为高级语言（如 Rio、Python、C 等）编译器的目标语言，生成高效的 RVM 可执行代码。
- **教学与研究** ：由于其接近硬件的特性，RA语言非常适合用于计算机体系结构和编译原理的教学与研究。
- **嵌入式系统** ：在资源受限的环境中，RA语言可以作为一种轻量级的语言选择。

#### 1.4 与其他语言的对比

RA语言的设计借鉴了传统汇编语言的特点，但与之相比有以下不同：

- **更强的语法结构** ：RA语言引入了统一的语法结构，简化了指令的编写和理解。
- **虚拟机专用** ：RA语言专为 RVM 设计，因此其指令集与 RVM 的架构高度匹配，而传统汇编语言通常针对物理硬件。
- **跨平台支持** ：由于 RVM 是一个虚拟机，RA语言天生具有跨平台特性，无需考虑底层硬件差异。

#### 1.5 文档结构

本文档将详细介绍 RA语言的设计与使用方法，主要包括以下内容：

1. [x] **词法结构** ：描述 RA语言的词法规则，包括关键字、标识符、注释和分隔符。
2. [x] **语法结构** ：描述 RA语言的基本语法规则，包括指令格式、数据类型。
3. [x] **语义规则** ：解释每条指令的具体行为及其在 RVM 上的执行过程。
4. [ ] **标准库**：列出 RA语言内置的标准库函数及其用途。
5. [ ] **运行时**：介绍 RA语言在 RVM 上的运行机制，包括内存管理、栈操作等。
6. [ ] **错误处理** ：描述 RA语言在运行时可能遇到的错误，并提供相应的处理方法。
7. [ ] **兼容性**：介绍 RA语言与 RVM 之间的兼容性，包括指令集、数据类型等。
8. [ ] **示例代码** ：通过实际案例展示 RA语言的使用方法。

### 二、词法结构

#### 2.1 字符集

RA语言支持以下字符集：

- **英文字母** ：`a-z`和`A-Z`
- **数字** ：`0-9`
- **特殊字符** ：（包括但不限于以下字符）
    - 注释符：`;`（单行注释符）
    - 连接符：`~`
    - 分隔符：`:`、`,`
    - 字符串表示符：`"`
    - 其他：空格、制表符、换行符等空白字符

所有输入必须符合上述字符集，否则将被视为非法字符。

#### 2.2 标识符

标识符用于命名变量、标签、函数等实体。RA语言的标识符规则如下：

- 必须以字母（A-Z 或 a-z）或下划线（_）开头。
- 后续字符可以是字母、数字或下划线。
- 标识符区分大小写。
- 长度没有明确限制，但建议不超过 32 个字符以提高可读性。

**示例：**

```ra
valid_identifier    ; 以下划线连接全小写字母的标识符通常用于命名变量
_valid              ; 以下划线开头命名的标识符通常用于命名隐藏变量
variable123         ; 标识符可以包含数字区分不同实体
FunctionIdentifier  ; 单词首字母大写的标识符通常用于命名函数 
VARIABLE            ; 全大写的标识符通常用于命名常量
_123                ; 不推荐无实体含义的标识符
_                   ; 以全下划线开头的标识符通常用于命名被忽略的变量
```

**非法示例：**

```ra
123variable  ; 错误：不能以数字开头
@temp        ; 错误：包含非法字符
```

#### 2.3 指令集

RA语言的指令集（RIS）是保留字，指代 RVM 能够直接执行的操作，不能用作标识符。

RIS可分为以下几类：

- **操作指令** ：有特定执行操作的指令。
- **标志指令** ：可作为标志位参数的指令。
- **虚式指令** ：无执行操作的指令，通常用于代码占位。

以下是 RA语言的指令集表格：

<table>
    <tr>
        <th rowspan="2" colspan="2"> 类别</th>
        <th rowspan="2"> 指令</th>
        <th rowspan="2"> 描述</th>
        <th colspan="2"> 参数</th>
        <th rowspan="2"> 代码示例</th>
        <th rowspan="2"> 备注</th>
    </tr>
    <tr>
        <th> 数量</th>
        <th> 描述</th>
    </tr>
    <tr>
        <td rowspan="30"> 操作指令</td>
        <td rowspan="4"> 内存操作</td>
        <td style="text-align: center;"><code>ALLOT</code></td>
        <td> 为数据分配实体内存空间</td>
        <td style="text-align: center;"> [0, n]</td>
        <td><code>ALLOT</code>指令会分配参数个数的实体内存空间并依次以参数命名</td>
        <td>
      <pre>
ALLOT:a,b,c
ALLOT:;
</pre>
        </td>
        <td> 在RA语言中，使用变量前都需要先为其分配内存空间，函数会为其自动分配内存空间，因此无需手动分配内存</td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>DELETE</code></td>
        <td> 释放实体内存空间</td>
        <td style="text-align: center;"> [0, n]</td>
        <td><code>DELETE</code>指令会依次释放参数对应的实体内存空间，释放后不可被访问</td>
        <td>
      <pre>
ALLOT:a,b,c
DELETE:a,b,c</pre>
        </td>
        <td> RA语言虽然支持手动释放内存（如使用 <code>DELETE</code> 指令），但RVM会自动管理内存，因此不建议手动释放内存空间。
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>PUT</code></td>
        <td> 将数据写入实体内存空间</td>
        <td style="text-align: center;"> 2</td>
        <td> 参1为待操作的数据，参2为待写入的数据实体</td>
        <td>
      <pre>
ALLOT:a,b
PUT:10,a
PUT:a,b</pre>
        </td>
        <td style="text-align: center;" rowspan="2"> 值得注意的是，<code>PUT</code> 指令是将原始数据进行移动写入，而不是创建新数据副本，如需创建副本，请使用 <code>COPY</code> 指令。<code>PUT</code>在操作数值类型（Numeric）的数据时，等价于<code>COPY</code></td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>COPY</code></td>
        <td> 以原始数据创建副本，并将副本写入目标实体内存空间</td>
        <td style="text-align: center;"> 2</td>
        <td> 参1为待操作的数据，参2为待写入的数据实体</td>
        <td>
      <pre>
ALLOT:a, b
PUT:10, a
COPY:a, b</pre>
        </td>
    </tr>
    <tr>
        <td rowspan="4"> 运算操作</td>
        <td style="text-align: center;"><code>ADD</code></td>
        <td> 加法操作</td>
        <td style="text-align: center;"> 3</td>
        <td> 参1、参2为待操作的数据，参3为待写入的数据实体</td>
        <td>
      <pre>
ALLOT:a
ADD:1,1,a</pre>
        </td>
        <td rowspan="4"> 只能用于数值类型（Numeric）</td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>OPP</code></td>
        <td> 取反操作</td>
        <td style="text-align: center;"> 2</td>
        <td> 参1为待操作的数据，参2为待写入的数据实体</td>
        <td>
      <pre>
ALLOT:a
OPP:1,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>MUL</code></td>
        <td> 乘法操作</td>
        <td style="text-align: center;"> 3</td>
        <td> 参1、参2为待操作的数据，参3为待写入的数据实体</td>
        <td>
      <pre>
ALLOT:a
MUL:2,5,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>DIV</code></td>
        <td> 除法操作</td>
        <td style="text-align: center;"> 3</td>
        <td> 参1、参2为待操作的数据，参3为待写入的数据实体</td>
        <td>
      <pre>
ALLOT:a
DIV:10,2,a</pre>
        </td>
    </tr>
    <tr>
        <td> 范围操作</td>
        <td style="text-align: center;"><code>END</code></td>
        <td> 标记局部域结束位置</td>
        <td style="text-align: center;"> [0, 1]</td>
        <td> 参数可以用于标记结束的局部域名称，可省略，但为了提高代码可读性，不建议省略</td>
        <td>
      <pre>
FUNC:Func
  ... ; 函数体
END:Func</pre>
        </td>
        <td><code>END</code>指令不仅标识局部域的结束位置，同时 RVM
            还会在执行此指令时自动清理释放局部域中分配的内存空间，因此<code>END</code>属于操作指令
        </td>
    </tr>
    <tr>
        <td rowspan="5"> 函数操作</td>
        <td style="text-align: center;"><code>FUNC</code></td>
        <td> 无返回值函数定义</td>
        <td style="text-align: center;"> [1, n]</td>
        <td> 参1为函数名，参2到参n为函数参数</td>
        <td>
      <pre>
FUNC:NoneReturnFunc,a,b,c
  ... ; 函数体
END:NoneReturnFunc</pre>
        </td>
        <td style="text-align: center;" rowspan="2"> /</td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>FUNI</code></td>
        <td> 有返回值函数定义</td>
        <td style="text-align: center;"> [2, n]</td>
        <td> 参1为函数名，参2到参n为函数参数</td>
        <td>
      <pre>
FUNI:HasReturnFunc,a,b,c
  ... ; 函数体
END:HasReturnFunc</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>CALL</code></td>
        <td> 无返回值函数调用</td>
        <td style="text-align: center;"> [1, m]</td>
        <td> 参1为函数名，参2到参m为函数参数，m-1与<code>FUNC</code>指令定义的函数参数个数相等</td>
        <td>
      <pre>
FUNC:NoneReturnFunc,a,b,c
  ... ; 函数体
END:NoneReturnFunc
CALL:NoneReturnFunc,1,2,3</pre>
        </td>
        <td> 只能用于<code>FUNC</code>指令定义的函数</td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>IVOK</code></td>
        <td> 有返回值函数调用</td>
        <td style="text-align: center;"> [2, m]</td>
        <td> 参1为函数名，参m为返回值接收实体，参2到参m-1为函数参数，m-2与<code>FUNI</code>指令定义的函数参数个数相等</td>
        <td>
      <pre>
FUNI:HasReturnFunc,a,b,c
  ... ; 函数体
END:HasReturnFunc
ALLOT:return_value
IVOK:HasReturnFunc,1,2,3,return_value</pre>
        </td>
        <td> 只能用于<code>FUNI</code>指令定义的函数</td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RET</code></td>
        <td> 函数返回值操作</td>
        <td style="text-align: center;"> 1</td>
        <td> 参数为函数的返回值</td>
        <td>
      <pre>
FUNI:HasReturnFunc,a,b,c
  ... ; 函数体
  RET:10
END:HasReturnFunc</pre>
        </td>
        <td> 只能用于<code>FUNI</code>指令定义的函数</td>
    </tr>
    <tr>
        <td> 比较操作</td>
        <td style="text-align: center;"><code>CMP</code></td>
        <td> 比较数据关系操作</td>
        <td style="text-align: center;"> 3</td>
        <td> 参1、参2为待比较的数据，参3为待写入的数据实体</td>
        <td>
      <pre>
ALLOT:a,b,cmp
CMP:a,b,cmp</pre>
        </td>
        <td style="text-align: center;"> /</td>
    </tr>
    <tr>
        <td rowspan="6"> 控制操作</td>
        <td style="text-align: center;"><code>REPEAT</code></td>
        <td> 循环执行次数指令</td>
        <td style="text-align: center;"> 1</td>
        <td> 参数为循环执行的次数</td>
        <td>
      <pre>
REPEAT:10
  ... ; 循环体
END:REPEAT</pre>
        </td>
        <td style="text-align: center;" rowspan="2"> /</td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>UNTIL</code></td>
        <td> 循环条件指令，重复执行直到满足条件</td>
        <td style="text-align: center;"> 2</td>
        <td> 参1为循环条件，通常是由<code>CMP</code>获取的比较组类型的数据，参2为关系标志参数</td>
        <td>
      <pre>
ALLOT:a,b,cmp
CMP:a,b,cmp
UNTIL:cmp,RE
  ... ; 循环体
END:UNTIL</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>EXIT</code></td>
        <td> 退出局部域指令</td>
        <td style="text-align: center;"> [0, 1]</td>
        <td> 参数可以用于标记退出的局部域名称，可省略，但为了提高代码可读性，不建议省略</td>
        <td>
      <pre>
FUNC:Func
  ... ; 函数体
  EXIT:Func
END:Func</pre>
        </td>
        <td><code>EXIT</code>指令可以在局部域中的任意位置使用，当 RVM
            执行此指令时会将执行器的下一个目标指令设置为该局部域的<code>END</code>指令
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>SET</code></td>
        <td> 设置标签指令</td>
        <td style="text-align: center;"> 1</td>
        <td> 参数为标签名</td>
        <td>
      <pre>
SET:label</pre>
        </td>
        <td>
            需要注意的是<code>SET</code>指令设置的标签不是数据实体，因此设置标签不需要执行<code>ALLOT</code>分配内存空间的操作，实际上标签是局部域的属性，它的内存空间由局部域管理
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>JMP</code></td>
        <td> 无条件跳转指令</td>
        <td style="text-align: center;"> 1</td>
        <td> 参数为标签名</td>
        <td>
      <pre>
JMP:label2
SET:label1
  JMP:label3
SET:label2
  JMP:label1
SET:label3</pre>
        </td>
        <td> 因为标签是局部域的属性，因此 RVM 在运行时进入局部域时会首先执行标签的设置操作，因此<code>JMP</code>指令可以在局部域的任意位置跳转到当前局部域的标签，目前无法跨域跳转（ToDo：也许未来将实现跨域跳转）
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>JR</code></td>
        <td> 条件跳转指令</td>
        <td style="text-align: center;"> 3</td>
        <td> 参1为数据比较组，通常由<code>CMP</code>获取比较组类型的数据，参2为关系标志参数，参3为跳转标签名</td>
        <td>
      <pre>
ALLOT:a,b,cmp
CMP:a,b,cmp
JR:cmp,RE,label1
JMP:label2
SET:label1
SET:label2</pre>
        </td>
        <td>
            <code>JR</code>指令与<code>JMP</code>指令类似，只能跳转到当前局部域的标签，目前无法跨域跳转（ToDo：也许未来将实现跨域跳转）
        </td>
    </tr>
    <tr>
        <td rowspan="2"> IO操作</td>
        <td style="text-align: center;"><code>SOUT</code></td>
        <td> 输出数据到标准输出</td>
        <td style="text-align: center;"> [1, n]</td>
        <td> 参1是输出模式标志指令，参数为待输出的数据</td>
        <td>
      <pre>
SOUT:s-l,a,b,c</pre>
        </td>
        <td>
            <a href="#io">输出模式标志指令</a>包括<code>s-l</code>（单行输出）、<code>s-m</code>（多行输出）
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>SIN</code></td>
        <td> 输入数据到标准输入</td>
        <td style="text-align: center;"> [1, n]</td>
        <td> 参1是输入模式标志指令，参数为待输入的数据</td>
        <td>
      <pre>
SIN:s-l,a,b,c</pre>
        </td>
        <td>
            <a href="#io">输入模式标志指令</a>包括<code>s-l</code>（单行输入）、<code>s-m</code>（多行输入）
        </td>
    </tr>
    <tr>
        <td rowspan="2"> 引用操作</td>
        <td style="text-align: center;"><code>QOT</code></td>
        <td> 引用数据操作</td>
        <td style="text-align: center;"> 2</td>
        <td> 参1为待引用的数据，参2为待写入的数据实体，注意：两个参数都必须是数据实体</td>
        <td>
      <pre>
ALLOT:a,b
QOT:a,b</pre>
        </td>
        <td>
            <code>QOT</code>指令只能用于引用数据实体，它会自动将目标数据实体的类型更改为引用类型，并将引用目标的ID存储到目标数据实体中
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>QOT_VAL</code></td>
        <td> 修改引用数据操作</td>
        <td style="text-align: center;"> 2</td>
        <td> 参1为待修改的数据，参2为待写入的数据实体，注意：参2的类型必须为引用类型</td>
        <td>
      <pre>
ALLOT:a,b
QOT:a,b
QOT_VAL:10,b</pre>
        </td>
        <td>
            <code>QOT_VAL</code>指令只能用于修改引用数据实体，它会将目标数据存储到引用数据实体引用的数据实体中
        </td>
    </tr>
    <tr>
        <td rowspan="2"> 类型操作</td>
        <td style="text-align: center;"><code>TP_SET</code></td>
        <td> 修改数据实体类型操作</td>
        <td style="text-align: center;"> 2</td>
        <td> 参1为类型标志指令，参2为待修改的数据实体</td>
        <td>
      <pre>
ALLOT:a,b
TP_SET:tp-int,a</pre>
        </td>
        <td>
            <a href="#customType">类型标志指令</a>包括<code>tp-int</code>（整型）、<code>tp-float</code>（浮点型）、<code>tp-str</code>（字符串）、<code>tp-bool</code>（布尔型）、<code>tp-char</code>（字符类型）、<code>tp-null</code>（空类型），<code>ALLOT</code>指令分配的数据实体类型默认为<code>tp-null</code>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>TP_GET</code></td>
        <td> 获取数据实体类型操作</td>
        <td style="text-align: center;"> 2</td>
        <td> 参1为待获取的数据实体，参2为类型数据待写入的数据实体</td>
        <td>
      <pre>
ALLOT:a,b
TP_GET:a,b</pre>
        </td>
        <td>
            <code>TP_GET</code>指令可以获取数据和实体的类型，它会将目标数据实体的类型存储到目标数据实体中
        </td>
    </tr>
    <tr>
        <td rowspan="4"> 作用域操作</td>
        <td style="text-align: center;"><code>SP_GET</code></td>
        <td> 获取作用域ID操作</td>
        <td style="text-align: center;"> 1</td>
        <td> 参数为作用域数据待写入的数据实体</td>
        <td>
      <pre>
ALLOT:a
SP_GET:a</pre>
        </td>
        <td>
            <code>SP_GET</code>指令可以获取作用域ID，它会将当前作用域的ID存储到目标数据实体中
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>SP_SET</code></td>
        <td> 修改当前作用域操作</td>
        <td style="text-align: center;"> 1</td>
        <td> 参数为存储作用域ID的数据实体</td>
        <td>
       <pre>
ALLOT:a
SP_SET:a</pre>
        </td>
        <td>
            <code>SP_SET</code>指令可以修改当前作用域，它会将当前的作用域修改为指定ID的作用域
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>SP_NEW</code></td>
        <td> 新建局部作用域操作</td>
        <td style="text-align: center;"> 1</td>
        <td> 参数为数据实体，用于存储新建作用域的ID</td>
        <td>
       <pre>
ALLOT:a
SP_NEW:a</pre>
        </td>
        <td>
            <code>SP_NEW</code>指令可以新建局部作用域，它会将新建作用域的ID存储到目标数据实体中
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>SP_DEL</code></td>
        <td> 删除局部作用域操作</td>
        <td style="text-align: center;"> 1</td>
        <td> 参数为存储作用域ID的数据实体</td>
        <td>
       <pre>
ALLOT:a
SP_GET:a
SP_DEL:a</pre>
        </td>
        <td>
            <code>SP_DEL</code>指令可以删除局部作用域，它会将目标ID的作用域删除，不推荐使用此指令操作其他可创建局部域指令创建的局部域（如：<code>FUNC</code>、<code>REPEAT</code>等），因为 RVM 会自动清理其他指令创建的局部域
        </td>
    </tr>
    <tr>
        <td rowspan="18"> 标志指令</td>
        <td rowspan="4"> IO标志<a id="io"></a></td>
        <td style="text-align: center;"><code>s-l</code></td>
        <td> 单行输出模式</td>
        <td style="text-align: center;" rowspan="4"> 0</td>
        <td style="text-align: center;" rowspan="4"> /</td>
        <td>
      <pre>
SOUT:s-l,...</pre>
        </td>
        <td>
            <code>s-l</code>表示单行输出模式，即输出数据时，后面的数据都会直接与前面的数据进行拼接输出
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>s-m</code></td>
        <td> 多行输出模式</td>
        <td>
        <pre>
SOUT:s-m,...</pre>
        </td>
        <td>
            <code>s-m</code> 表示多行输出模式，即输出数据时，后面的数据会以换行符与前面的数据进行拼接输出
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>s-f</code></td>
        <td> 刷新输出缓存标志</td>
        <td>
        <pre>
SOUT:s-l,...,s-f,...</pre>
        </td>
        <td>
            <code>s-f</code> 是用于立即刷新输出缓存的标志。在RVM中，数据首先被存入输出缓存，待缓存达到一定量后才输出到终端。使用 <code>s-f</code> 可以强制将当前缓存中的数据立刻输出，而不必等待缓存满载。
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>s-n</code></td>
        <td> 换行标志</td>
        <td>
        <pre>
SOUT:s-l,...,s-n,...</pre>
        </td>
        <td>
            <code>s-n</code>表示换行标志，即输出数据时，在标志位置输出换行符
        </td>
    </tr>
    <tr>
        <td rowspan="6"> 类型标志<a id="customType"></a></td>
        <td style="text-align: center;"><code>tp-int</code></td>
        <td> 整型标志</td>
        <td style="text-align: center;" rowspan="6"> 0</td>
        <td style="text-align: center;" rowspan="6"> /</td>
        <td>
        <pre>
ALLOT:a
TP_SET:tp-int,a</pre>
        </td>
        <td rowspan="6">
            类型标志表示其对应的数据类型，它内部存储的值是其对应类型的ID字符串，因此它本身的数据类型是字符串类型，可以使用<code>TP_GET</code>查看其类型
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>tp-float</code></td>
        <td> 浮点型标志</td>
        <td>
        <pre>
ALLOT:a
TP_SET:tp-float,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>tp-str</code></td>
        <td> 字符串标志</td>
        <td>
        <pre>
ALLOT:a
TP_SET:tp-str,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>tp-bool</code></td>
        <td> 布尔标志</td>
        <td>
        <pre>
ALLOT:a
TP_SET:tp-bool,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>tp-null</code></td>
        <td> 空值标志</td>
        <td>
        <pre>
ALLOT:a
TP_SET:tp-null,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>tp-char</code></td>
        <td> 字符标志</td>
        <td>
        <pre>
ALLOT:a
TP_SET:tp-char,a</pre>
        </td>
    </tr>
    <tr>
        <td rowspan="8"> 关系标志<a id="relationship"></a> </td>
        <td style="text-align: center;"><code>RL</code></td>
        <td> 大于关系标志 </td>
        <td style="text-align: center;" rowspan="8"> 0</td>
        <td style="text-align: center;" rowspan="8"> /</td>
        <td>
        <pre>
ALLOT:a
TP_GET:RL,a</pre>
        </td>
        <td style="text-align: center;" rowspan="8"> /</td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RLE</code><a id="relation-RLE"></a></td>
        <td> 大于等于关系标志 </td>
        <td>
        <pre>
ALLOT:a
TP_GET:RLE,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RNE</code><a id="relation-RNE"></a></td>
        <td> 不等于关系标志 </td>
        <td>
        <pre>
ALLOT:a
TP_GET:RNE,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RE</code><a id="relation-RE"></a></td>
        <td> 等于关系标志 </td>
        <td>
        <pre>
ALLOT:a
TP_GET:RE,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RSE</code><a id="relation-RSE"></a></td>
        <td> 小于等于关系标志 </td>
        <td>
        <pre>
ALLOT:a
TP_GET:RSE,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RS</code><a id="relation-RS"></a></td>
        <td> 小于关系标志 </td>
        <td>
        <pre>
ALLOT:a
TP_GET:RS,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RT</code><a id="relation-RT"></a></td>
        <td> 真标志 </td>
        <td>
        <pre>
ALLOT:a
TP_GET:RT,a</pre>
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>RF</code><a id="relation-RF"></a></td>
        <td> 假标志 </td>
        <td>
        <pre>
ALLOT:a
TP_GET:RF,a</pre>
        </td>
    </tr>
    <tr>
        <td rowspan="2"> 虚式指令</td>
        <td rowspan="2"> </td>
        <td style="text-align: center;"><code>PASS</code></td>
        <td> 占位符指令</td>
        <td style="text-align: center;"> [0, n]</td>
        <td style="text-align: center;"> /</td>
        <td>
        <pre>
PASS:;</pre>
        </td>
        <td>
            占位符指令，用于占位，当指令执行时，RVM 会忽略此指令，直接执行后续指令
        </td>
    </tr>
    <tr>
        <td style="text-align: center;"><code>UNKNOWN</code></td>
        <td> 未知指令</td>
        <td style="text-align: center;"> [0, n]</td>
        <td style="text-align: center;"> /</td>
        <td>
        <pre>
UNKNOWN:;</pre>
        </td>
        <td>
            用于 RVM 标识解析到的未知指令
        </td>
    </tr>
</table>

#### 2.4 常量
RA语言支持以下类型的常量：

- **数值类型（Numeric）**
  - **整数常量** ： 如 `123`, `-456`
  - **浮点常量** ： 如 `3.14`, `-0.001`
  - **布尔常量** ： 只有两个值：`true` 和 `false`。
- **可迭代类型（Iterable）**
  - **字符串常量** ： 使用双引号括住，如 `"Hello, World!"`。
  支持转义字符，如 `\n`（换行）、`\t`（制表符）、`\"`（双引号）。
- **空类型（Null）**
  - **空值常量** ： 只有一个值：`null`。

**示例：**
```Ra
ALLOT:a,b,c,d,e
PUT:10,a              ; 整数常量
PUT:3.14,b            ; 浮点常量
PUT:"Hello, World!",c ; 字符串常量
PUT:true,d            ; 布尔常量
PUT:null,e            ; 空值常量
```

#### 2.5 关系
RA语言支持以下类型的关系：

- **大于关系**：`RL`
- **大于等于关系**：`RLE`
- **不等于关系**：`RNE`
- **等于关系**：`RE`
- **小于等于关系**：`RSE`
- **小于关系**：`RS`
- **真关系**：`RT`
- **假关系**：`RF`

详细描述见 [关系标志](#relationship)。

#### 2.6 注释
RA语言支持单行注释和多行注释：
- **单行注释** ：

  以分号（;）开头，直到行尾的所有内容均被视为注释。

  **示例：**
  ```Ra
  ; 这是一个单行注释
  ALLOT:a ; 为变量a分配内存空间
  PUT:10,a ; 将10存储到数据实体a
  ```
- **多行注释** ：
  
  多行注释可以使用`~`配合`;`完成。

  **示例：**
  ```Ra
  ; 这是一个多行注释
  ~ 这是多行注释的第二行
  ~ 这是多行注释的第三行
  ALLOT:a ; 为变量a分配内存空间
  ```
  
**注：**_`~`是连接符，RVM 在解析代码时会将开头带有`~`的一行拼接到它的上一行，因此上面展示的多行注释实际上也只是单行注释。_

#### 2.7 分隔符
RA语言使用以下分隔符来组织代码结构：

- **分号（;）** ：用于结束一条指令或用于注释。
- **逗号（,）** ：用于分隔参数或操作数。
- **冒号（:）** ：用于分隔指令和参数。

**示例：**
```Ra
ALLOT:a,b,c,d,e;    分号可以用于结束一条指令或用于注释
PUT:10,a;           逗号用于分隔参数或操作数
SOUT:s-l,a,b,c,d,e; 冒号用于分隔指令和参数
```

#### 2.8 空白字符
空白字符（空格、制表符、换行符）在 RA语言中被忽略。

**示例：**
```Ra
ALLOT:a,b,c; 没有空白字符
ALLOT : d, e, f; 有空白字符，但是解析时会忽略
```

### 三、语法结构

RA语言的语法结构简单且统一，采用助记符（mnemonic）形式表示指令，具体格式如下：

```Ra
[RI] : <[Arg], [Arg], ...> (;)
```

#### 3.1 指令格式解析
以下是该语法结构中各部分的详细说明：
- [**RI**] ：
  - 表示指令的操作码（Opcode），即指令的核心部分。
  - 它是 RA语言中的一个有效指令名称，例如 `ALLOT`、`PUT`、`SOUT`、... 等。
  - 指令通常是由字母与下划线组成，不区分大小写。
  - 必须符合词法结构中定义的指令集。

- **:** ：
  - 分隔操作码与参数列表的符号。
  - 是语法结构中固定的分隔符，用于明确指令与其参数的关系。

- <[**Arg**], [**Arg**], ...> ：
  - 参数列表，包含指令所需的操作数。
  - 参数之间用逗号（`,`）分隔。
  - 参数可以是数据、数据实体、关键字等，具体类型取决于指令的要求。
  - 参数的数量和类型由指令的定义决定。
  
  **示例：**
  ```
  ALLOT:a,b; 两个参数，分别是数据实体名称a和b。
  ADD:10,20,a; 三个参数，分别是目标存储数据实体和两个数据。
  ```

- (`;`) ：
  - 可选的分号`;`，用于结束一条指令。
  - 如果省略分号，则默认以换行符作为指令的结束标志。 
  - 分号后可以添加注释，用于说明指令的功能或用途。

#### 3.2 示例代码
以下是一些符合 RA语言语法结构的示例代码：

1. **数据传输指令** ：

   ```Ra
   ALLOT:a,b
   PUT:10,a
   PUT:a,b
   ```

2. **算数运算指令** ：

   ```Ra
   ALLOT:a,b,c,d
   ; 计算10+20，结果存储到数据实体a中
   ADD:10,20,a
   ; 计算30-a，结果存储到数据实体b中
   OPP:a,a; 取反，实现减法运算
   ADD:30,a,b
   ; 计算2*5，结果存储到数据实体c中
   MUL:2,5,c
   ; 计算c/2，结果存储到数据实体d中
   DIV:c,2,d
   ```

3. **控制操作指令** ：

    ```Ra
    JMP:label3
    SET:label1
        JMP:label2
    SET:label2
        JMP:label_end
    SET:label3
        JMP:label1
    SET:label_end
    ```

4. **系统操作指令** ：

    ```Ra
    PASS:; 空操作，不执行任何操作
    ```

#### 3.3 参数类型
RA语言支持多种类型的参数，具体包括以下几种：

- **临时数据** ：

  形如`10`、`12.5`、`"Hello world!"`这种直接以值呈现的称为**临时数据**。RVM 的内存中自动管理着这些临时数据，包括内存的分配与回收。它们的特定是只能在当前指令执行期间有效，执行结束后会被自动释放。

  - **数字类型**
    - **整数**
    - **浮点数**
    - **布尔值**
    
    详情请见 [数值类型](#24-常量)。
  - **字符串类型**
    
    详情请见 [字符串类型](#24-常量)。

- **数据实体** ：

  **数据实体**是 RA 语言中的核心概念，用于管理和操作数据。数据实体是通过 `ALLOT` 指令创建的，当创建一个数据实体，RVM 会为这个数据实体分配一段内存空间，并通过ID映射机制将指定的标识符与分配的内存空间ID进行映射，以实现临时数据的存储和访问。当临时数据被存入数据实体后，RVM 将不会在指令的执行周期结束后自动释放这个数据，直到程序退出创建此数据实体的局部域时，RVM 会自动释放这个数据实体所占用的内存空间以避免内存泄漏。
  
  - **标识符**
    
    通过 `ALLOT` 指令创建的数据实体的标识符，即数据实体的名称。
  - **关键字**

    RVM 自带的数据实体关键字，包括：
    - `SN` ：用于手动存储函数返回值的数据实体。
    - `SR` ：用于自动存储函数返回值的数据实体。
    - `SE` ：用于自动存储错误信息的数据实体。
    - `SS` ：用于手动存储局部域信息的数据实体。

#### 3.5 语法规则
为了确保 RA语言的语法一致性，需遵守以下规则：

1. **指令顺序** ：
    
    指令按顺序执行，除非遇到控制转移指令（如 `JMP`、`CALL`、`IVOK`等）。
    
    **例如** ：
    ```Ra
    ALLOT:a
    ADD:1,2,a       ; 在 ALLOT 指令执行之后，ADD 指令才会被执行。
    ```
2. **参数数量匹配** ：
   
   每条指令的参数数量必须符合其定义要求。
   
   **例如** ：
   
   ```Ra
   ALLOT:a
   PUT:10,a        ; 正确：PUT 指令需要两个参数
   PUT:10          ; 错误：缺少第二个参数
   ```

3. **参数类型匹配** ：

   参数类型必须与指令的要求一致。

   **例如** ：

   ```Ra
   ALLOT:a,b,c     ; 正确：ALLOT 指令需要三个标识符参数
   ALLOT:1,2,3     ; 错误：ALLOT不支持临时类型的参数
   ```

4. **分号的使用** ： 
   
   分号是可选的，建议仅在注释中使用，其他处省略。
   **示例** ：
   ```Ra
   ALLOT:a,b,c;   ; 合法
   ALLOT:a,b,c    ; 更推荐
   ```

### 四、标准库

### 五、运行时

### 六、错误处理

### 七、兼容性与限制

### 八、示例代码
