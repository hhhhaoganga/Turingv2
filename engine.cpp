#include "engine.h"
#include "graphics.h"
#include <QMessageBox>
#include <QDebug>
#include <QJsonObject>

// === Pin 实现 ===
Pin::Pin(Component* owner, PinType type, int index) : m_owner(owner), m_type(type), m_index(index), m_state(false) {}
bool Pin::getState() const { return m_state; }
void Pin::setState(bool state) { m_state = state; }
Component* Pin::owner() const { return m_owner; }
Pin::PinType Pin::type() const { return m_type; }
int Pin::index() const { return m_index; }
QPointF Pin::getScenePos() const {
    if (m_owner && m_owner->getGraphicsItem()) {
        int pinCount = (m_type == Input) ? m_owner->inputPins().size() : m_owner->outputPins().size();
        qreal yPos = 50.0 * (m_index + 1) / (pinCount + 1);
        QPointF localPos = (m_type == Input) ? QPointF(0, yPos) : QPointF(100, yPos);
        return m_owner->getGraphicsItem()->mapToScene(localPos);
    }
    return QPointF();
}

// === Wire 实现 ===
Wire::Wire(Pin* start, Pin* end) : m_startPin(start), m_endPin(end) {}
Pin* Wire::startPin() const { return m_startPin; }
Pin* Wire::endPin() const { return m_endPin; }
bool Wire::getState() const { return m_startPin->getState(); }

// === Component 实现 ===
Component::Component(ComponentType type, const QPointF& position, int numInputs, int numOutputs) : m_type(type), m_position(position), m_graphicsItem(nullptr) {
    for (int i = 0; i < numInputs; ++i) m_inputPins.append(new Pin(this, Pin::Input, i));
    for (int i = 0; i < numOutputs; ++i) m_outputPins.append(new Pin(this, Pin::Output, i));
}
Component::~Component() { qDeleteAll(m_inputPins); qDeleteAll(m_outputPins); }
ComponentType Component::type() const { return m_type; }
const QVector<Pin*>& Component::inputPins() const { return m_inputPins; }
const QVector<Pin*>& Component::outputPins() const { return m_outputPins; }
void Component::setGraphicsItem(ComponentItem* item) { m_graphicsItem = item; }
ComponentItem* Component::getGraphicsItem() const { return m_graphicsItem; }
void Component::setPosition(const QPointF& pos) { m_position = pos; }
QPointF Component::position() const { return m_position; }


// === 具体元件实现 ===
Input::Input(const QPointF& pos) : Component(ComponentType::Input, pos, 0, 1), m_currentState(false) {}
void Input::evaluate() { if (!m_outputPins.isEmpty()) { m_outputPins[0]->setState(m_currentState); } }
void Input::toggleState() { m_currentState = !m_currentState; }

Output::Output(const QPointF& pos) : Component(ComponentType::Output, pos, 1, 0) {}
void Output::evaluate() { /* 状态由输入引脚决定 */ }

AndGate::AndGate(const QPointF& pos) : Component(ComponentType::And, pos, 2, 1) {}
void AndGate::evaluate() { if (m_inputPins.size() == 2 && !m_outputPins.isEmpty()) m_outputPins[0]->setState(m_inputPins[0]->getState() && m_inputPins[1]->getState()); }

OrGate::OrGate(const QPointF& pos) : Component(ComponentType::Or, pos, 2, 1) {}
void OrGate::evaluate() { if (m_inputPins.size() == 2 && !m_outputPins.isEmpty()) m_outputPins[0]->setState(m_inputPins[0]->getState() || m_inputPins[1]->getState()); }

NotGate::NotGate(const QPointF& pos) : Component(ComponentType::Not, pos, 1, 1) {}
void NotGate::evaluate() { if (!m_inputPins.isEmpty() && !m_outputPins.isEmpty()) m_outputPins[0]->setState(!m_inputPins[0]->getState()); }

NandGate::NandGate(const QPointF& pos) : Component(ComponentType::Nand, pos, 2, 1) {}
void NandGate::evaluate() { if (m_inputPins.size() == 2 && !m_outputPins.isEmpty()) m_outputPins[0]->setState(!(m_inputPins[0]->getState() && m_inputPins[1]->getState())); }

NorGate::NorGate(const QPointF& pos) : Component(ComponentType::Nor, pos, 2, 1) {}
void NorGate::evaluate() { if (m_inputPins.size() == 2 && !m_outputPins.isEmpty()) m_outputPins[0]->setState(!(m_inputPins[0]->getState() || m_inputPins[1]->getState())); }

XorGate::XorGate(const QPointF& pos) : Component(ComponentType::Xor, pos, 2, 1) {}
void XorGate::evaluate() { if (m_inputPins.size() == 2 && !m_outputPins.isEmpty()) m_outputPins[0]->setState(m_inputPins[0]->getState() != m_inputPins[1]->getState()); }

XnorGate::XnorGate(const QPointF& pos) : Component(ComponentType::Xnor, pos, 2, 1) {}
void XnorGate::evaluate() { if (m_inputPins.size() == 2 && !m_outputPins.isEmpty()) m_outputPins[0]->setState(m_inputPins[0]->getState() == m_inputPins[1]->getState()); }

