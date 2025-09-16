#include "graphics.h"                // 图形项与场景类声明
#include "engine.h"                  // 访问 Component/Pin/Wire 数据
#include <QPainter>                   // 自定义绘制
#include <QGraphicsSceneMouseEvent>   // 场景鼠标事件
#include <QDebug>                     // 调试输出
#include <QStyleOptionGraphicsItem>   // 绘制选中态等风格信息
/**
 * @file graphics.cpp
 * @brief 前端图形项(ComponentItem/WireItem)与交互场景(GraphicsScene)的实现。
 */

// ===============================================
// === ComponentItem 实现
// ===============================================

/** 通过后端组件数据构造，并建立双向绑定 */
ComponentItem::ComponentItem(Component* data) : m_componentData(data) {
    setPos(data->position());
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
    data->setGraphicsItem(this);
}

/** 返回组件的包围盒 */
/** 返回组件的包围盒 */
QRectF ComponentItem::boundingRect() const {
    // 【修改】动态计算高度
    int numInputs = m_componentData->inputPins().size();
    int numOutputs = m_componentData->outputPins().size();
    int maxPins = qMax(numInputs, numOutputs); // 使用 qMax 获取输入和输出引脚数中的较大值

    qreal bodyHeight = 50.0; // 默认高度
    if (maxPins > 4) {
        // 当引脚超过4个时，我们希望保持4个引脚时的间距。
        // 4个引脚时的间距是 50 / (4 + 1) = 10。
        // 所以新高度 = 间距 * (新引脚数 + 1)
        bodyHeight = 10.0 * (maxPins + 1);
    }

    // 返回一个能容纳新高度的包围盒，上下各留10像素的边距
    return QRectF(-10, -10, 120, bodyHeight + 20);
}

/** 绘制组件主体、文字、引脚与选中高亮效果 */
void ComponentItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    int numInputs = m_componentData->inputPins().size();
    int numOutputs = m_componentData->outputPins().size();
    int maxPins = qMax(numInputs, numOutputs);

    qreal bodyHeight = 50.0;
    if (maxPins > 4) {
        bodyHeight = 10.0 * (maxPins + 1);
    }

    // 使用动态计算的高度来定义元件主体的矩形
    QRectF bodyRect(0, 0, 100, bodyHeight);
    painter->setRenderHint(QPainter::Antialiasing);

    // 绘制主体
    painter->setPen(QPen(Qt::darkGray, 2));
    QColor bodyColor("#f0f0f0");
    bodyColor.setAlphaF(0.85);
    painter->setBrush(bodyColor);
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
            painter->drawText(QRectF(0,0,50,50), Qt::AlignCenter, state ? "1" : "0");
            painter->setFont(QFont()); // 恢复
            painter->setPen(Qt::black);
        }
        painter->drawText(QRectF(50,0,50,50), Qt::AlignCenter, text);
        break;

    case ComponentType::Output:
        text = "输出";
        if (!m_componentData->inputPins().isEmpty()) {
            state = m_componentData->inputPins()[0]->getState();
            painter->setBrush(state ? QColor("#4CAF50") : QColor("#F44336"));
            painter->setPen(Qt::NoPen);
            painter->drawRect(0, 0, 50, 50);
            painter->setPen(Qt::white);
            painter->setFont(QFont("Arial", 10, QFont::Bold));
            painter->drawText(QRectF(0,0,50,50), Qt::AlignCenter, state ? "1" : "0");
            painter->setFont(QFont());
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
    case ComponentType::Encapsulated:
    { // 使用花括号创建一个局部作用域
        auto comp = static_cast<EncapsulatedComponent*>(m_componentData);
        text = comp->getName(); // 直接获取名字作为文本
    }
    break;

    }

    // 为普通逻辑门统一绘制文字
    if (m_componentData->type() >= ComponentType::And) {
        painter->drawText(bodyRect, Qt::AlignCenter, text);
    }

    // 绘制引脚
    for (int i = 0; i < numInputs; ++i) {
        qreal yPos = bodyRect.height() * (i + 1) / (numInputs + 1);
        painter->setBrush(m_componentData->inputPins()[i]->getState() ? Qt::green : Qt::darkGray);
        painter->drawEllipse(QPointF(0, yPos), 4, 4);
    }

    for (int i = 0; i < numOutputs; ++i) {
        qreal yPos = bodyRect.height() * (i + 1) / (numOutputs + 1);
        painter->setBrush(m_componentData->outputPins()[i]->getState() ? Qt::green : Qt::darkGray);
        painter->drawEllipse(QPointF(100, yPos), 4, 4);
    }
}

/** 返回后端组件数据 */
Component* ComponentItem::component() const {
    return m_componentData;
}

