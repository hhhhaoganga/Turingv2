#include "graphics.h"
#include "engine.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QStyleOptionGraphicsItem>

// ===============================================
// === ComponentItem 实现
// ===============================================

ComponentItem::ComponentItem(Component* data) : m_componentData(data) {
    setPos(data->position());
    setFlags(ItemIsMovable | ItemIsSelectable);
    data->setGraphicsItem(this);
}

QRectF ComponentItem::boundingRect() const {
    return QRectF(-10, -10, 120, 70); // 扩展边界以容纳引脚
}

void ComponentItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF bodyRect(0, 0, 100, 50);
    painter->setRenderHint(QPainter::Antialiasing);

    // 绘制主体
    painter->setPen(QPen(Qt::darkGray, 2));
    painter->setBrush(QColor("#f0f0f0"));
    painter->drawRoundedRect(bodyRect, 5, 5);

    // 绘制选中状态
    if (option->state & QStyle::State_Selected) {
        // 循环画5层，每一层都更粗、更透明
        for (int i = 0; i < 5; ++i) {
            QColor glowColor = QColor("#66ccff");
            // 透明度从 80 递减到 0
            glowColor.setAlpha(80 - i * 16);

            painter->setPen(QPen(glowColor, i * 2)); // 画笔越来越粗
            painter->setBrush(Qt::NoBrush);
            painter->drawRoundedRect(bodyRect, 5, 5);
        }
    }

    // 准备文字和状态
    painter->setPen(Qt::black);
    QString text;
    bool state = false;

    // 根据类型进行特殊绘制和文本设置
    switch (m_componentData->type()) {
    case ComponentType::Input:
        text = "输入";
        if (!m_componentData->outputPins().isEmpty()) {
            state = m_componentData->outputPins()[0]->getState();
            painter->setBrush(state ? QColor("#4CAF50") : QColor("#F44336"));
            painter->setPen(Qt::NoPen);
            painter->drawRect(0, 0, 50, 50);
            painter->setPen(Qt::white);
            painter->setFont(QFont("Arial", 10, QFont::Bold));
            painter->drawText(QRectF(0,0,50,50), Qt::AlignCenter, state ? "ON" : "OFF");
            painter->setFont(QFont()); // 恢复
            painter->setPen(Qt::black);
        }
        painter->drawText(QRectF(50,0,50,50), Qt::AlignCenter, text);
        break;

    case ComponentType::Output:
        text = "输出";
        if (!m_componentData->inputPins().isEmpty()) {
            state = m_componentData->inputPins()[0]->getState();
            painter->setBrush(state ? QColor("#FFEB3B") : QColor("#616161"));
            painter->setPen(Qt::NoPen);
            painter->drawRect(0, 0, 50, 50);
        }
        painter->setPen(Qt::black);
        painter->drawText(QRectF(50,0,50,50), Qt::AlignCenter, text);
        break;

    case ComponentType::And: text = "与门"; break;
    case ComponentType::Or: text = "或门"; break;
    case ComponentType::Not: text = "非门"; break;
    case ComponentType::Nand: text = "与非门"; break;
    case ComponentType::Nor: text = "或非门"; break;
    case ComponentType::Xor: text = "异或门"; break;
    case ComponentType::Xnor: text = "同或门"; break;
    }

    // 为普通逻辑门统一绘制文字
    if (m_componentData->type() >= ComponentType::And) {
        painter->drawText(bodyRect, Qt::AlignCenter, text);
    }

    // 绘制引脚
    int numInputs = m_componentData->inputPins().size();
    for (int i = 0; i < numInputs; ++i) {
        qreal yPos = bodyRect.height() * (i + 1) / (numInputs + 1);
        painter->setBrush(m_componentData->inputPins()[i]->getState() ? Qt::green : Qt::darkGray);
        painter->drawEllipse(QPointF(0, yPos), 4, 4);
    }

    int numOutputs = m_componentData->outputPins().size();
    for (int i = 0; i < numOutputs; ++i) {
        qreal yPos = bodyRect.height() * (i + 1) / (numOutputs + 1);
        painter->setBrush(m_componentData->outputPins()[i]->getState() ? Qt::green : Qt::darkGray);
        painter->drawEllipse(QPointF(100, yPos), 4, 4);
    }
}

