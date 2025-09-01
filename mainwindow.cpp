#include "mainwindow.h"
#include "engine.h"
#include "graphics.h"
#include "ui_mainwindow.h"

/**
 * @file mainwindow.cpp
 * @brief 【C同学负责】实现主窗口的所有功能。
 * @details 你将在这里实例化A和B的模块，并将UI上用户的操作“翻译”成对其他模块的调用。
 */

// 目的: 构造主窗口，完成所有模块的初始化和“粘合”工作。
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 实例化A同学的引擎模块
    m_engine = new Engine();
    // 2. 实例化B同学的图形模块，并把引擎的“遥控器”交给它
    m_scene = new GraphicsScene(m_engine, this);

    // 3. 将B同学的画布“安装”到UI界面上
    ui->graphicsView->setScene(m_scene);
}

// 目的: 销毁主窗口时，清理我们手动创建的对象，防止内存泄漏。
MainWindow::~MainWindow()
{
    delete ui;
    // C同学负责清理他创建的模块
    delete m_engine;
    // m_scene 会被Qt作为子对象自动管理销毁
}

// --- 槽函数实现 ---

/**
 * @brief 响应用户点击“添加输入源”按钮。
 * @利用   调用B同学场景的公共接口，命令其进入“添加元件”模式。
 * @param  - 【输入】无。此函数由Qt信号槽机制自动调用。
 * @return - 【输出】无。
 */
void MainWindow::on_actionAdd_Input_triggered()
{
    m_scene->setComponentTypeToAdd(ComponentType::Input);
    m_scene->setMode(GraphicsScene::Mode::AddingComponent);
}
void MainWindow::on_actionAdd_Output_triggered()
{
    m_scene->setComponentTypeToAdd(ComponentType::Output);
    m_scene->setMode(GraphicsScene::AddingComponent);
}
void MainWindow::on_actionAdd_AndGate_triggered()
{
    m_scene->setComponentTypeToAdd(ComponentType::And);
    m_scene->setMode(GraphicsScene::AddingComponent);
}
void MainWindow::on_actionAdd_OrGate_triggered()
{
    m_scene->setComponentTypeToAdd(ComponentType::Or);
    m_scene->setMode(GraphicsScene::AddingComponent);
}
void MainWindow::on_actionAdd_NotGate_triggered()
{
    m_scene->setComponentTypeToAdd(ComponentType::Not);
    m_scene->setMode(GraphicsScene::AddingComponent);
}
void MainWindow::on_actionAdd_NandGate_triggered()
{
    m_scene->setComponentTypeToAdd(ComponentType::Nand);
    m_scene->setMode(GraphicsScene::AddingComponent);
}
void MainWindow::on_actionAdd_NorGate_triggered()
{
    m_scene->setComponentTypeToAdd(ComponentType::Nor);
    m_scene->setMode(GraphicsScene::AddingComponent);
}
void MainWindow::on_actionAdd_XorGate_triggered()
{
    m_scene->setComponentTypeToAdd(ComponentType::Xor);
    m_scene->setMode(GraphicsScene::AddingComponent);
}
void MainWindow::on_actionAdd_XnorGate_triggered()
{
    m_scene->setComponentTypeToAdd(ComponentType::Xnor);
    m_scene->setMode(GraphicsScene::AddingComponent);
}

// C同学将在这里填充 on_actionSave_triggered 等其他功能的槽函数实现
