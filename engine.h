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

// ===============================================
// 枚举与类的定义 (严格按照成熟版本)
// ===============================================

enum class ComponentType {
    Input, Output, And, Or, Not, Nand, Nor, Xor, Xnor
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
    Wire* createWire(Pin* startPin, Pin* endPin);
    void simulate();
    const QMap<intptr_t, Component*>& getAllComponents() const;
    const QVector<Wire*>& getAllWires() const;
    void deleteComponent(Component* component);
    void deleteWire(Wire* wire);
    bool loadCircuitFromJson(const QJsonObject& json);
    void clearAll();
private:
    QMap<intptr_t, Component*> m_components;
    QVector<Wire*> m_wires;
};


#endif // ENGINE_H