/** 根据局部坐标命中检测，返回被击中的引脚 */
/** 根据局部坐标命中检测，返回被击中的引脚 */
Pin* ComponentItem::getPinAt(const QPointF &localPos) {
    const qreal pinRadius = 4.0;
    const qreal clickableRadius = pinRadius * 2;

    // 【修改】再次使用相同的逻辑计算元件高度
    int numInputs = m_componentData->inputPins().size();
    int numOutputs = m_componentData->outputPins().size();
    int maxPins = qMax(numInputs, numOutputs);

    qreal bodyHeight = 50.0;
    if (maxPins > 4) {
        bodyHeight = 10.0 * (maxPins + 1);
    }

    for(int i = 0; i < numInputs; ++i) {
        // 使用动态高度 bodyHeight 替代硬编码的 50.0
        qreal yPos = bodyHeight * (i + 1) / (numInputs + 1);
        QRectF pinArea(QPointF(-clickableRadius/2, yPos - clickableRadius/2), QSizeF(clickableRadius, clickableRadius));
        if (pinArea.contains(localPos)) {
            return m_componentData->inputPins()[i];
        }
    }

    for(int i = 0; i < numOutputs; ++i) {
        // 使用动态高度 bodyHeight 替代硬编码的 50.0
        qreal yPos = bodyHeight * (i + 1) / (numOutputs + 1);
        QRectF pinArea(QPointF(100 - clickableRadius/2, yPos - clickableRadius/2), QSizeF(clickableRadius, clickableRadius));
        if (pinArea.contains(localPos)) {
            return m_componentData->outputPins()[i];
        }
    }
    return nullptr;
}
/** 捕获位置变化，将位置同步回后端数据 */
QVariant ComponentItem::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionHasChanged && m_componentData) {
        // 将 this->pos() 改为 value.toPointF()
        qDebug() << "itemChange called! New position:" << value.toPointF() << "Old position:" << this->pos();
        m_componentData->setPosition(value.toPointF());
    }
    return QGraphicsItem::itemChange(change, value);
}

// ===============================================
// === WireItem 实现
// ===============================================

/** 构造导线图形项 */
WireItem::WireItem(Wire* data) : m_wireData(data) {
    setPen(QPen(Qt::black, 2));
    setZValue(-1); // 确保导线在元件下面
}

/** 获取后端导线数据 */
Wire* WireItem::wireData() const {
    return m_wireData;
}

/** 使用后端引脚的场景坐标更新几何 */
void WireItem::updatePosition() {
    if (m_wireData) {
        setLine(QLineF(m_wireData->startPin()->getScenePos(), m_wireData->endPin()->getScenePos()));
    }
}

/** 根据导线状态上色绘制 */
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

/** 通过引擎构造场景，初始化交互状态 */
GraphicsScene::GraphicsScene(Engine* engine, QObject* parent)
    : QGraphicsScene(parent), m_engine(engine), m_tempLine(nullptr), m_startPin(nullptr), m_currentMode(Idle)
{}

/** 设置场景交互模式 */
void GraphicsScene::setMode(Mode mode) {
    m_currentMode = mode;
}

/** 设置下一个要添加的组件类型 */
void GraphicsScene::setComponentTypeToAdd(ComponentType type) {
    m_typeToAdd = type;
}

/** 处理按下事件：右键删除、左键放置/连线 */
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
        Component* data = nullptr;

        // --- 【核心修改】 ---
        if (m_typeToAdd == ComponentType::Encapsulated) {
            // 如果要添加的是封装元件，我们不能用引擎的工厂函数创建
            // 而是直接使用我们存储的 JSON 数据来构造
            data = new EncapsulatedComponent(event->scenePos(), m_nameToAdd, m_jsonToAdd);

            // 重要：因为我们绕过了引擎的创建函数，所以必须手动将这个新元件注册到引擎中
            // (下一步我们将为 Engine 添加这个 registerComponent 函数)
            m_engine->registerComponent(data);
        } else {
            // 对于其他普通元件，继续使用引擎的工厂函数
            data = m_engine->createComponent(m_typeToAdd, event->scenePos());
        }
        // --- 【修改结束】 ---

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

/** 拖动事件：更新临时连线或刷新导线位置 */
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

/** 释放事件：尝试完成连线并触发仿真 */
void GraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (m_tempLine) {
        removeItem(m_tempLine);
        delete m_tempLine;
        m_tempLine = nullptr;

        ComponentItem* endCompItem = qgraphicsitem_cast<ComponentItem*>(itemAt(event->scenePos(), QTransform()));

        // 【核心修改】
        // 旧逻辑: if (endCompItem) { ... }
        // 新逻辑: 先判断是否点中了元件，再判断是否点中了引脚
        if (endCompItem) {
            Pin* endPin = endCompItem->getPinAt(endCompItem->mapFromScene(event->scenePos()));

            // 只有当终点确实是一个有效的引脚时，才尝试创建导线
            if (endPin) {
                Wire* newWireData = m_engine->createWire(m_startPin, endPin);
                if(newWireData){
                    WireItem* wireItem = new WireItem(newWireData);
                    addItem(wireItem);
                    wireItem->updatePosition();
                    m_engine->simulate();
                    update();
                }
            }
            // 如果 endPin 是 nullptr (即点在了元件上但不是引脚)，则什么也不做，静默失败。
        }
        // 如果 endCompItem 是 nullptr (即点在了空白处)，也什么都不做，静默失败。

        m_startPin = nullptr;
        return;
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

/** 根据引擎当前数据重建整张画布（打开文件后使用） */
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
/** 设置下一个封装元件的内部JSON */
void GraphicsScene::setJsonForNextComponent(const QJsonObject& json)
{
    m_jsonToAdd = json;
}
/** 设置下一个封装元件的名称 */
void GraphicsScene::setNameForNextComponent(const QString& name)
{
    m_nameToAdd = name;
}
