#include "graphics.h"
#include "engine.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
//测试
/**
 * @file graphics.cpp
 * @brief 【B同学负责】实现所有图形和交互功能。
 * @details 你将在这里填充所有在 graphics.h 中声明的函数的具体实现。
 */

// === ComponentItem 实现 ===
ComponentItem::ComponentItem(Component* data) : m_data(data) {
    setFlags(ItemIsMovable | ItemIsSelectable);
    setPos(data->position());
    data->setGraphicsItem(this);
}
ComponentItem::~ComponentItem() {}
QRectF ComponentItem::boundingRect() const { return QRectF(-5, -5, 110, 60); }

/**
 * @brief 绘制元件本身。
 * @param painter - 【输入】Qt提供的“画笔”对象。
 * @note  这个函数在需要重绘时被Qt自动调用。
 * @利用   m_data->type() 来从后台获取元件类型，以决定画什么。
 */
void ComponentItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // --- 准备工作 ---
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(Qt::black, 2)); // 默认画笔用于绘制轮廓
    painter->setBrush(Qt::white);     // 默认画刷用于填充主体

    // 定义元件主体尺寸
    const qreal itemWidth = 100.0;
    const qreal itemHeight = 50.0;
    QRectF bodyRect(0, 0, itemWidth, itemHeight);

    // --- 步骤 1 & 2: 绘制方框主体和文字 (不变) ---
    painter->drawRect(bodyRect);
    QString text = "?";
    switch (m_data->type()) {
    case ComponentType::Input: text = "输入"; break;
    case ComponentType::Output: text = "输出"; break;
    case ComponentType::And: text = "与门"; break;
    case ComponentType::Or: text = "或门"; break;
    case ComponentType::Not: text = "非门"; break;
    case ComponentType::Nand: text = "与非门"; break;
    case ComponentType::Nor: text = "或非门"; break;
    case ComponentType::Xor: text = "异或"; break;
    case ComponentType::Xnor: text = "同或"; break;
    case ComponentType::SubCircuit: text = "子电路"; break;
    default: break;
    }
    painter->drawText(bodyRect, Qt::AlignCenter, text);

    // =======================================================
    // ==           ↓↓↓ 引脚绘制逻辑大升级 ↓↓↓           ==
    // =======================================================

    const qreal pinRadius = 5.0; // 定义引脚圆形的半径

    // --- 步骤 3: 绘制输入引脚 (圆形 + 状态颜色) ---
    int numInputs = m_data->inputPins().size();
    if (numInputs > 0) {
        for (int i = 0; i < numInputs; ++i) {
            // 3a. 获取这个引脚的后台 Pin 对象
            Pin* inputPin = m_data->inputPins().at(i);
            // 3b. 从 Pin 对象获取它的当前逻辑状态 (true/false)
            bool pinState = inputPin->getState();

            // 3c. 根据状态设置画刷的颜色
            if (pinState) { // 如果状态是 true (高电平)
                painter->setBrush(Qt::green); // 用绿色填充
            } else { // 如果状态是 false (低电平)
                painter->setBrush(Qt::red); // 用红色填充
            }

            // 3d. 计算引脚圆心的位置 (y坐标不变, x坐标在左边框)
            qreal y = (itemHeight / (numInputs + 1)) * (i + 1);
            QPointF pinCenter(0, y);

            // 3e. 以该点为中心，画出圆形引脚
            painter->drawEllipse(pinCenter, pinRadius, pinRadius);
        }
    }

    // --- 步骤 4: 绘制输出引脚 (圆形 + 状态颜色) ---
    int numOutputs = m_data->outputPins().size();
    if (numOutputs > 0) {
        for (int i = 0; i < numOutputs; ++i) {
            // 同样，获取状态并设置颜色
            Pin* outputPin = m_data->outputPins().at(i);
            bool pinState = outputPin->getState();
            painter->setBrush(pinState ? Qt::green : Qt::red); // (这是 if/else 的一种简写形式)

            // 计算圆心位置 (x坐标在右边框)
            qreal y = (itemHeight / (numOutputs + 1)) * (i + 1);
            QPointF pinCenter(itemWidth, y);

            // 画出圆形
            painter->drawEllipse(pinCenter, pinRadius, pinRadius);
        }
    }
}

// === GraphicsScene 实现 ===
GraphicsScene::GraphicsScene(Engine* engine, QObject* parent)
    : QGraphicsScene(parent), m_engine(engine), m_currentMode(Idle),m_startPin(nullptr), m_tempLine(nullptr)  {}

void GraphicsScene::setMode(Mode mode) { m_currentMode = mode; }
void GraphicsScene::setComponentTypeToAdd(ComponentType type) { m_typeToAdd = type; }

/**
 * @brief 处理所有鼠标按下事件。
 * @param event - 【输入】Qt提供的鼠标事件对象，包含点击位置等信息。
 * @利用   1. m_currentMode - 判断当前应该执行什么操作。
 *        2. m_engine - 向后台引擎发送“创建元件”等命令。
 */
