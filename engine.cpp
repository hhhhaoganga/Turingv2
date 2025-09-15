#include "engine.h"        // 引擎与组件/导线/引脚的声明
#include "graphics.h"      // 访问 ComponentItem（用于计算引脚场景坐标）
#include <QMessageBox>      // 弹出非法连接等警告
#include <QDebug>           // 调试日志输出
#include <QJsonObject>      // JSON 对象读写
#include <QJsonArray>       // JSON 数组读写
#include <algorithm>        // std::sort 等算法
/**
 * @file engine.cpp
 * @brief 引擎与基础数据结构(Pin/Wire/Component)的实现，以及封装元件逻辑。
 */
// === Pin 实现 ===
/** Pin 构造函数 */
Pin::Pin(Component* owner, PinType type, int index) : m_owner(owner), m_type(type), m_index(index), m_state(false) {}
/** 获取引脚状态 */
bool Pin::getState() const { return m_state; }
/** 设置引脚状态 */
void Pin::setState(bool state) { m_state = state; }
/** 获取所属组件 */
Component* Pin::owner() const { return m_owner; }
/** 获取引脚类型 */
Pin::PinType Pin::type() const { return m_type; }
/** 获取引脚索引 */
int Pin::index() const { return m_index; }
/** 计算并返回场景坐标中的引脚位置 */
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
/** Wire 构造函数：连接两个引脚 */
Wire::Wire(Pin* start, Pin* end) : m_startPin(start), m_endPin(end) {}
/** 获取起始引脚 */
Pin* Wire::startPin() const { return m_startPin; }
/** 获取终止引脚 */
Pin* Wire::endPin() const { return m_endPin; }
/** 导线状态：等于起始引脚状态 */
bool Wire::getState() const { return m_startPin->getState(); }

// === Component 实现 ===
/** Component 构造：根据数量创建输入/输出引脚 */
Component::Component(ComponentType type, const QPointF& position, int numInputs, int numOutputs) : m_type(type), m_position(position), m_graphicsItem(nullptr) {
    for (int i = 0; i < numInputs; ++i) m_inputPins.append(new Pin(this, Pin::Input, i));
    for (int i = 0; i < numOutputs; ++i) m_outputPins.append(new Pin(this, Pin::Output, i));
}
/** 析构：释放引脚 */
Component::~Component() { qDeleteAll(m_inputPins); qDeleteAll(m_outputPins); }
/** 获取类型 */
ComponentType Component::type() const { return m_type; }
/** 读取输入引脚 */
const QVector<Pin*>& Component::inputPins() const { return m_inputPins; }
/** 读取输出引脚 */
const QVector<Pin*>& Component::outputPins() const { return m_outputPins; }
/** 绑定图形项 */
void Component::setGraphicsItem(ComponentItem* item) { m_graphicsItem = item; }
/** 获取图形项 */
ComponentItem* Component::getGraphicsItem() const { return m_graphicsItem; }
/** 设置位置 */
void Component::setPosition(const QPointF& pos) { m_position = pos; }
/** 获取位置 */
QPointF Component::position() const { return m_position; }


// === 具体元件实现 ===
/** Input 构造：0入1出 */
Input::Input(const QPointF& pos) : Component(ComponentType::Input, pos, 0, 1), m_currentState(false) {}
/** 将内部状态输出到引脚 */
void Input::evaluate() { if (!m_outputPins.isEmpty()) { m_outputPins[0]->setState(m_currentState); } }
/** 翻转状态 */
void Input::toggleState() { m_currentState = !m_currentState; evaluate(); }
/** 设置状态并触发一次评估 */
void Input::setState(bool state) {
    m_currentState = state;
    evaluate(); // <-- 加上这一行！
}

/** Output 构造：1入0出 */
Output::Output(const QPointF& pos) : Component(ComponentType::Output, pos, 1, 0) {}
/** 输出端不主动改变状态，显示输入 */
void Output::evaluate() { /* 状态由输入引脚决定 */ }

/** 与门：2入1出 */
AndGate::AndGate(const QPointF& pos) : Component(ComponentType::And, pos, 2, 1) {}
/** 计算与 */
void AndGate::evaluate() { if (m_inputPins.size() == 2 && !m_outputPins.isEmpty()) m_outputPins[0]->setState(m_inputPins[0]->getState() && m_inputPins[1]->getState()); }

/** 或门：2入1出 */
OrGate::OrGate(const QPointF& pos) : Component(ComponentType::Or, pos, 2, 1) {}
/** 计算或 */
void OrGate::evaluate() { if (m_inputPins.size() == 2 && !m_outputPins.isEmpty()) m_outputPins[0]->setState(m_inputPins[0]->getState() || m_inputPins[1]->getState()); }

