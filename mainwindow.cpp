#include "mainwindow.h"
#include "engine.h"
#include "graphics.h"
#include "ui_mainwindow.h"
#include <QActionGroup>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QTabWidget>
#include <QToolButton>
#include <QPainter> // 为了设置 QGraphicsView 的渲染提示
#include <QGraphicsView>

/**
 * @file mainwindow.cpp
 * @brief 【C同学负责】实现主窗口的所有功能，已升级为多标签页架构。
 * @details 你将在这里实例化A和B的模块，并将UI上用户的操作“翻译”成对其他模块的调用。
 */
void MainWindow::on_actionNew_Tab_triggered()
{
    onNewTab(); // 直接调用我们已有的 onNewTab() 逻辑即可
}
// 目的: 构造主窗口，完成所有模块的初始化和“粘合”工作。
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 清空UI编辑器为预览而生成的占位符Tab页
    while (ui->tabWidget->count() > 0) {
        ui->tabWidget->removeTab(0);
    }

    // 2. 初始化元件添加按钮组
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

    // 3. 设置TabWidget的功能
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabClose);

    // 4. 启动时自动创建一个空白标签页
    onNewTab();
}

// 目的: 销毁主窗口时，清理我们手动创建的对象，防止内存泄漏。
MainWindow::~MainWindow()
{
    delete ui;
    // 由于 Engine 和 Scene 的生命周期已与Tab页绑定，此处无需再手动清理
}

// === 辅助函数：获取当前活动标签页的 Engine 和 Scene ===

GraphicsScene* MainWindow::currentScene() {
    if (ui->tabWidget->currentIndex() == -1) {
        return nullptr;
    }
    QGraphicsView* view = qobject_cast<QGraphicsView*>(ui->tabWidget->currentWidget());
    if (view) {
        return qobject_cast<GraphicsScene*>(view->scene());
    }
    return nullptr;
}

Engine* MainWindow::currentEngine() {
    GraphicsScene* scene = currentScene();
    if (scene) {
        return scene->getEngine();
    }
    return nullptr;
}


// === 标签页管理槽函数 ===

void MainWindow::onNewTab()
{
    // 1. 为新标签页创建一套独立的 Engine 和 Scene
    Engine* engine = new Engine();
    GraphicsScene* scene = new GraphicsScene(engine, this); // 将 engine 传入

    // 2. 将 Scene 安装到一个 QGraphicsView 中
    QGraphicsView* view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing); // 让视图渲染更平滑，观感更好

    // 3. 把这个 view 添加为一个新的标签页
    QString tabName = QString("电路 %1").arg(ui->tabWidget->count() + 1);
    int index = ui->tabWidget->addTab(view, tabName);

    // 4. 自动切换到这个新创建的标签页
    ui->tabWidget->setCurrentIndex(index);

    // 5. 连接 componentAdded 信号，以便在放置元件后取消工具栏按钮的选中状态
    connect(scene, &GraphicsScene::componentAdded, this, &MainWindow::onComponentPlaced);
}

void MainWindow::onTabClose(int index)
{
    // 在关闭前，可以增加“是否保存”的提示，这里为了简化先直接关闭

    // 1. 获取即将被关闭的标签页中的 view
    QGraphicsView* view = qobject_cast<QGraphicsView*>(ui->tabWidget->widget(index));
    if (view) {
        // 2. 通过 view 找到 scene，再找到 engine
        GraphicsScene* scene = qobject_cast<GraphicsScene*>(view->scene());
        Engine* engine = scene->getEngine();

        // 3. 释放后台数据（Engine是我们手动new的，必须手动delete）
        delete engine;

        // 4. 关闭并删除标签页
        // removeTab会删除标签页，而QWidget(view)因为父子关系会被Qt自动清理
        ui->tabWidget->removeTab(index);
    }
}


// === UI Action 槽函数（已全部适配多标签页） ===

void MainWindow::on_actionAdd_Input_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Input);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
void MainWindow::on_actionAdd_Output_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Output);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
void MainWindow::on_actionAdd_AndGate_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::And);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
void MainWindow::on_actionAdd_OrGate_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Or);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
void MainWindow::on_actionAdd_NotGate_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Not);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
void MainWindow::on_actionAdd_NandGate_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Nand);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
void MainWindow::on_actionAdd_NorGate_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Nor);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
void MainWindow::on_actionAdd_XorGate_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Xor);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
void MainWindow::on_actionAdd_XnorGate_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Xnor);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}

