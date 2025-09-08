#include "mainwindow.h"
#include "engine.h"
#include "graphics.h"
#include "ui_mainwindow.h"
#include <QActionGroup>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>  // 用于输入对话框
#include <QJsonDocument> // 用于处理JSON数据
#include <QJsonObject>
#include <QFile>         // 用于文件读写
#include <QFileInfo>

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
    //4.
    m_addComponentActionGroup = new QActionGroup(this);
    m_addComponentActionGroup->addAction(ui->actionAdd_Input);
    m_addComponentActionGroup->addAction(ui->actionAdd_Output);
    m_addComponentActionGroup->addAction(ui->actionAdd_AndGate);
    m_addComponentActionGroup->addAction(ui->actionAdd_OrGate);
    m_addComponentActionGroup->addAction(ui->actionAdd_NotGate);
    m_addComponentActionGroup->addAction(ui->actionAdd_NandGate);
    m_addComponentActionGroup->addAction(ui->actionAdd_NorGate);
    m_addComponentActionGroup->addAction(ui->actionAdd_XorGate);
    m_addComponentActionGroup->addAction(ui->actionAdd_XnorGate);
    m_addComponentActionGroup->setExclusive(true);
    connect(m_scene, &GraphicsScene::componentAdded, this, &MainWindow::onComponentPlaced);

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
void MainWindow::on_actionClear_triggered(){
     if (m_engine) {
         m_engine->clearAll();
      }
      if(m_scene){
          m_scene->clear();
      }
  }

void MainWindow::onComponentPlaced()
{
    // 检查当前是否有被按下的(checked)动作
    QAction* checked_action = m_addComponentActionGroup->checkedAction();
    if (checked_action) {
        // 如果有，就取消它的选中状态。
        // 关键：第二个参数 false 表示“不要触发此Action的信号”，
        // 避免了可能产生的逻辑循环。
        checked_action->setChecked(false);
    }

    // 将状态栏恢复到“准备就绪”
    ui->statusbar->showMessage("准备就绪");
}

// C同学将在这里填充 on_actionSave_triggered 等其他功能的槽函数实现

void MainWindow::on_actionSave_triggered()
{static QString lastUsedDir = "";
    QString selectedFilter;
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "保存电路文件", // 对话框的标题文字
        lastUsedDir + "/untitled.json",  // 默认的文件名
        "JSON 文件 (*.json);;所有文件 (*.*)",
        &selectedFilter
        ) ; // 文件类型过滤器，用";;"分隔多个类型
    if (filePath.isEmpty()) {
        ui->statusbar->showMessage("保存操作已取消", 3000); // 在状态栏提示一下，持续3秒
        return;
    }
    if (selectedFilter == "JSON 文件 (*.json)" && !filePath.endsWith(".json", Qt::CaseInsensitive)) {
        filePath += ".json";
    }
    lastUsedDir = QFileInfo(filePath).path();
    QString message = QString("文件将保存到:\n%1").arg(filePath);
    QMessageBox::information(this, "保存路径", message);
    ui->statusbar->showMessage("文件已“保存”到: " + filePath, 5000);

}


void MainWindow::on_actionOpen_triggered()
{    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "确认操作",
                                  "加载新文件将会覆盖当前未保存的电路，是否继续？",
                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (reply == QMessageBox::No) {
        // 如果选择“否”，他可能想先保存。我们可以顺便调用一下保存函数。
        on_actionSave_triggered();
        return; // 保存后，本次“打开”操作终止，让用户重新决定
    } else if (reply == QMessageBox::Cancel) {
        // 如果用户选择“取消”，则什么都不做
        ui->statusbar->showMessage("打开操作已取消", 3000);
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(
        this,
        "打开电路文件",
        "", // 默认目录
        "JSON 文件 (*.json);;所有文件 (*.*)"
        );
    if (filePath.isEmpty()) {
        ui->statusbar->showMessage("打开操作已取消", 3000);
        return;}
    QFile loadFile(filePath);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "打开错误", "无法打开文件进行读取！");
        return;}
    QByteArray fileData = loadFile.readAll();
    loadFile.close();
    QJsonDocument loadDoc = QJsonDocument::fromJson(fileData);
    if (loadDoc.isNull() || !loadDoc.isObject()) {
        QMessageBox::critical(this, "解析错误", "文件不是一个有效的JSON对象！");
        return;
    }
    QJsonObject circuitJson = loadDoc.object();
    if (!m_engine) return;
    if (m_engine->loadCircuitFromJson(circuitJson)) {
        // 如果后台引擎成功加载了数据...

        //  命令前台画布根据引擎的新状态重绘
        if (!m_scene) return; // 健壮性检查
        m_scene->rebuildSceneFromEngine();

        // 给出成功反馈
        ui->statusbar->showMessage("电路已成功从 " + filePath + " 加载", 5000);

    } else {
        // 返回false，说明加载失败
        QMessageBox::critical(this, "加载失败", "文件内容格式错误或数据不兼容，无法加载。");
    }
}




void MainWindow::on_actionEncapsulate_triggered()
{

}