/** 非门：1入1出 */
NotGate::NotGate(const QPointF& pos) : Component(ComponentType::Not, pos, 1, 1) {}
/** 计算非 */
void NotGate::evaluate() { if (!m_inputPins.isEmpty() && !m_outputPins.isEmpty()) m_outputPins[0]->setState(!m_inputPins[0]->getState()); }

/** 与非门：2入1出 */
NandGate::NandGate(const QPointF& pos) : Component(ComponentType::Nand, pos, 2, 1) {}
/** 计算与非 */
void NandGate::evaluate() { if (m_inputPins.size() == 2 && !m_outputPins.isEmpty()) m_outputPins[0]->setState(!(m_inputPins[0]->getState() && m_inputPins[1]->getState())); }

/** 或非门：2入1出 */
NorGate::NorGate(const QPointF& pos) : Component(ComponentType::Nor, pos, 2, 1) {}
/** 计算或非 */
void NorGate::evaluate() { if (m_inputPins.size() == 2 && !m_outputPins.isEmpty()) m_outputPins[0]->setState(!(m_inputPins[0]->getState() || m_inputPins[1]->getState())); }

/** 异或门：2入1出 */
XorGate::XorGate(const QPointF& pos) : Component(ComponentType::Xor, pos, 2, 1) {}
/** 计算异或 */
void XorGate::evaluate() { if (m_inputPins.size() == 2 && !m_outputPins.isEmpty()) m_outputPins[0]->setState(m_inputPins[0]->getState() != m_inputPins[1]->getState()); }

/** 同或门：2入1出 */
XnorGate::XnorGate(const QPointF& pos) : Component(ComponentType::Xnor, pos, 2, 1) {}
/** 计算同或 */
void XnorGate::evaluate() { if (m_inputPins.size() == 2 && !m_outputPins.isEmpty()) m_outputPins[0]->setState(m_inputPins[0]->getState() == m_inputPins[1]->getState()); }

// === Engine 实现 ===
/** 引擎构造 */
Engine::Engine() {}
/** 析构：释放组件与导线 */
Engine::~Engine() { qDeleteAll(m_components.values()); qDeleteAll(m_wires); }

/** 创建组件并注册到引擎 */
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

/**
 * @brief 从JSON对象创建组件（支持 Encapsulated）。
 * @param compObject 组件的JSON定义（包含 type/x/y 以及封装元件的内部定义）
 * @return 创建成功返回新组件指针，失败返回nullptr
 */
Component* Engine::createComponent(const QJsonObject& compObject)
{
    ComponentType type = static_cast<ComponentType>(compObject["type"].toInt());
    QPointF pos(compObject["x"].toDouble(), compObject["y"].toDouble());

    if (type == ComponentType::Encapsulated) {
        // 如果是封装元件，从 "internal_circuit" 字段获取其定义
        QJsonObject internalJson = compObject["internal_circuit"].toObject();
        QString name = compObject["name"].toString("封装元件");

        // 1. 创建元件实例
        Component* newComponent = new EncapsulatedComponent(pos, name, internalJson);

        // 2. 【核心修复】创建后，必须手动将其注册到当前引擎实例中
        //    这样内部引擎在仿真时才能找到这个嵌套的子元件。
        if (newComponent) {
            m_components.insert(reinterpret_cast<intptr_t>(newComponent), newComponent);
        }

        // 3. 返回创建的实例
        return newComponent;
    }

    // 对于其他简单元件，调用旧的创建函数 (该函数内部已经包含注册逻辑)
    return createComponent(type, pos);
}

/**
 * @brief 创建导线：做多项合法性检查与端点类型规范化。
 * @param startPin 起点引脚（可为输入/输出，内部会规范为输出）
 * @param endPin 终点引脚（将规范为输入）
 * @return 创建成功返回新导线指针，失败返回nullptr
 */
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

/**
 * @brief 运行传播-评估循环，直到稳定或达到最大迭代次数。
 * @details 处理删除导线后的残留状态，通过在每轮开始清零非源头输入引脚修复。
 */
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
/** @return 返回组件映射（键为指针地址） */
const QMap<intptr_t, Component*>& Engine::getAllComponents() const { return m_components; }
/** @return 返回所有导线的数组 */
const QVector<Wire*>& Engine::getAllWires() const { return m_wires; }

/** 删除组件：从Map移除并释放 */
void Engine::deleteComponent(Component* component) {
    if (!component) {
        return; // 如果传入的是空指针，直接返回
    }

    // 1. 使用元件的内存地址作为键，在 m_components 中查找并移除它
    //    reinterpret_cast 用于将指针转换为整数类型的键
    intptr_t componentKey = reinterpret_cast<intptr_t>(component);
    if (m_components.remove(componentKey)) {
        // 2. 如果成功移除了键值对，说明元件确实存在于Map中，
        //    现在可以安全地释放它占用的内存了
        delete component;
    }
}
/** 删除导线并释放 */
void Engine::deleteWire(Wire* wire) { if (!wire) return; m_wires.removeAll(wire); delete wire; }