Component* ComponentItem::component() const {
    return m_componentData;
}

Pin* ComponentItem::getPinAt(const QPointF &localPos) {
    const qreal pinRadius = 4.0;
    const qreal clickableRadius = pinRadius * 2;

    int numInputs = m_componentData->inputPins().size();
    for(int i = 0; i < numInputs; ++i) {
        qreal yPos = 50.0 * (i + 1) / (numInputs + 1);
        QRectF pinArea(QPointF(-clickableRadius/2, yPos - clickableRadius/2), QSizeF(clickableRadius, clickableRadius));
        if (pinArea.contains(localPos)) {
            return m_componentData->inputPins()[i];
        }
    }

    int numOutputs = m_componentData->outputPins().size();
    for(int i = 0; i < numOutputs; ++i) {
        qreal yPos = 50.0 * (i + 1) / (numOutputs + 1);
        QRectF pinArea(QPointF(100 - clickableRadius/2, yPos - clickableRadius/2), QSizeF(clickableRadius, clickableRadius));
        if (pinArea.contains(localPos)) {
            return m_componentData->outputPins()[i];
        }
    }
    return nullptr;
}

QVariant ComponentItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionHasChanged && m_componentData) {
        m_componentData->setPosition(this->pos());
    }
    return QGraphicsItem::itemChange(change, value);
}

// ===============================================
// === WireItem 实现
// ===============================================

WireItem::WireItem(Wire* data) : m_wireData(data) {
    setPen(QPen(Qt::black, 2));
    setZValue(-1); // 确保导线在元件下面
}

Wire* WireItem::wireData() const {
    return m_wireData;
}

void WireItem::updatePosition() {
    if (m_wireData) {
        setLine(QLineF(m_wireData->startPin()->getScenePos(), m_wireData->endPin()->getScenePos()));
    }
}

void WireItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    if (!m_wireData) return;
    bool state = m_wireData->getState();
    QPen customPen;
    customPen.setColor(state ? Qt::green : Qt::red);
    customPen.setWidth(2);
    painter->setPen(customPen);
    painter->drawLine(line());
}

// ===============================================
// === GraphicsScene 实现
// ===============================================

GraphicsScene::GraphicsScene(Engine* engine, QObject* parent)
    : QGraphicsScene(parent), m_engine(engine), m_tempLine(nullptr), m_startPin(nullptr), m_currentMode(Idle)
{}

void GraphicsScene::setMode(Mode mode) {
    m_currentMode = mode;
}

void GraphicsScene::setComponentTypeToAdd(ComponentType type) {
    m_typeToAdd = type;
}

// in graphics.cpp

