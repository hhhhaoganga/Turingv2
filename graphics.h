#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <QJsonObject>        // 封装元件添加时传递内部电路定义
#include <QGraphicsScene>     // 自定义场景基类
#include <QGraphicsItem>      // 自定义组件图形项基类
#include <QGraphicsLineItem>  // 导线图形项
#include "engine.h"          // 后端数据结构与引擎接口

/** 前向声明：避免不必要的头文件耦合 */
class Wire;
class WireItem;

// =============================================================
// == 类: ComponentItem
// =============================================================
/**
 * @brief 组件对应的图形项，负责绘制组件外观与引脚。
 * @details 该类与后端 `Component` 双向绑定：位置改变会同步到数据层。
 */
class ComponentItem : public QGraphicsItem {
public:
    /** 通过后端组件数据构造图形项，并建立绑定 */
    ComponentItem(Component* data);
    /** 包围盒，用于视图刷新/选择判定 */
    QRectF boundingRect() const override;
    /** 绘制组件主体、文字、引脚及选中高亮 */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    /** 获取后端组件数据指针 */
    Component* component() const;
    /** 根据局部坐标命中检测，返回被点中的引脚 */
    Pin* getPinAt(const QPointF& localPos);
protected:
    /** 捕获位置变化，将几何同步回后端数据层 */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
private:
    /** 后端组件数据（非拥有） */
    Component* m_componentData;
};

// =============================================================
// == 类: WireItem
// =============================================================
/**
 * @brief 导线对应的图形项，根据导线状态上色并保持端点跟随。
 */
class WireItem : public QGraphicsLineItem {
public:
    /** 通过后端导线数据构造 */
    WireItem(Wire* data);
    /** 根据后端引脚的场景坐标更新自身几何 */
    void updatePosition();
    /** 获取后端导线数据 */
    Wire* wireData() const;
    /** 自定义绘制：根据状态选择颜色 */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
private:
    /** 后端导线数据（非拥有） */
    Wire* m_wireData;
};

// =============================================================
// == 类: GraphicsScene (精确修正以兼容C同学)
// =============================================================
/**
 * @brief 自定义场景：处理元件添加、连线绘制与删除等交互逻辑。
 */
class GraphicsScene : public QGraphicsScene {
    Q_OBJECT
public:
    // 【修正】使用我们最初的枚举名
    enum Mode { Idle, AddingComponent };
    /** 根据引擎状态重建整张场景（打开文件后使用） */
    void rebuildSceneFromEngine();
    /** 通过后端引擎构造场景 */
    GraphicsScene(Engine* engine, QObject* parent = nullptr);

    // 【核心修正】恢复为两个独立的函数，以匹配 mainwindow.cpp 的调用
    /** 设置交互模式 */
    void setMode(Mode mode);
    /** 设置待添加的组件类型 */
    void setComponentTypeToAdd(ComponentType type);
    /** 获取绑定的后端引擎 */
    Engine* getEngine() const;

    /** 设置下一个封装元件所用的内部电路 JSON */
    void setJsonForNextComponent(const QJsonObject& json);
    /** 设置下一个封装元件的显示名称 */
    void setNameForNextComponent(const QString& name);
signals:
    /** 当一个组件被放置到场景中时发出 */
    void componentAdded();
protected:
    /** 处理放置组件、开始连线、右键删除等按下事件 */
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    /** 处理临时连线拖拽与导线位置更新 */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    /** 处理完成连线或清理临时连线 */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
private:
    /** 绑定的后端引擎（非拥有） */
    Engine* m_engine;
    /** 临时绘制的虚线用于拖拽连线 */
    QGraphicsLineItem* m_tempLine;
    /** 连线的起始引脚（拖拽期间缓存） */
    Pin* m_startPin;

    // 【修正】恢复为我们最初的成员变量
    /** 当前交互模式 */
    Mode m_currentMode;
    /** 待添加的组件类型 */
    ComponentType m_typeToAdd;

    // 【新增】用于临时存储下一个要创建的封装元件的 JSON 定义
    /** 待添加封装元件的内部电路 JSON */
    QJsonObject m_jsonToAdd;

    /** 待添加封装元件的名称 */
    QString m_nameToAdd;
};
inline Engine* GraphicsScene::getEngine() const {
        return m_engine;
    }
#endif // GRAPHICS_H