void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (m_currentMode == AddingComponent) {
        // 调用A同学的引擎在后台创建数据
        Component* newComp = m_engine->createComponent(m_typeToAdd, event->scenePos());
        if (newComp) {
            // 在前台创建对应的图形项
            ComponentItem* newItem = new ComponentItem(newComp);
            addItem(newItem);
            emit componentAdded();// 广播“元件添加完毕”的信号
        }
        setMode(Idle); // 添加完后自动返回空闲模式
    } else { // m_currentMode == Idle
        // --- 双向连接的新逻辑：允许从任何引脚开始 ---
        Pin* startPin = findPinAt(event->scenePos()); // 侦测引脚

        // 只要点中了任何一个引脚 (不管是输入还是输出)，就开始画线
        if (startPin) {
            m_startPin = startPin; // 记住起点

            // 创建临时虚线 (这部分逻辑不变)
            m_tempLine = new QGraphicsLineItem();
            ComponentItem* startItem = startPin->owner()->getGraphicsItem();
            QPointF startPos = startItem->getPinScenePosition(startPin->type(), startPin->index());
            m_tempLine->setLine(QLineF(startPos, startPos));
            m_tempLine->setPen(QPen(Qt::gray, 2, Qt::DashLine));
            addItem(m_tempLine);
        } else {
            // 如果没点中任何引脚，执行默认行为
            QGraphicsScene::mousePressEvent(event);
        }
    }
}


// in graphics.cpp

QPointF ComponentItem::getPinScenePosition(Pin::PinType pinType, int index) const
{
    // 定义元件的固定尺寸，这应该和你在paint函数里画的范围一致。
    const qreal itemWidth = 100.0;
    const qreal itemHeight = 50.0;
    QPointF localPinPos; // 用来存储计算出的局部坐标

    if (pinType == Pin::PinType::Input) {
        // --- 处理输入引脚 ---
        int numInputs = m_data->inputPins().size(); // 获取输入引脚总数
        localPinPos.setX(0); // 输入引脚在左边，x=0

        // 如果没有输入引脚，避免除以0的错误
        if (numInputs > 0) {
            localPinPos.setY((itemHeight / (numInputs + 1)) * (index + 1));
        }

    } else { // pinType == Pin::PinType::Output
        // --- 处理输出引脚 ---
        int numOutputs = m_data->outputPins().size(); // 获取输出引脚总数
        localPinPos.setX(itemWidth); // 输出引脚在右边，x=100

        // 避免除以0的错误
        if (numOutputs > 0) {
            localPinPos.setY((itemHeight / (numOutputs + 1)) * (index + 1));
        }
    }

    // 将计算好的局部坐标，转换为整个场景的全局坐标并返回
    return this->mapToScene(localPinPos);
}

/**
 * @brief 【B同学新增】一个辅助函数，用于查找指定场景坐标下的引脚。
 * @param scenePos 【输入】鼠标点击处的场景坐标。
 * @return 如果找到了引脚，返回指向该Pin对象的指针；否则返回nullptr。
 */
Pin* GraphicsScene::findPinAt(const QPointF& scenePos) const
{
    // 1. 获取该坐标下的所有图形项
    QList<QGraphicsItem*> itemsAtPos = items(scenePos);

    // 2. 遍历这些图形项
    for (QGraphicsItem* item : itemsAtPos) {
        // 3. 尝试将它转换为我们自定义的 ComponentItem
        ComponentItem* compItem = dynamic_cast<ComponentItem*>(item);

        // 如果转换成功，说明我们点中的是一个元件
        if (compItem) {
            // 4. 拿到后台的 Component 对象
            Component* comp = compItem->component(); // (这里需要把 m_data 暴露出来，或者提供一个公共接口)

            // 5. 遍历这个元件的所有输入引脚
            for (Pin* pin : comp->inputPins()) {
                // 获取引脚的全局坐标
                QPointF pinPos = compItem->getPinScenePosition(Pin::Input, pin->index());
                // 计算鼠标点击位置和引脚位置的距离
                if (QLineF(scenePos, pinPos).length() <= 5.0) { // 5.0是点击的容差半径
                    return pin; // 找到了！
                }
            }

            // 6. 遍历所有输出引脚 (逻辑同上)
            for (Pin* pin : comp->outputPins()) {
                QPointF pinPos = compItem->getPinScenePosition(Pin::Output, pin->index());
                if (QLineF(scenePos, pinPos).length() <= 5.0) {
                    return pin;
                }
            }
        }
    }

    return nullptr; // 遍历完所有东西都没找到，返回空指针
}

// in graphics.cpp
// in graphics.cpp
void GraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (m_tempLine) {
        m_tempLine->setLine(QLineF(m_tempLine->line().p1(), event->scenePos()));
    } else {
        // 先让元件移动
        QGraphicsScene::mouseMoveEvent(event);
        // 然后强制所有导线更新
        for(QGraphicsItem* item : items()){
            if(auto wireItem = qgraphicsitem_cast<WireItem*>(item)){
                wireItem->updatePosition();
            }
        }
    }
}

// in graphics.cpp

void GraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_startPin && m_tempLine) {
        removeItem(m_tempLine);
        delete m_tempLine;
        m_tempLine = nullptr;

        Pin* endPin = findPinAt(event->scenePos());

        if (endPin && endPin->owner() != m_startPin->owner() && endPin->type() != m_startPin->type()) {
            Wire* newWireData = m_engine->createWire(outputPin, inputPin);
            if(newWireData){
                // 创建前台 WireItem
                WireItem* wireItem = new WireItem(newWireData);
                addItem(wireItem);
                // 创建后立刻更新一次
                wireItem->updatePosition();
            }
        }
        m_startPin = nullptr;
    } else {
        QGraphicsScene::mouseReleaseEvent(event);
    }
}


// in graphics.cpp (at the end of file)
WireItem::WireItem(Wire* data) : m_wireData(data)
{
    setPen(QPen(Qt::black, 2));
    setZValue(-1);
}

void WireItem::updatePosition()
{
    // 这个逻辑和你成熟版本里的完全一样
    if (m_wireData) {
        // 【协作点】这里需要 A 同学实现 Wire 类和 getScenePos
        setLine(QLineF(m_wireData->startPin()->getScenePos(), m_wireData->endPin()->getScenePos()));
    }
}