// === Engine 实现 ===
Engine::Engine() {}
Engine::~Engine() { qDeleteAll(m_components.values()); qDeleteAll(m_wires); }

Component* Engine::createComponent(ComponentType type, const QPointF& pos) {
    Component* newComponent = nullptr;
    switch (type) {
    case ComponentType::Input: newComponent = new Input(pos); break;
    case ComponentType::Output: newComponent = new Output(pos); break;
    case ComponentType::And: newComponent = new AndGate(pos); break;
    case ComponentType::Or: newComponent = new OrGate(pos); break;
    case ComponentType::Not: newComponent = new NotGate(pos); break;
    case ComponentType::Nand: newComponent = new NandGate(pos); break;
    case ComponentType::Nor: newComponent = new NorGate(pos); break;
    case ComponentType::Xor: newComponent = new XorGate(pos); break;
    case ComponentType::Xnor: newComponent = new XnorGate(pos); break;
    }
    if (newComponent) { m_components.insert(reinterpret_cast<intptr_t>(newComponent), newComponent); }
    return newComponent;
}

Wire* Engine::createWire(Pin* startPin, Pin* endPin) {
    if (!startPin || !endPin || startPin->owner() == endPin->owner() || startPin->type() == endPin->type()) {
        QMessageBox::warning(nullptr, "非法连接", "不能连接到自身或同类型引脚。");
        return nullptr;
    }
    if (startPin->type() == Pin::Input) { std::swap(startPin, endPin); }
    if (startPin->type() != Pin::Output || endPin->type() != Pin::Input) {
        QMessageBox::warning(nullptr, "非法连接", "必须由输出引脚连接到输入引脚。");
        return nullptr;
    }
    for (const auto& wire : m_wires) {
        if (wire->endPin() == endPin) {
            QMessageBox::warning(nullptr, "非法连接", "该输入引脚已被占用。");
            return nullptr;
        }
    }
    Wire* newWire = new Wire(startPin, endPin);
    m_wires.append(newWire);
    return newWire;
}

// in engine.cpp

// in engine.cpp

void Engine::simulate()
{
    const int maxIterations = 100;
    bool stateChangedInLastIteration = true;

    for (int i = 0; i < maxIterations && stateChangedInLastIteration; ++i) {
        stateChangedInLastIteration = false;

        QMap<Pin*, bool> oldPinStates;
        for (auto const& [key, comp] : m_components.asKeyValueRange()) {
            for (Pin* pin : comp->inputPins()) { oldPinStates[pin] = pin->getState(); }
            for (Pin* pin : comp->outputPins()) { oldPinStates[pin] = pin->getState(); }
        }

        // --- 核心修复：先将所有非源头的输入引脚状态清零 ---
        // 这是解决“删除导线后状态不更新”Bug的关键
        for (auto const& [key, comp] : m_components.asKeyValueRange()) {
            if (comp->type() != ComponentType::Input) {
                for (Pin* pin : comp->inputPins()) {
                    pin->setState(false);
                }
            }
        }

        // --- 正常的传播与计算 ---
        for (auto const& [key, comp] : m_components.asKeyValueRange()) {
            if (comp->type() == ComponentType::Input) {
                comp->evaluate();
            }
        }
        for (auto wire : m_wires) {
            wire->endPin()->setState(wire->startPin()->getState());
        }
        for (auto const& [key, comp] : m_components.asKeyValueRange()) {
            if (comp->type() != ComponentType::Input) {
                comp->evaluate();
            }
        }

        // --- 检查稳定 ---
        for (auto const& [key, comp] : m_components.asKeyValueRange()) {
            for (Pin* pin : comp->inputPins()) {
                if (oldPinStates[pin] != pin->getState()) { stateChangedInLastIteration = true; break; }
            }
            if (stateChangedInLastIteration) break;
            for (Pin* pin : comp->outputPins()) {
                if (oldPinStates[pin] != pin->getState()) { stateChangedInLastIteration = true; break; }
            }
            if (stateChangedInLastIteration) break;
        }
    }
}
const QMap<intptr_t, Component*>& Engine::getAllComponents() const { return m_components; }
const QVector<Wire*>& Engine::getAllWires() const { return m_wires; }
void Engine::deleteComponent(Component* component) { /* ... */ } // 省略删除逻辑，保持B同学文件独立
void Engine::deleteWire(Wire* wire) { if (!wire) return; m_wires.removeAll(wire); delete wire; }

bool Engine::loadCircuitFromJson(const QJsonObject& json)
{
    // A同学未来在这里实现真正的JSON解析和电路重建逻辑
    qDebug() << "后台引擎：loadCircuitFromJson 被调用，但尚未实现。";
    // 暂时总是返回 true，让C同学的流程能跑通
    return true;
}

void Engine::clearAll() {
    qDeleteAll(m_wires);
    m_wires.clear();
    qDeleteAll(m_components.values());
    m_components.clear();
}