void MainWindow::on_actionClear_triggered(){
    Engine* engine = currentEngine();
    GraphicsScene* scene = currentScene();
    if (engine) {
        engine->clearAll();
    }
    if(scene){
        scene->clear(); // clear()会删除场景中的所有图形项
        scene->update(); // 立即刷新视图
    }
}

void MainWindow::onComponentPlaced()
{
    // 检查当前是否有被按下的(checked)动作
    QAction* checked_action = m_addComponentActionGroup->checkedAction();
    if (checked_action) {
        // 取消它的选中状态，第二个参数 false 避免触发信号
        checked_action->setChecked(false);
    }

    // 恢复场景模式到空闲状态
    GraphicsScene* scene = currentScene();
    if(scene) {
        scene->setMode(GraphicsScene::Idle);
    }

    // 将状态栏恢复到“准备就绪”
    ui->statusbar->showMessage("准备就绪");
}

void MainWindow::on_actionSave_triggered()
{
    Engine* engine = currentEngine();
    if (!engine) {
        QMessageBox::warning(this, "无活动电路", "没有可以保存的活动电路。");
        return;
    }

    static QString lastUsedDir = "";
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "保存电路文件",
        lastUsedDir.isEmpty() ? "untitled.json" : lastUsedDir + "/untitled.json",
        "JSON 文件 (*.json)"
        );

    if (filePath.isEmpty()) {
        ui->statusbar->showMessage("保存操作已取消", 3000);
        return;
    }
    // 确保文件后缀是 .json
    if (!filePath.endsWith(".json", Qt::CaseInsensitive)) {
        filePath += ".json";
    }
    lastUsedDir = QFileInfo(filePath).path();

    // 从当前引擎获取电路数据
    QJsonObject circuitJson = engine->saveCircuitToJson();
    QJsonDocument saveDoc(circuitJson);

    // 写入文件
    QFile saveFile(filePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "保存错误", "无法打开文件进行写入！\n" + saveFile.errorString());
        return;
    }
    saveFile.write(saveDoc.toJson());
    saveFile.close();

    ui->statusbar->showMessage("文件已成功保存到: " + filePath, 5000);
}


void MainWindow::on_actionOpen_triggered()
{
    // 询问用户是否要保存当前活动标签页的修改（这是一个好的实践）
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "确认操作",
                                  "打开一个新文件将会替换当前标签页的电路，是否要先保存当前修改？",
                                  QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (reply == QMessageBox::Cancel) {
        ui->statusbar->showMessage("打开操作已取消", 3000);
        return;
    }
    if (reply == QMessageBox::Save) {
        on_actionSave_triggered();
    }

    // 打开文件对话框
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "打开电路文件",
        "", // 默认目录
        "JSON 文件 (*.json);;所有文件 (*.*)"
        );
    if (filePath.isEmpty()) {
        ui->statusbar->showMessage("打开操作已取消", 3000);
        return;
    }

    // 读取文件
    QFile loadFile(filePath);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "打开错误", "无法打开文件进行读取！");
        return;
    }
    QByteArray fileData = loadFile.readAll();
    loadFile.close();

    // 解析JSON
    QJsonDocument loadDoc = QJsonDocument::fromJson(fileData);
    if (loadDoc.isNull() || !loadDoc.isObject()) {
        QMessageBox::critical(this, "解析错误", "文件不是一个有效的JSON对象！");
        return;
    }
    QJsonObject circuitJson = loadDoc.object();

    // 获取当前引擎和场景，准备加载数据
    Engine* engine = currentEngine();
    GraphicsScene* scene = currentScene();
    if (!engine || !scene) {
        // 如果没有活动标签页（虽然不太可能发生），就新建一个
        onNewTab();
        engine = currentEngine();
        scene = currentScene();
    }

    // 调用引擎的加载功能
    if (engine->loadCircuitFromJson(circuitJson)) {
        // 如果后台引擎成功加载了数据...
        // 命令前台画布根据引擎的新状态重绘
        scene->rebuildSceneFromEngine();
        // 给出成功反馈
        ui->statusbar->showMessage("电路已成功从 " + filePath + " 加载", 5000);

    } else {
        // 返回false，说明加载失败
        QMessageBox::critical(this, "加载失败", "文件内容格式错误或数据不兼容，无法加载。");
    }
}

// 封装功能的占位符，将在下一步实现
void MainWindow::on_actionEncapsulate_triggered()
{
    QMessageBox::information(this, "待开发", "封装功能将在下一步实现！");
}
