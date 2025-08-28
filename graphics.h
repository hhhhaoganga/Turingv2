#ifndef GRAPHICS_H
#define GRAPHICS_H

/**
 * @file graphics.h
 * @brief 【B同学负责】定义项目的所有图形和交互相关的类。
 * @details 定义了元件在画布上的“图形化身”，以及处理所有用户鼠标操作的“画布”类。
 */

#include <QGraphicsScene>
#include <QGraphicsItem>
#include "engine.h" // 需要包含engine.h来使用公共枚举ComponentType

// --- 前向声明 ---
class Component;
class Engine;

// =============================================================
// == 类: ComponentItem
// == 目的: 任何电路元件在画布上的图形表示。
// =============================================================
class ComponentItem : public QGraphicsItem {
public:
    ComponentItem(Component* data);
    ~ComponentItem();
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    virtual QPointF getPinScenePosition(Pin::PinType pinType, int index) const;//根据B同学的说法加入了这行代码
private:
    Component* m_data;
};

// =============================================================
// == 类: GraphicsScene
// == 目的: 整个项目的交互核心，用户的“画布”。
// =============================================================
class GraphicsScene : public QGraphicsScene {
    Q_OBJECT
public:
    enum Mode { Idle, AddingComponent };

    /**
     * @brief 构造函数。
     * @param engine - 【输入】一个指向后台引擎的指针，用于命令引擎进行数据操作。
     * @param parent - 【输入】父对象。
     */
    GraphicsScene(Engine* engine, QObject* parent = nullptr);

    /**
     * @brief 设置场景的当前操作模式。由C同学的MainWindow调用。
     * @param mode - 【输入】要切换到的模式 (Idle或AddingComponent)。
     */
    void setMode(Mode mode);

    /**
     * @brief 告知场景接下来要添加的元件类型。由C同学的MainWindow调用。
     * @param type - 【输入】要添加的元件类型。
     *               【合法值】: 必须是 engine.h 中定义的 ComponentType 枚举里的一个值。
     */
    void setComponentTypeToAdd(ComponentType type);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    // ... 其他事件声明 ...
private:
    Engine* m_engine; // 指向A同学引擎的“遥控器”
    Mode m_currentMode;
    ComponentType m_typeToAdd;
};

#endif // GRAPHICS_H
