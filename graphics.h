#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <QJsonObject>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsLineItem> // 确保包含
#include "engine.h"

// --- 前向声明 ---
class Wire;
class WireItem;

// =============================================================
// == 类: ComponentItem
// =============================================================
class ComponentItem : public QGraphicsItem {
public:
    ComponentItem(Component* data);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    Component* component() const;
    Pin* getPinAt(const QPointF& localPos);
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
private:
    Component* m_componentData;
};

// =============================================================
// == 类: WireItem
// =============================================================
class WireItem : public QGraphicsLineItem {
public:
    WireItem(Wire* data);
    void updatePosition();
    Wire* wireData() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
private:
    Wire* m_wireData;
};

// =============================================================
// == 类: GraphicsScene (精确修正以兼容C同学)
// =============================================================
class GraphicsScene : public QGraphicsScene {
    Q_OBJECT
public:
    // 【修正】使用我们最初的枚举名
    enum Mode { Idle, AddingComponent };
    void rebuildSceneFromEngine();
    GraphicsScene(Engine* engine, QObject* parent = nullptr);

    // 【核心修正】恢复为两个独立的函数，以匹配 mainwindow.cpp 的调用
    void setMode(Mode mode);
    void setComponentTypeToAdd(ComponentType type);
    Engine* getEngine() const;

    void setJsonForNextComponent(const QJsonObject& json);
signals:
    void componentAdded();
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
private:
    Engine* m_engine;
    QGraphicsLineItem* m_tempLine;
    Pin* m_startPin;

    // 【修正】恢复为我们最初的成员变量
    Mode m_currentMode;
    ComponentType m_typeToAdd;

    // 【新增】用于临时存储下一个要创建的封装元件的 JSON 定义
    QJsonObject m_jsonToAdd;
};
inline Engine* GraphicsScene::getEngine() const {
        return m_engine;
    }
#endif // GRAPHICS_H