void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // =============================================================
    // == 【核心修正】恢复右键删除逻辑
    // =============================================================
    if (event->button() == Qt::RightButton) {
        QGraphicsItem* itemToDelete = itemAt(event->scenePos(), QTransform());
        if (!itemToDelete) return; // 如果没有点中任何东西，直接返回

        // --- 情况一：删除导线 ---
        if (auto wireItem = qgraphicsitem_cast<WireItem*>(itemToDelete)) {
            // 1. 命令引擎删除后台数据
            m_engine->deleteWire(wireItem->wireData());
            // 2. 从场景中删除前台图形
            removeItem(wireItem);
            delete wireItem;
        }
        // --- 情况二：删除元件 (这会同时删除所有与之相连的导线) ---
        else if (auto compItem = qgraphicsitem_cast<ComponentItem*>(itemToDelete)) {
            Component* compData = compItem->component();

            // 1. 找出所有需要被删除的后台Wire数据
            QVector<Wire*> wiresToRemove;
            for (Wire* wire : m_engine->getAllWires()) {
                if (wire->startPin()->owner() == compData || wire->endPin()->owner() == compData) {
                    wiresToRemove.append(wire);
                }
            }

            // 2. 遍历场景，删除这些Wire数据对应的前台WireItem图形
            for (Wire* wireData : wiresToRemove) {
                for (QGraphicsItem* item : items()) {
                    if (auto wireItem = qgraphicsitem_cast<WireItem*>(item)) {
                        if (wireItem->wireData() == wireData) {
                            removeItem(wireItem);
                            delete wireItem;
                            break; // 找到了就跳出内层循环
                        }
                    }
                }
                // 命令引擎删除后台Wire数据
                m_engine->deleteWire(wireData);
            }

            // 3. 最后，删除元件本身
            m_engine->deleteComponent(compData);
            removeItem(compItem);
            delete compItem;
        }

        // 删除后，立即重新模拟并刷新界面
        m_engine->simulate();
        update();
        return; // 右键事件处理完毕
    }

    // =============================================================
    // == 左键逻辑 (保持不变)
    // =============================================================
    if (event->button() != Qt::LeftButton) {
        QGraphicsScene::mousePressEvent(event);
        return;
    }

    if (m_currentMode == AddingComponent) {
        Component* data = m_engine->createComponent(m_typeToAdd, event->scenePos());
        if(data) {
            addItem(new ComponentItem(data));
            emit componentAdded();
        }
        setMode(Idle);
        m_engine->simulate();
        update();
        return;
    }

    ComponentItem* compItem = qgraphicsitem_cast<ComponentItem*>(itemAt(event->scenePos(), QTransform()));
    if (compItem) {
        QPointF localPos = compItem->mapFromScene(event->scenePos());
        if (compItem->component()->type() == ComponentType::Input && localPos.x() < 50) {
            static_cast<Input*>(compItem->component())->toggleState();
            m_engine->simulate();
            update();
            return;
        }
        m_startPin = compItem->getPinAt(localPos);
        if (m_startPin) {
            m_tempLine = new QGraphicsLineItem(QLineF(m_startPin->getScenePos(), event->scenePos()));
            m_tempLine->setPen(QPen(Qt::gray, 2, Qt::DashLine));
            addItem(m_tempLine);
            return;
        }
    }
    QGraphicsScene::mousePressEvent(event);
}

void GraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (m_tempLine) {
        m_tempLine->setLine(QLineF(m_tempLine->line().p1(), event->scenePos()));
    } else {
        QGraphicsScene::mouseMoveEvent(event);
        // 全局更新所有导线
        for(QGraphicsItem* item : items()){
            if(auto wireItem = qgraphicsitem_cast<WireItem*>(item)){
                wireItem->updatePosition();
            }
        }
    }
}

void GraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (m_tempLine) {
        removeItem(m_tempLine);
        delete m_tempLine;
        m_tempLine = nullptr;

        ComponentItem* endCompItem = qgraphicsitem_cast<ComponentItem*>(itemAt(event->scenePos(), QTransform()));
        if (endCompItem) {
            Pin* endPin = endCompItem->getPinAt(endCompItem->mapFromScene(event->scenePos()));
            Wire* newWireData = m_engine->createWire(m_startPin, endPin);
            if(newWireData){
                WireItem* wireItem = new WireItem(newWireData);
                addItem(wireItem);
                wireItem->updatePosition();
                m_engine->simulate();
                update();
            }
        }
        m_startPin = nullptr;
        return;
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

void GraphicsScene::rebuildSceneFromEngine()
{
    // 1. 清空当前画布上所有的图形项
    clear();

    // 2. 遍历引擎后台的所有元件数据
    for (Component* compData : m_engine->getAllComponents().values()) {
        // 为每一个后台元件，创建一个新的前台图形项
        ComponentItem* item = new ComponentItem(compData);
        addItem(item);
    }

    // 3. 遍历引擎后台的所有导线数据
    for (Wire* wireData : m_engine->getAllWires()) {
        // 为每一条后台导线，创建一个新的前台图形项
        WireItem* item = new WireItem(wireData);
        addItem(item);
        item->updatePosition(); // 创建后立即更新一次位置
    }

    // （可选）给出调试信息
    qDebug() << "前台画布：已根据引擎状态成功重建。";
}
