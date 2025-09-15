#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QActionGroup>

/**
 * @file mainwindow.h
 * @brief 【C同学负责】定义了应用程序的主窗口类。
 * @details 定义了程序的主UI框架以及响应用户操作的槽函数。
 */

#include <QMainWindow>
#include "engine.h" // 需要包含engine.h来使用公共枚举ComponentType
#include "graphics.h"
// --- 前向声明 ---
class Engine;
class GraphicsScene;
class QActionGroup;


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 这些槽函数将由Qt自动连接到UI设计器里的同名Action
    void on_actionAdd_Input_triggered();
    void on_actionAdd_Output_triggered();
    void on_actionAdd_AndGate_triggered();
    void on_actionAdd_OrGate_triggered();
    void on_actionAdd_NotGate_triggered();
    void on_actionAdd_NandGate_triggered();
    void on_actionAdd_NorGate_triggered();
    void on_actionAdd_XorGate_triggered();
    void on_actionAdd_XnorGate_triggered();
    void on_actionNew_Tab_triggered();
    void on_actionClear_triggered();
    void onComponentPlaced();
    // ... 其他功能按钮的槽函数声明 ...

    void on_actionSave_triggered();

    void on_actionOpen_triggered();

    void on_actionEncapsulate_triggered();
    void onNewTab();
    void onTabClose(int index);

    void onCustomComponentActionTriggered();

    // 【新增】这个槽函数将用于响应在工具栏上的右键点击
    void onCustomComponentToolbarContextMenuRequested(const QPoint &pos);

private:
    Ui::MainWindow *ui;

    QActionGroup *m_addComponentActionGroup;

    void populateCustomComponentToolbar();
    GraphicsScene* currentScene();
    Engine* currentEngine();
};
#endif // MAINWINDOW_H
