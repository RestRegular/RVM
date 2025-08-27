### CustomType (自定义类型)
- #### 概述
    CustomType 表示一种自定义的数据类型，可以包含以下元素：

  - 类型字段 (tpFields) - 属于类型本身的字段
  - 实例字段 (instFields) - 属于实例的字段
  - 方法字段 (methodFields) - 分为实例方法和类型方法

- #### 主要特性
  - 继承机制：每个 CustomType 可以有一个父类型 (parentType)

  - 字段管理：
  
    - 只能通过 CustomType 添加字段 
    - 类型字段只能通过 CustomType 设置 
    - 实例字段只能通过 CustomInst 设置 
    - 类型检查：提供 checkBelongTo() 方法检查类型关系

- #### 重要方法
  - `addTpField()/addInstField()` - 添加类型/实例字段
  - `setTpField()` - 设置类型字段值
  - `getTpField()` - 获取类型字段值
  - `hasField()/hasInstField()` - 检查字段存在性

### CustomInst (自定义类型实例)
- #### 概述
    CustomInst 表示 CustomType 的实例，具有以下特点：

  - 不能添加新字段
  - 只能设置实例字段和调用实例方法
  - 可以访问实例字段和类型字段

- #### 主要特性
  - 字段存储：使用嵌套映射存储实例字段 (instFields)
  - 类型关联：每个实例关联一个 CustomType (customType)
  - 类型转换：支持 derivedToChildType() 方法转换为子类型

- #### 重要方法
  - `getField()` - 获取字段值（可指定特定类型）
  - `setField()` - 设置实例字段值
  - `hasField()` - 检查字段存在性

### 使用关系
1. 首先定义 CustomType，添加所需的字段
2. 然后创建 CustomInst 实例
3. 通过 CustomInst 操作实例字段
4. 通过关联的 CustomType 操作类型字段