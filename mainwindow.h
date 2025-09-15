#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>  // 主窗口基类
#include <QActionGroup> // 互斥动作组，用于元件添加动作

/**
 * @file mainwindow.h
 * @brief 主窗口类定义：负责多标签页画布与菜单/工具栏交互。
 */

#include <QMainWindow> // 再次包含以确保MOC一致性（无功能影响）
#include "engine.h"   // 使用 ComponentType 与 Engine 接口
#include "graphics.h" // 使用 GraphicsScene 类型
// --- 前向声明 ---
class Engine;
class GraphicsScene;
class QActionGroup;


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
 * @brief 主窗口，管理多标签页电路画布与文件/元件操作。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    /** 构造主窗口并初始化UI与工具栏/标签页 */
    MainWindow(QWidget *parent = nullptr);
    /** 析构，释放UI对象 */
    ~MainWindow();

private slots:
    // 由Qt自动连接到UI设计器里的同名Action
    /** 添加一个输入元件 */
    void on_actionAdd_Input_triggered();
    /** 添加一个输出元件 */
    void on_actionAdd_Output_triggered();
    /** 添加与门 */
    void on_actionAdd_AndGate_triggered();
    /** 添加或门 */
    void on_actionAdd_OrGate_triggered();
    /** 添加非门 */
    void on_actionAdd_NotGate_triggered();
    /** 添加与非门 */
    void on_actionAdd_NandGate_triggered();
    /** 添加或非门 */
    void on_actionAdd_NorGate_triggered();
    /** 添加异或门 */
    void on_actionAdd_XorGate_triggered();
    /** 添加同或门 */
    void on_actionAdd_XnorGate_triggered();
    /** 新建标签页 */
    void on_actionNew_Tab_triggered();
    /** 清空当前画布 */
    void on_actionClear_triggered();
    /** 在元件放置后重置工具栏按钮状态 */
    void onComponentPlaced();
    // ... 其他功能按钮的槽函数声明 ...

    /** 保存当前电路到JSON文件 */
    void on_actionSave_triggered();

    /** 打开JSON文件到新标签页 */
    void on_actionOpen_triggered();

    /** 将当前电路封装为自定义元件并保存到库 */
    void on_actionEncapsulate_triggered();
    /** 新建一个标签页（内部复用） */
    void onNewTab();
    /** 关闭指定索引的标签页 */
    void onTabClose(int index);

    /** 点击自定义元件按钮（从库加载） */
    void onCustomComponentActionTriggered();

    // 【新增】这个槽函数将用于响应在工具栏上的右键点击
    /** 自定义元件工具栏的右键菜单（删除库中元件） */
    void onCustomComponentToolbarContextMenuRequested(const QPoint &pos);

private:
    Ui::MainWindow *ui;

    QActionGroup *m_addComponentActionGroup;
    /** 扫描自定义库并填充到工具栏 */
    void populateCustomComponentToolbar();
    /** 获取当前标签页的场景指针 */
    GraphicsScene* currentScene();
    /** 获取当前标签页的引擎指针 */
    Engine* currentEngine();
};
#endif // MAINWINDOW_H
