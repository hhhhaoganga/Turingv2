#include "graphics.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include "engine.h"
//测试
/**
 * @file graphics.cpp
 * @brief 【B同学负责】实现所有图形和交互功能。
 * @details 你将在这里填充所有在 graphics.h 中声明的函数的具体实现。
 */

// === ComponentItem 实现 ===
ComponentItem::ComponentItem(Component *data)
    : m_data(data)
{
    setFlags(ItemIsMovable | ItemIsSelectable);
    setPos(data->position());
    data->setGraphicsItem(this);
}
ComponentItem::~ComponentItem() {}
QRectF ComponentItem::boundingRect() const
{
    return QRectF(-5, -5, 110, 60);
}

/**
 * @brief 绘制元件本身。
 * @param painter - 【输入】Qt提供的“画笔”对象。
 * @note  这个函数在需要重绘时被Qt自动调用。
 * @利用   m_data->type() 来从后台获取元件类型，以决定画什么。
 */
void ComponentItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // B同学在这里填充所有元件的绘制逻辑
    painter->setBrush(Qt::lightGray);
    painter->drawRect(0, 0, 100, 50);

    QString text = "Unknown";
    switch (m_data->type()) {
    case ComponentType::Input:
        text = "输入";
        break;
    case ComponentType::Output:
        text = "输出";
        break;
    case ComponentType::And:
        text = "与门";
        break;
    case ComponentType::Or:
        text = "或门";
        break;
    case ComponentType::Not:
        text = "非门";
        break;
    case ComponentType::Nand:
        text = "与非门";
        break;
    case ComponentType::Nor:
        text = "或非门";
        break;
    case ComponentType::Xor:
        text = "异或门";
        break;
    case ComponentType::Xnor:
        text = "同或门";
        break;
    case ComponentType::SubCircuit:
        text = " 封装";
        break;
    default:
        break;
    }
    painter->drawText(QRectF(0, 0, 100, 50), Qt::AlignCenter, text);
}

// === GraphicsScene 实现 ===
GraphicsScene::GraphicsScene(Engine *engine, QObject *parent)
    : QGraphicsScene(parent)
    , m_engine(engine)
    , m_currentMode(Idle)
{}

void GraphicsScene::setMode(Mode mode)
{
    m_currentMode = mode;
}
void GraphicsScene::setComponentTypeToAdd(ComponentType type)
{
    m_typeToAdd = type;
}

/**
 * @brief 处理所有鼠标按下事件。
 * @param event - 【输入】Qt提供的鼠标事件对象，包含点击位置等信息。
 * @利用   1. m_currentMode - 判断当前应该执行什么操作。
 *        2. m_engine - 向后台引擎发送“创建元件”等命令。
 */
void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_currentMode == AddingComponent) {
        // 调用A同学的引擎在后台创建数据
        Component *newComp = m_engine->createComponent(m_typeToAdd, event->scenePos());
        if (newComp) {
            // 在前台创建对应的图形项
            ComponentItem *newItem = new ComponentItem(newComp);
            addItem(newItem);
        }
        setMode(Idle); // 添加完后自动返回空闲模式
    } else {
        QGraphicsScene::mousePressEvent(event);
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
        localPinPos.setX(0);                        // 输入引脚在左边，x=0

        // 如果没有输入引脚，避免除以0的错误
        if (numInputs > 0) {
            localPinPos.setY((itemHeight / (numInputs + 1)) * (index + 1));
        }

    } else { // pinType == Pin::PinType::Output
        // --- 处理输出引脚 ---
        int numOutputs = m_data->outputPins().size(); // 获取输出引脚总数
        localPinPos.setX(itemWidth);                  // 输出引脚在右边，x=100

        // 避免除以0的错误
        if (numOutputs > 0) {
            localPinPos.setY((itemHeight / (numOutputs + 1)) * (index + 1));
        }
    }

    // 将计算好的局部坐标，转换为整个场景的全局坐标并返回
    return this->mapToScene(localPinPos);
}
