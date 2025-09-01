#ifndef ENGINE_H
#define ENGINE_H

/**
 * @file engine.h
 * @brief 【A同学负责】定义项目的后台核心，包括所有数据结构和模拟引擎。
 * @details 该文件是项目的“数据蓝图”，定义了所有电路元件的逻辑表示。
 *          它完全独立于任何图形界面。B和C同学需要包含此文件来与引擎交互。
 */

#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QStringList>
#include <QVector>

// --- 前向声明 ---
class Pin;
class Component;
class Wire;
class Circuit;
class ComponentItem;

// =============================================================
// == 公共枚举: ComponentType
// == 目的: 定义一个全局统一的元件类型列表。
// == 使用: 这是调用 createComponent 函数时，'type'参数的【所有合法选项】。
// =============================================================
enum class ComponentType {
    Input,
    Output,
    And,
    Or,
    Not,
    Nand,
    Nor,
    Xor,
    Xnor,
    SubCircuit // 为封装功能预留
};

// =============================================================
// == 类: Pin
// == 目的: 表示元件的输入或输出引脚。
// =============================================================
class Pin
{
public:
    enum PinType { Input, Output };
    Pin(Component *owner, PinType type, int index);
    bool getState() const;
    void setState(bool state);
    Component *owner() const;
    int index() const;
    QPointF getScenePos() const;

private:
    Component *m_owner;
    PinType m_type;
    int m_index;
    bool m_state;
};

// =============================================================
// == 类: Component (抽象基类)
// == 目的: 所有电路元件的“共同祖先”。
// == 职责: 定义所有元件的通用属性和行为。
// =============================================================
class Component
{
    friend class Pin;

public:
    Component(ComponentType type, const QPointF &position, int numInputs, int numOutputs);
    virtual ~Component();
    virtual void evaluate() = 0;
    ComponentType type() const;
    const QVector<Pin *> &inputPins() const;
    const QVector<Pin *> &outputPins() const;
    void setGraphicsItem(ComponentItem *item);
    ComponentItem *getGraphicsItem() const;
    QPointF position() const;

protected:
    ComponentType m_type;
    QPointF m_position;
    QVector<Pin *> m_inputPins;
    QVector<Pin *> m_outputPins;
    ComponentItem *m_graphicsItem;
};

// =============================================================
// == 【契约】所有具体的元件类声明
// == 目的: 明确声明引擎支持创建的所有基础元件类型。
// =============================================================
class Input : public Component
{
public:
    Input(const QPointF &pos);
    void evaluate() override;
};
class Output : public Component
{
public:
    Output(const QPointF &pos);
    void evaluate() override;
};
class AndGate : public Component
{
public:
    AndGate(const QPointF &pos);
    void evaluate() override;
};
class OrGate : public Component
{
public:
    OrGate(const QPointF &pos);
    void evaluate() override;
};
class NotGate : public Component
{
public:
    NotGate(const QPointF &pos);
    void evaluate() override;
};
class NandGate : public Component
{
public:
    NandGate(const QPointF &pos);
    void evaluate() override;
};
class NorGate : public Component
{
public:
    NorGate(const QPointF &pos);
    void evaluate() override;
};
class XorGate : public Component
{
public:
    XorGate(const QPointF &pos);
    void evaluate() override;
};
class XnorGate : public Component
{
public:
    XnorGate(const QPointF &pos);
    void evaluate() override;
};

// =============================================================
// == 类: Engine
// == 目的: 项目的后台核心，唯一的“数据中心”和“计算中心”。
// =============================================================
class Engine
{
public:
    Engine();
    ~Engine();

    /**
     * @brief 在后台创建一个新的元件数据实例。
     * @param type - 【输入】要创建的元件类型。
     *               【合法值】: 必须是 ComponentType 枚举中定义的任何一个值,
     *                          例如 ComponentType::And, ComponentType::Input 等。
     * @param pos  - 【输入】元件在场景中的初始位置坐标。
     * @return     - 【输出】指向新创建的Component对象的指针。
     */
    Component *createComponent(ComponentType type, const QPointF &pos);

    // ... 其他未来需要的函数声明 ...
};

#endif // ENGINE_H