/**
 * @brief 从JSON加载电路（先清空，再内部加载并simulate）。
 * @param json 完整电路的JSON对象（components + wires）
 * @return 成功返回true，失败返回false
 */
bool Engine::loadCircuitFromJson(const QJsonObject& json)
{
    // 使命A：清空！
    clearAll();

    // 调用底层函数完成加载
    if (loadCircuitInternal(json)) {
        // 加载成功后，运行一次仿真以更新所有引脚的初始状态
        simulate();
        return true;
    }

    return false;
}
/** 清空所有组件与导线 */
void Engine::clearAll() {
    qDeleteAll(m_wires);
    m_wires.clear();
    qDeleteAll(m_components.values());
    m_components.clear();
}

/**
 * @brief 将当前电路序列化为JSON：组件+导线。
 * @return 代表电路的JSON对象
 */
QJsonObject Engine::saveCircuitToJson() const
{
    QJsonObject circuitJson; // 这是最终要返回的JSON总对象
    QJsonArray componentsArray; // 用于存放所有元件信息的数组
    QJsonArray wiresArray;      // 用于存放所有导线信息的数组

    // 1. 遍历所有元件，将它们的信息序列化
    for (Component* comp : m_components.values()) {
        QJsonObject compObject;
        // 使用元件在内存中的地址作为其独一无二的ID
        compObject["id"] = reinterpret_cast<qint64>(comp);
        compObject["type"] = static_cast<int>(comp->type());
        compObject["x"] = comp->position().x();
        compObject["y"] = comp->position().y();
        if (comp->type() == ComponentType::Encapsulated) {
            // 如果是封装元件，额外保存其内部电路的JSON定义
            auto encapsulatedComp = static_cast<EncapsulatedComponent*>(comp);
            compObject["name"] = encapsulatedComp->getName();
            compObject["internal_circuit"] = encapsulatedComp->getInternalJson();
        }
        componentsArray.append(compObject);
    }

    // 2. 遍历所有导线，将它们的信息序列化
    for (Wire* wire : m_wires) {
        QJsonObject wireObject;
        // 记录导线连接的起始元件ID和引脚索引
        wireObject["start_comp_id"] = reinterpret_cast<qint64>(wire->startPin()->owner());
        wireObject["start_pin_index"] = wire->startPin()->index();
        // 记录导线连接的终止元件ID和引脚索引
        wireObject["end_comp_id"] = reinterpret_cast<qint64>(wire->endPin()->owner());
        wireObject["end_pin_index"] = wire->endPin()->index();

        wiresArray.append(wireObject);
    }

    // 3. 将元件数组和导线数组放入总对象中
    circuitJson["components"] = componentsArray;
    circuitJson["wires"] = wiresArray;

    return circuitJson;
}
// ===============================================
// === EncapsulatedComponent 实现
// ===============================================

/** 构造封装元件：创建内部引擎并载入内部电路，随后构建引脚映射 */
EncapsulatedComponent::EncapsulatedComponent(const QPointF& pos, const QString& name, const QJsonObject& internalCircuitJson)
    : Component(ComponentType::Encapsulated, pos, 0, 0),
    m_internalEngine(new Engine()),
    m_internalCircuitJson(internalCircuitJson),
    m_name(name)
{
    // 1. 加载内部电路到迷你引擎
    m_internalEngine->loadCircuitInternal(m_internalCircuitJson);

    // 2. 根据内部电路的 Input/Output 元件，建立引脚映射并创建外部引脚
    buildPinMappings();
}

/** 析构：释放内部引擎 */
EncapsulatedComponent::~EncapsulatedComponent()
{
    delete m_internalEngine;
}

/** 获取封装元件名称 */
QString EncapsulatedComponent::getName() const
{
    return m_name;
}

/**
 * @brief 构建外部引脚与内部Input/Output的映射，并按Y坐标排序保持一致。
 * @return 无返回值
 */
