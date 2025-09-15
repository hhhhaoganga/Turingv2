#ifndef ENGINE_H
#define ENGINE_H
#include <QJsonObject>  // JSON 序列化/反序列化的数据结构
#include <QVector>      // 动态数组容器（用于保存引脚/导线等）
#include <QPointF>      // 场景中的二维坐标
#include <QMap>         // 组件映射（以指针地址为键）

/**
 * @brief 前向声明以减少编译依赖。
 */
class Pin;
class Component;
class Wire;
class ComponentItem;
class EncapsulatedComponent;
// ===============================================
// 枚举与类的定义 (严格按照成熟版本)
// ===============================================

/**
 * @brief 组件类型的枚举。
 * - Input/Output: 输入输出端
 * - And/Or/Not/Nand/Nor/Xor/Xnor: 基本逻辑门
 * - Encapsulated: 封装组件（内部含子电路）
 */
enum class ComponentType {
    Input, Output, And, Or, Not, Nand, Nor, Xor, Xnor, Encapsulated
};

/**
 * @brief 引脚，表示组件的输入或输出端口。
 */
class Pin {
public:
    /** 引脚类型：输入或输出 */
    enum PinType { Input, Output };
    /**
     * @brief 构造函数。
     * @param owner 所属组件
     * @param type 引脚类型
     * @param index 引脚在所属组件中的序号
     */
    Pin(Component* owner, PinType type, int index);
    /** 获取当前逻辑状态 */
    bool getState() const;
    /** 设置当前逻辑状态 */
    void setState(bool state);
    /** 获取所属组件 */
    Component* owner() const;
    /** 获取引脚类型 */
    PinType type() const;
    /** 获取引脚索引 */
    int index() const;
    /**
     * @brief 在场景坐标系中的位置。
     * @details 依赖其所属 `ComponentItem` 的几何映射。
     */
    QPointF getScenePos() const;
private:
    /** 所属组件指针（非拥有） */
    Component* m_owner;
    /** 引脚类型 */
    PinType m_type;
    /** 引脚索引（自0递增） */
    int m_index;
    /** 当前逻辑电平 */
    bool m_state;
};

/**
 * @brief 导线，连接一个输出引脚到一个输入引脚。
 */
class Wire {
public:
    /**
     * @brief 构造函数。
     * @param start 起始输出引脚
     * @param end 终止输入引脚
     */
    Wire(Pin* start, Pin* end);
    /** 获取起始引脚 */
    Pin* startPin() const;
    /** 获取终止引脚 */
    Pin* endPin() const;
    /**
     * @brief 获取导线上传播的状态。
     * @details 等价于起始引脚的状态。
     */
    bool getState() const;
private:
    /** 起始输出引脚（非拥有） */
    Pin* m_startPin;
    /** 终止输入引脚（非拥有） */
    Pin* m_endPin;
};

/**
 * @brief 组件基类，抽象出通用的引脚与位置等信息。
 */
class Component {
public:
    /**
     * @brief 构造函数。
     * @param type 组件类型
     * @param position 场景中的位置
     * @param numInputs 输入引脚数量
     * @param numOutputs 输出引脚数量
     */
    Component(ComponentType type, const QPointF& position, int numInputs, int numOutputs);
    /** 虚析构，释放引脚 */
    virtual ~Component();
    /** 计算组件输出（纯虚） */
    virtual void evaluate() = 0;
    /** 获取组件类型 */
    ComponentType type() const;
    /** 获取输入引脚数组（只读） */
    const QVector<Pin*>& inputPins() const;
    /** 获取输出引脚数组（只读） */
    const QVector<Pin*>& outputPins() const;
    /** 绑定图形项（前端） */
    void setGraphicsItem(ComponentItem* item);
    /** 获取绑定的图形项 */
    ComponentItem* getGraphicsItem() const;
    /** 设置组件位置 */
    void setPosition(const QPointF& pos);
    /** 获取组件位置 */
    QPointF position() const;
protected:
    /** 组件类型 */
    ComponentType m_type;
    /** 场景位置 */
    QPointF m_position;
    /** 输入引脚集合（拥有） */
    QVector<Pin*> m_inputPins;
    /** 输出引脚集合（拥有） */
    QVector<Pin*> m_outputPins;
    /** 对应的图形项指针（非拥有） */
    ComponentItem* m_graphicsItem;
};

// --- 具体元件类声明 ---
/**
 * @brief 输入源组件，可手动切换状态。
 */
