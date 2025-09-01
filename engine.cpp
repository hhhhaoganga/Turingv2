#include "engine.h"
#include "graphics.h"

/**
 * @file engine.cpp
 * @brief 【A同学负责】实现后台引擎的所有功能。
 * @details 你将在这里填充所有在 engine.h 中声明的函数的具体实现。
 */

// === Pin 实现 ===
Pin::Pin(Component *owner, PinType type, int index)
    : m_owner(owner)
    , m_type(type)
    , m_index(index)
    , m_state(false)
{}
bool Pin::getState() const
{
    return m_state;
}
void Pin::setState(bool state)
{
    m_state = state;
}
Component *Pin::owner() const
{
    return m_owner;
}
int Pin::index() const
{
    return m_index;
}
QPointF Pin::getScenePos() const
{
    if (m_owner && m_owner->getGraphicsItem()) {
        // 占位逻辑
    }
    return QPointF();
}

// === Component 实现 ===
Component::Component(ComponentType type, const QPointF &position, int numInputs, int numOutputs)
    : m_type(type)
    , m_position(position)
    , m_graphicsItem(nullptr)
{
    for (int i = 0; i < numInputs; ++i)
        m_inputPins.append(new Pin(this, Pin::Input, i));
    for (int i = 0; i < numOutputs; ++i)
        m_outputPins.append(new Pin(this, Pin::Output, i));
}
Component::~Component()
{
    qDeleteAll(m_inputPins);
    qDeleteAll(m_outputPins);
}
ComponentType Component::type() const
{
    return m_type;
}
const QVector<Pin *> &Component::inputPins() const
{
    return m_inputPins;
}
const QVector<Pin *> &Component::outputPins() const
{
    return m_outputPins;
}
void Component::setGraphicsItem(ComponentItem *item)
{
    m_graphicsItem = item;
}
ComponentItem *Component::getGraphicsItem() const
{
    return m_graphicsItem;
}
QPointF Component::position() const
{
    return m_position;
}
// === 具体元件类实现 ===
// 目的: 为每个在.h中声明的元件类提供构造函数和evaluate的空实现，作为开发的起点。
Input::Input(const QPointF &pos)
    : Component(ComponentType::Input, pos, 0, 1)
{}
void Input::evaluate()
{ /* A同学填充 */
}

Output::Output(const QPointF &pos)
    : Component(ComponentType::Output, pos, 1, 0)
{}
void Output::evaluate()
{ /* A同学填充 */
}

AndGate::AndGate(const QPointF &pos)
    : Component(ComponentType::And, pos, 2, 1)
{}
void AndGate::evaluate()
{ /* A同学填充 */
}

OrGate::OrGate(const QPointF &pos)
    : Component(ComponentType::Or, pos, 2, 1)
{}
void OrGate::evaluate()
{ /* A同学填充 */
}

NotGate::NotGate(const QPointF &pos)
    : Component(ComponentType::Not, pos, 1, 1)
{}
void NotGate::evaluate()
{ /* A同学填充 */
}

NandGate::NandGate(const QPointF &pos)
    : Component(ComponentType::Nand, pos, 2, 1)
{}
void NandGate::evaluate()
{ /* A同学填充 */
}

NorGate::NorGate(const QPointF &pos)
    : Component(ComponentType::Nor, pos, 2, 1)
{}
void NorGate::evaluate()
{ /* A同学填充 */
}

XorGate::XorGate(const QPointF &pos)
    : Component(ComponentType::Xor, pos, 2, 1)
{}
void XorGate::evaluate()
{ /* A同学填充 */
}

XnorGate::XnorGate(const QPointF &pos)
    : Component(ComponentType::Xnor, pos, 2, 1)
{}
void XnorGate::evaluate()
{ /* A同学填充 */
}

// === Engine 实现 ===
Engine::Engine() {}
Engine::~Engine() {}

/**
 * @brief 实现createComponent这个对外的API。
 * @param type - 【输入】要创建的元件类型。
 *               【处理方式】: 使用switch语句，为.h中ComponentType枚举里的【每一个值】
 *                            都提供一个对应的创建逻辑。
 * @param pos  - 【输入】元件的初始位置。
 * @return     - 【输出】指向新创建的Component对象的指针。
 */
Component *Engine::createComponent(ComponentType type, const QPointF &pos)
{
    Component *newComp = nullptr;
    switch (type) {
    case ComponentType::Input:
        newComp = new Input(pos);
        break;
    case ComponentType::Output:
        newComp = new Output(pos);
        break;
    case ComponentType::And:
        newComp = new AndGate(pos);
        break;
    case ComponentType::Or:
        newComp = new OrGate(pos);
        break;
    case ComponentType::Not:
        newComp = new NotGate(pos);
        break;
    case ComponentType::Nand:
        newComp = new NandGate(pos);
        break;
    case ComponentType::Nor:
        newComp = new NorGate(pos);
        break;
    case ComponentType::Xor:
        newComp = new XorGate(pos);
        break;
    case ComponentType::Xnor:
        newComp = new XnorGate(pos);
        break;
    case ComponentType::SubCircuit:
        // A同学未来在这里实现创建自定义封装元件的逻辑
        break;
    }
    // if (newComp) { m_mainCircuit->addComponent(newComp); } // 这是未来更优的实现
    return newComp;
}
