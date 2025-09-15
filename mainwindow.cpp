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
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onComponentPlaced);

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


// ... (其他函数保持不变) ...

void MainWindow::on_actionOpen_triggered()
{
    // 1. 打开文件对话框，让用户选择文件
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "打开电路文件",
        "", // 默认目录
        "JSON 文件 (*.json);;所有文件 (*.*)"
        );

    // 如果用户取消了选择，直接返回
    if (filePath.isEmpty()) {
        ui->statusbar->showMessage("打开操作已取消", 3000);
        return;
    }

    // 2. 读取文件内容
    QFile loadFile(filePath);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "打开错误", "无法打开文件进行读取！");
        return;
    }
    QByteArray fileData = loadFile.readAll();
    loadFile.close();

    // 3. 解析JSON数据
    QJsonDocument loadDoc = QJsonDocument::fromJson(fileData);
    if (loadDoc.isNull() || !loadDoc.isObject()) {
        QMessageBox::critical(this, "解析错误", "文件不是一个有效的JSON对象！");
        return;
    }
    QJsonObject circuitJson = loadDoc.object();

    // 4. 【核心修改】创建一个新的标签页用于承载打开的文件
    onNewTab();

    // 5. 获取这个刚刚创建的新标签页的 Engine 和 Scene
    Engine* engine = currentEngine();
    GraphicsScene* scene = currentScene();

    // 健壮性检查，虽然 onNewTab 之后这里应该总是有值的
    if (!engine || !scene) {
        QMessageBox::critical(this, "内部错误", "无法创建新的画布来加载文件。");
        // 关闭刚刚创建的空标签页
        onTabClose(ui->tabWidget->currentIndex());
        return;
    }

    // 6. 调用引擎的加载功能，将JSON数据加载到新引擎中
    if (engine->loadCircuitFromJson(circuitJson)) {
        // 如果后台引擎成功加载...
        // ...命令前台画布根据引擎的新状态重绘
        scene->rebuildSceneFromEngine();

        // 7. 【体验优化】将新标签页的标题设置为文件名
        QString fileName = QFileInfo(filePath).fileName();
        ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), fileName);

        // 给出成功反馈
        ui->statusbar->showMessage("电路已成功从 " + filePath + " 加载到新画布", 5000);

    } else {
        // 如果加载失败，给出提示并关闭刚刚创建的空标签页
        QMessageBox::critical(this, "加载失败", "文件内容格式错误或数据不兼容，无法加载。");
        onTabClose(ui->tabWidget->currentIndex());
    }
}

// ... (其他函数保持不变) ...
// 封装功能的占位符，将在下一步实现
void MainWindow::on_actionEncapsulate_triggered()
{
    QMessageBox::information(this, "待开发", "封装功能将在下一步实现！");
}