class Input : public Component {
public:
    /** 构造输入组件 */
    Input(const QPointF& pos);
    /** 输出当前内部状态到输出引脚 */
    void evaluate() override;
    /** 翻转当前状态并立即计算 */
    void toggleState();
    /** 设置当前状态并立即计算 */
    void setState(bool state);
private:
    /** 当前内部状态 */
    bool m_currentState;
};
/** 输出端组件，显示输入状态。*/
class Output : public Component { public: /** 构造输出组件 */ Output(const QPointF& pos); /** 输出由输入决定 */ void evaluate() override; };
/** 与门 */
class AndGate : public Component { public: /** 构造与门 */ AndGate(const QPointF& pos); /** 计算与 */ void evaluate() override; };
/** 或门 */
class OrGate : public Component { public: /** 构造或门 */ OrGate(const QPointF& pos); /** 计算或 */ void evaluate() override; };
/** 非门 */
class NotGate : public Component { public: /** 构造非门 */ NotGate(const QPointF& pos); /** 计算非 */ void evaluate() override; };
/** 与非门 */
class NandGate : public Component { public: /** 构造与非门 */ NandGate(const QPointF& pos); /** 计算与非 */ void evaluate() override; };
/** 或非门 */
class NorGate : public Component { public: /** 构造或非门 */ NorGate(const QPointF& pos); /** 计算或非 */ void evaluate() override; };
/** 异或门 */
class XorGate : public Component { public: /** 构造异或门 */ XorGate(const QPointF& pos); /** 计算异或 */ void evaluate() override; };
/** 同或门 */
class XnorGate : public Component { public: /** 构造同或门 */ XnorGate(const QPointF& pos); /** 计算同或 */ void evaluate() override; };

/**
 * @brief 引擎，负责组件/导线的创建、删除与逻辑仿真，以及JSON序列化。
 */
class Engine {
public:
    /** 构造函数 */
    Engine();
    /** 析构函数，释放组件与导线 */
    ~Engine();
    /**
     * @brief 工厂：创建一个组件并注册到引擎。
     * @param type 组件类型
     * @param pos 组件位置
     */
    Component* createComponent(ComponentType type, const QPointF& pos);
    /**
     * @brief 从JSON对象创建组件（支持封装元件）。
     */
    Component* createComponent(const QJsonObject& compObject);
    /**
     * @brief 创建一条导线并注册。
     * @param startPin 起点（输出引脚）
     * @param endPin 终点（输入引脚）
     */
    Wire* createWire(Pin* startPin, Pin* endPin);
    /** 运行一次稳定化仿真 */
    void simulate();
    /** 获取所有组件映射（键为指针地址） */
    const QMap<intptr_t, Component*>& getAllComponents() const;
    /** 获取所有导线 */
    const QVector<Wire*>& getAllWires() const;
    /** 删除一个组件（连带移除Map记录） */
    void deleteComponent(Component* component);
    /** 删除一条导线 */
    void deleteWire(Wire* wire);
    /** 从JSON加载电路（会先清空） */
    bool loadCircuitFromJson(const QJsonObject& json);
    /** 清理所有组件与导线 */
    void clearAll();
    /** 手动注册外部创建的组件（如封装元件） */
    void registerComponent(Component* component);
    /** 保存完整电路为JSON */
    QJsonObject saveCircuitToJson() const;
    /** 保存给定组件集合为JSON（保留接口） */
    QJsonObject saveComponentsToJson(const QVector<Component*>& components) const;
    friend class EncapsulatedComponent;
private:
    /** 组件集合（拥有） */
    QMap<intptr_t, Component*> m_components;
    /** 导线集合（拥有） */
    QVector<Wire*> m_wires;
    /**
     * @brief 内部加载函数（不清空已存在内容）。
     * @details 用于封装元件内部引擎的构建。
     */
    bool loadCircuitInternal(const QJsonObject& json);
};
/**
 * @brief 封装组件：包含一套内部电路，外部以若干输入/输出引脚暴露。
 */
class EncapsulatedComponent : public Component {
public:
    /**
     * @brief 构造封装组件。
     * @param pos 外部组件位置
     * @param name 组件名称（用于显示/识别）
     * @param internalCircuitJson 内部电路定义
     */
    EncapsulatedComponent(const QPointF& pos, const QString& name, const QJsonObject& internalCircuitJson);
    /** 析构函数，释放内部引擎 */
    ~EncapsulatedComponent() override;

    /** 核心评估：同步外部输入→内部，运行内部仿真，再回填外部输出 */
    void evaluate() override;

    /** 获取内部电路JSON（只读引用） */
    const QJsonObject& getInternalJson() const;

    /** 获取封装组件名称 */
    QString getName() const;

private:
    /** 根据内部电路自动构建外部引脚与内部引脚的映射 */
    void buildPinMappings();

    /** 内部迷你引擎（拥有） */
    Engine* m_internalEngine;
    /** 内部电路定义（用于序列化） */
    QJsonObject m_internalCircuitJson;
    /** 组件名称 */
    QString m_name;

    /** 指向内部 Input 元件的输出引脚（作为外部输入的源） */
    QVector<Pin*> m_internalInputs;
    /** 指向内部 Output 元件的输入引脚（作为外部输出的汇） */
    QVector<Pin*> m_internalOutputs;
};

#endif // ENGINE_H
