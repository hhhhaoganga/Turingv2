#ifndef ENGINE_H
#define ENGINE_H
#include <QJsonObject>
#include <QVector>
#include <QPointF>
#include <QMap>

// --- 前向声明 ---
class Pin;
class Component;
class Wire;
class ComponentItem;
class EncapsulatedComponent;
// ===============================================
// 枚举与类的定义 (严格按照成熟版本)
// ===============================================

enum class ComponentType {
    Input, Output, And, Or, Not, Nand, Nor, Xor, Xnor, Encapsulated
};

class Pin {
public:
    enum PinType { Input, Output };
    Pin(Component* owner, PinType type, int index);
    bool getState() const;
    void setState(bool state);
    Component* owner() const;
    PinType type() const;
    int index() const;
    QPointF getScenePos() const;
private:
    Component* m_owner;
    PinType m_type;
    int m_index;
    bool m_state;
};

class Wire {
public:
    Wire(Pin* start, Pin* end);
    Pin* startPin() const;
    Pin* endPin() const;
    bool getState() const;
private:
    Pin* m_startPin;
    Pin* m_endPin;
};

class Component {
public:
    Component(ComponentType type, const QPointF& position, int numInputs, int numOutputs);
    virtual ~Component();
    virtual void evaluate() = 0;
    ComponentType type() const;
    const QVector<Pin*>& inputPins() const;
    const QVector<Pin*>& outputPins() const;
    void setGraphicsItem(ComponentItem* item);
    ComponentItem* getGraphicsItem() const;
    void setPosition(const QPointF& pos);
    QPointF position() const;
protected:
    ComponentType m_type;
    QPointF m_position;
    QVector<Pin*> m_inputPins;
    QVector<Pin*> m_outputPins;
    ComponentItem* m_graphicsItem;
};

// --- 具体元件类声明 ---
class Input : public Component {
public:
    Input(const QPointF& pos);
    void evaluate() override;
    void toggleState();
    void setState(bool state);
private:
    bool m_currentState;
};
class Output : public Component { public: Output(const QPointF& pos); void evaluate() override; };
class AndGate : public Component { public: AndGate(const QPointF& pos); void evaluate() override; };
class OrGate : public Component { public: OrGate(const QPointF& pos); void evaluate() override; };
class NotGate : public Component { public: NotGate(const QPointF& pos); void evaluate() override; };
class NandGate : public Component { public: NandGate(const QPointF& pos); void evaluate() override; };
class NorGate : public Component { public: NorGate(const QPointF& pos); void evaluate() override; };
class XorGate : public Component { public: XorGate(const QPointF& pos); void evaluate() override; };
class XnorGate : public Component { public: XnorGate(const QPointF& pos); void evaluate() override; };

class Engine {
public:
    Engine();
    ~Engine();
    Component* createComponent(ComponentType type, const QPointF& pos);
    Component* createComponent(const QJsonObject& compObject);
    Wire* createWire(Pin* startPin, Pin* endPin);
    void simulate();
    const QMap<intptr_t, Component*>& getAllComponents() const;
    const QVector<Wire*>& getAllWires() const;
    void deleteComponent(Component* component);
    void deleteWire(Wire* wire);
    bool loadCircuitFromJson(const QJsonObject& json);
    void clearAll();
    void registerComponent(Component* component);
    QJsonObject saveCircuitToJson() const;
    QJsonObject saveComponentsToJson(const QVector<Component*>& components) const;
    friend class EncapsulatedComponent;
private:
    QMap<intptr_t, Component*> m_components;
    QVector<Wire*> m_wires;
    bool loadCircuitInternal(const QJsonObject& json);
};
class EncapsulatedComponent : public Component {
public:
    EncapsulatedComponent(const QPointF& pos, const QString& name, const QJsonObject& internalCircuitJson);
    ~EncapsulatedComponent() override;

    // 核心评估函数
    void evaluate() override;

    // 用于保存/加载
    const QJsonObject& getInternalJson() const;

    QString getName() const;

private:
    void buildPinMappings();

    // 每个封装元件内部都有一个自己的“迷你引擎”
    Engine* m_internalEngine;
    // 保存内部电路的定义，用于序列化
    QJsonObject m_internalCircuitJson;
    // 【新增】成员变量来存储名字
    QString m_name;

    // 外部引脚到内部引脚的映射
    QVector<Pin*> m_internalInputs;  // 指向内部 Input 元件的输出引脚
    QVector<Pin*> m_internalOutputs; // 指向内部 Output 元件的输入引脚
};

#endif // ENGINE_H
