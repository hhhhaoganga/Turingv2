# Turingv2 电路编辑与仿真

> 面向用户的入门请参阅：`USER_README.md`

---

## 开发者文档

本项目基于 Qt/C++（Qt 6）实现，包含电路元件的图形编辑、连线与逻辑仿真，并支持将任意电路封装为“自定义元件”。本文档面向开发者，简要说明模块职责、调用关系与扩展要点。

## 目录结构
- `engine.h/.cpp`: 后端核心引擎与数据模型。
  - `Component`/`Pin`/`Wire` 基础概念
  - `Engine` 负责创建/删除/仿真/序列化
  - `EncapsulatedComponent` 支持内部电路与引脚映射
- `graphics.h/.cpp`: 前端图形表示与交互。
  - `ComponentItem`/`WireItem` 图形项
  - `GraphicsScene` 自定义场景，处理添加、连线、删除等交互
- `mainwindow.h/.cpp` 与 `mainwindow.ui`: 主窗体与菜单/工具栏/多标签逻辑。
- `CMakeLists.txt`: 构建配置。

## 核心职责
- 引擎（engine）
  - 管理组件与导线的生命周期（创建、注册、删除、清理）。
  - 进行逻辑仿真：按“输入源→导线传播→其他组件评估”的顺序迭代，直至稳定或达上限迭代次数。
  - JSON 序列化/反序列化：保存/加载完整电路（组件与导线）。
  - 封装元件：内部持有独立 `Engine`，通过引脚映射与外部交互。
- 图形层（graphics）
  - `ComponentItem` 绘制组件主体、文字、引脚，选中高亮；位置变化回写到数据层。
  - `WireItem` 以导线状态上色（绿/红），自动跟随端点引脚位置。
  - `GraphicsScene` 交互集中地：
    - 左键：放置元件、开始/完成连线、翻转输入；
    - 右键：删除导线或组件（并同步数据层），随后触发仿真与刷新；
    - 从引擎重建整张场景（打开文件后）。
- 主窗口（mainwindow）
  - 多标签页，每个标签页拥有独立的 `Engine` 和 `GraphicsScene`。
  - 文件菜单：新建标签、打开（到新标签）、保存当前标签。
  - 自定义元件：将当前电路保存到 `components/` 为 JSON，工具栏自动扫描并生成按钮。

## 调用关系（高层）
- `MainWindow` 调度 `GraphicsScene` 与 `Engine`：
  - 用户选择“添加元件” → `GraphicsScene::setComponentTypeToAdd` → 鼠标左键放置 → `Engine::createComponent` → `ComponentItem` 加入场景。
  - 用户拖拽连线 → `Engine::createWire` → `WireItem` 加入场景。
  - 用户右键删除 → `Engine::deleteWire/deleteComponent` → 场景移除对应图形项。
  - 任何结构变化后 → `Engine::simulate` → `WireItem::paint`/`ComponentItem::paint` 用状态更新展示。

## 扩展点
- 新增逻辑门：
  - 在 `ComponentType` 中加入枚举项；
  - 新增对应子类（构造设置引脚数，重写 `evaluate`）；
  - 在 `Engine::createComponent` 的 `switch` 中实例化；
  - 在 `ComponentItem::paint` 中增加显示文本（可选）。
- 自定义外观或交互：修改 `ComponentItem::paint`/`WireItem::paint` 或 `GraphicsScene` 相关事件处理。
- 文件格式兼容：在 `Engine::saveCircuitToJson/loadCircuitFromJson` 中演进字段（注意兼容旧字段或提供迁移）。

## 重要设计说明
- 仿真稳定性：每轮仿真开始前将“非源头（非 Input）组件的输入引脚”清零，以避免删除导线后残留状态导致的错误。
- 封装元件引脚对齐：内部 `Input/Output` 组件按其 Y 坐标排序后建立映射，确保外部引脚顺序稳定。
- 组件ID：保存时以内存地址写入，仅用于文件内部的连线映射；打开时使用临时 `idMap` 重新关联。

## 构建与运行
- 使用 Qt Creator 直接打开项目或运行 CMake 生成工程；
- 运行后默认创建一个空白标签页，可从工具栏添加元件并连线；
- 菜单/工具栏支持保存/打开电路，自定义元件保存在可执行目录的 `components/` 下。

## 代码风格与注释
- 保持 Doxygen 风格注释（`@brief/@param/@return`），私有与内部函数也补充简明说明；
- 文件顶部简述包含的 `#include` 用途；
- 不改变项目既有缩进/格式风格，注释为轻量不冗长优先。