void EncapsulatedComponent::buildPinMappings()
{
    // --- 1. 先收集所有内部的 Input 和 Output 元件 ---
    QVector<Component*> internalInputComps;
    QVector<Component*> internalOutputComps;

    for (Component* comp : m_internalEngine->getAllComponents().values()) {
        if (comp->type() == ComponentType::Input) {
            internalInputComps.append(comp);
        } else if (comp->type() == ComponentType::Output) {
            internalOutputComps.append(comp);
        }
    }

    // --- 2. 【核心修改】根据 Y 坐标对它们进行排序 ---
    // C++ Lambda 表达式用于定义一个临时的比较函数
    std::sort(internalInputComps.begin(), internalInputComps.end(),
              [](const Component* a, const Component* b) {
                  return a->position().y() < b->position().y();
              }
              );
    std::sort(internalOutputComps.begin(), internalOutputComps.end(),
              [](const Component* a, const Component* b) {
                  return a->position().y() < b->position().y();
              }
              );

    // --- 3. 按照排序后的顺序，建立引脚映射 ---
    for (Component* comp : internalInputComps) {
        m_internalInputs.append(comp->outputPins()[0]);
    }
    for (Component* comp : internalOutputComps) {
        m_internalOutputs.append(comp->inputPins()[0]);
    }

    // --- 4. 根据最终的引脚数量，动态创建自己的外部引脚 ---
    for (int i = 0; i < m_internalInputs.size(); ++i) {
        m_inputPins.append(new Pin(this, Pin::Input, i));
    }
    for (int i = 0; i < m_internalOutputs.size(); ++i) {
        m_outputPins.append(new Pin(this, Pin::Output, i));
    }
}

/**
 * @brief 评估封装元件：
 * 1) 同步外部输入到内部Input；2) 运行内部引擎；3) 回填内部Output到外部输出。
 * @return 无返回值
 */
void EncapsulatedComponent::evaluate()
{
    // 1. 将外部输入引脚的状态，传递给内部电路对应的 Input 元件
    for (int i = 0; i < m_inputPins.size(); ++i) {
        bool externalState = m_inputPins[i]->getState();
        Component* internalInputComp = m_internalInputs[i]->owner();
        // 我们需要一种方式来直接设置 Input 元件的状态
        // 我们去给 Input 类加一个 setState 方法
        static_cast<Input*>(internalInputComp)->setState(externalState);
    }

    // 2. 运行内部电路的仿真
    m_internalEngine->simulate();

    // 3. 从内部电路的 Output 元件获取状态，设置到自己的外部输出引脚上
    for (int i = 0; i < m_outputPins.size(); ++i) {
        bool internalState = m_internalOutputs[i]->getState();
        m_outputPins[i]->setState(internalState);
    }
}

/** @return 内部电路定义JSON（只读引用） */
const QJsonObject& EncapsulatedComponent::getInternalJson() const
{
    return m_internalCircuitJson;
}

/**
 * @brief 手动注册一个外部创建的组件（封装元件等）。
 * @param component 需要注册到引擎管理的组件指针
 * @return 无返回值
 */
void Engine::registerComponent(Component* component)
{
    if (component) {
        m_components.insert(reinterpret_cast<intptr_t>(component), component);
    }
}
/**
 * @brief 内部加载函数：不清空现有内容，使用ID映射重建组件与导线。
 * @param json 完整电路JSON
 * @return 成功返回true，失败返回false
 */
bool Engine::loadCircuitInternal(const QJsonObject& json)
{
    // 【核心】没有 clearAll()
    if (!json.contains("components") || !json["components"].isArray()) {
        qWarning("JSON load error: 'components' array not found or is not an array.");
        return false;
    }

    QMap<qint64, Component*> idMap;
    const QJsonArray componentsArray = json["components"].toArray();

    for (const QJsonValue &compValue : componentsArray) {
        QJsonObject compObject = compValue.toObject();
        qint64 id = compObject["id"].toInteger();
        Component* newComponent = createComponent(compObject);
        if (newComponent) {
            idMap[id] = newComponent;
        } else {
            // 清理已创建的元件以防内存泄漏
            qDeleteAll(m_components.values());
            m_components.clear();
            return false;
        }
    }

    if (json.contains("wires") && json["wires"].isArray()) {
        const QJsonArray wiresArray = json["wires"].toArray();
        for (const QJsonValue &wireValue : wiresArray) {
            QJsonObject wireObject = wireValue.toObject();
            qint64 startCompId = wireObject["start_comp_id"].toInteger();
            int startPinIndex = wireObject["start_pin_index"].toInt();
            qint64 endCompId = wireObject["end_comp_id"].toInteger();
            int endPinIndex = wireObject["end_pin_index"].toInt();

            // 使用idMap，通过旧ID找到我们刚刚创建的新元件
            Component* startComp = idMap.value(startCompId);
            Component* endComp = idMap.value(endCompId);

            // 健壮性检查：确保元件和引脚都有效
            if (!startComp || !endComp || startPinIndex >= startComp->outputPins().size() || endPinIndex >= endComp->inputPins().size()) {
                clearAll(); // 数据无效，回滚
                return false;
            }

            // 创建新的导线并添加到引擎的导线列表中
            Wire* newWire = new Wire(startComp->outputPins()[startPinIndex], endComp->inputPins()[endPinIndex]);
            m_wires.append(newWire);
        }

    }
    return true;
}
