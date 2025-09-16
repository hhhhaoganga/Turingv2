#include "mainwindow.h"     // 主窗口声明
#include "engine.h"         // 使用 Engine 接口
#include "graphics.h"       // 使用 GraphicsScene/Item
#include "ui_mainwindow.h"  // Qt Designer 生成的UI类
#include <QActionGroup>       // 互斥动作组
#include <QMessageBox>        // 弹窗提示
#include <QFileDialog>        // 文件选择对话框
#include <QInputDialog>       // 简易输入框
#include <QJsonDocument>      // JSON 文档读写
#include <QJsonObject>        // JSON 对象
#include <QFile>              // 文件IO
#include <QFileInfo>          // 路径/扩展处理
#include <QTabWidget>         // 多标签容器
#include <QPainter>           // 设置 QGraphicsView 的渲染提示
#include <QGraphicsView>      // 视图容器
#include <QDir>               // 目录访问
#include <QToolBar>           // 工具栏
#include <QMenu>              // 右键菜单
/**
 * @file mainwindow.cpp
 * @brief 主窗口实现：多标签页管理、文件读写、自定义元件封装与加载。
 */
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
/** 构造主窗口：初始化UI、动作组、工具栏与默认标签页 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("逻辑链成");

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
    ui->toolBar_2->setContextMenuPolicy(Qt::CustomContextMenu);

    // 【新增】将右键点击信号连接到我们新创建的槽函数
    connect(ui->toolBar_2, &QToolBar::customContextMenuRequested,
            this, &MainWindow::onCustomComponentToolbarContextMenuRequested);
    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabClose);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onComponentPlaced);
    // 程序启动时，扫描元件库并填充到现有工具栏
    populateCustomComponentToolbar();
    // 4. 启动时自动创建一个空白标签页
    onNewTab();
}

// 目的: 销毁主窗口时，清理我们手动创建的对象，防止内存泄漏。
/** 析构：释放UI（标签页中Engine由关闭时释放） */
MainWindow::~MainWindow()
{
    delete ui;
    // 由于 Engine 和 Scene 的生命周期已与Tab页绑定，此处无需再手动清理
}

// === 辅助函数：获取当前活动标签页的 Engine 和 Scene ===

/** 返回当前标签页的场景指针 */
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

/** 返回当前标签页的引擎指针 */
Engine* MainWindow::currentEngine() {
    GraphicsScene* scene = currentScene();
    if (scene) {
        return scene->getEngine();
    }
    return nullptr;
}


// === 标签页管理槽函数 ===

/** 新建一个标签页：创建独立 Engine 与 Scene 并安装到视图 */
void MainWindow::onNewTab()
{
    // 1. 为新标签页创建一套独立的 Engine 和 Scene
    Engine* engine = new Engine();
    GraphicsScene* scene = new GraphicsScene(engine, this); // 将 engine 传入

    // 2. 将 Scene 安装到一个 QGraphicsView 中
    QGraphicsView* view = new QGraphicsView(scene);

    // =============================================================
    // == 【核心修改】在这里添加导航和滚动条的配置
    // =============================================================

    // a. 设置渲染提示，让图形和文字更平滑
    view->setRenderHint(QPainter::Antialiasing);

    // b. 设置滚动条策略为“按需显示”
    //    当场景内容超出视图范围时，滚动条会自动出现。
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // d. 确保支持鼠标滚轮缩放 (这是默认行为，但明确设置可以避免意外)
    //    用户可以按住 Ctrl 键并滚动鼠标滚轮来缩放视图。
    view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse); // 以鼠标为中心进行缩放
    view->setResizeAnchor(QGraphicsView::AnchorViewCenter);

    // 3. 把这个 view 添加为一个新的标签页
    QString tabName = QString("电路 %1").arg(ui->tabWidget->count() + 1);
    int index = ui->tabWidget->addTab(view, tabName);

    // 4. 自动切换到这个新创建的标签页
    ui->tabWidget->setCurrentIndex(index);

    // 5. 连接 componentAdded 信号，以便在放置元件后取消工具栏按钮的选中状态
    connect(scene, &GraphicsScene::componentAdded, this, &MainWindow::onComponentPlaced);
}

/** 关闭指定索引的标签页，并释放其 Engine */
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

/** 工具栏：添加输入元件 */
void MainWindow::on_actionAdd_Input_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Input);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
/** 工具栏：添加输出元件 */
void MainWindow::on_actionAdd_Output_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Output);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
/** 工具栏：添加与门 */
void MainWindow::on_actionAdd_AndGate_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::And);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
/** 工具栏：添加或门 */
void MainWindow::on_actionAdd_OrGate_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Or);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
/** 工具栏：添加非门 */
void MainWindow::on_actionAdd_NotGate_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Not);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
/** 工具栏：添加与非门 */
void MainWindow::on_actionAdd_NandGate_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Nand);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
/** 工具栏：添加或非门 */
void MainWindow::on_actionAdd_NorGate_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Nor);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
/** 工具栏：添加异或门 */
void MainWindow::on_actionAdd_XorGate_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Xor);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}
/** 工具栏：添加同或门 */
void MainWindow::on_actionAdd_XnorGate_triggered()
{
    GraphicsScene* scene = currentScene();
    if (scene) {
        scene->setComponentTypeToAdd(ComponentType::Xnor);
        scene->setMode(GraphicsScene::AddingComponent);
    }
}

/** 清空当前画布与引擎数据 */
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

/** 元件放置后：取消工具栏选中并恢复状态栏 */
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

/** 保存当前电路为JSON文件 */
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




/** 打开JSON文件到新标签页，并重建场景 */
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
        // 【修改】使用 baseName() 替代 fileName() 来移除后缀
        QString fileName = QFileInfo(filePath).baseName();
        ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), fileName);

        // 给出成功反馈
        ui->statusbar->showMessage("电路已成功从 " + filePath + " 加载到新画布", 5000);

    } else {
        // 如果加载失败，给出提示并关闭刚刚创建的空标签页
        QMessageBox::critical(this, "加载失败", "文件内容格式错误或数据不兼容，无法加载。");
        onTabClose(ui->tabWidget->currentIndex());
    }
}


// in mainwindow.cpp

/** 将当前电路保存为一个自定义封装元件 */
void MainWindow::on_actionEncapsulate_triggered()
{
    Engine* engine = currentEngine();
    if (!engine || engine->getAllComponents().isEmpty()) {
        QMessageBox::warning(this, "封装错误", "当前画布是空的，无法封装。");
        return;
    }

    // 1. 检查电路是否包含至少一个输入和输出
    bool hasInput = false, hasOutput = false;
    for (Component* comp : engine->getAllComponents().values()) {
        if (comp->type() == ComponentType::Input) hasInput = true;
        if (comp->type() == ComponentType::Output) hasOutput = true;
    }
    if (!hasInput || !hasOutput) {
        QMessageBox::warning(this, "封装错误", "电路必须至少包含一个输入(Input)和一个输出(Output)元件，才能定义封装后的引脚。");
        return;
    }

    // 2. 弹出窗口让用户命名，并使用当前标签页的标题作为默认名
    QString defaultName = ui->tabWidget->tabText(ui->tabWidget->currentIndex());
    bool ok;
    QString componentName = QInputDialog::getText(this, "另存为新元件",
                                                  "请输入新元件的名称:", QLineEdit::Normal, defaultName, &ok);
    if (!ok || componentName.isEmpty()) {
        ui->statusbar->showMessage("操作已取消", 3000);
        return;
    }

    // 3. 创建元件库目录 (如果不存在)
    QString libPath = QCoreApplication::applicationDirPath() + "/components";
    QDir dir(libPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 4. 将当前电路保存为 JSON 文件
    QString filePath = libPath + "/" + componentName + ".json";
    QFile saveFile(filePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "保存错误", "无法创建元件文件！");
        return;
    }

    QJsonObject circuitJson = engine->saveCircuitToJson();
    saveFile.write(QJsonDocument(circuitJson).toJson());
    saveFile.close();

    QMessageBox::information(this, "成功", QString("新元件 '%1' 已成功保存！").arg(componentName));

    // 5. 【关键】保存成功后，立即刷新工具栏，新元件按钮就会出现！
    populateCustomComponentToolbar();
}

// in mainwindow.cpp

/** 扫描应用目录下 components/，重建自定义元件的工具栏按钮 */
void MainWindow::populateCustomComponentToolbar()
{
    // --- 1. 清理旧的自定义按钮 ---
    QList<QAction*> actionsToRemove;
    for (QAction *action : ui->toolBar_2->actions()) {
        // 通过我们设置的自定义属性来识别
        if (action->property("isCustom").toBool()) {
            actionsToRemove.append(action);
        }
    }

    for (QAction *action : actionsToRemove) {
        ui->toolBar_2->removeAction(action);
        m_addComponentActionGroup->removeAction(action);
        delete action;
    }

    // --- 2. 扫描文件并重建按钮 ---
    QString libPath = QCoreApplication::applicationDirPath() + "/components";
    QDir dir(libPath);
    if (!dir.exists()) return; // 如果文件夹不存在，就没什么可做的了

    QStringList filters;
    filters << "*.json";
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);

    // 如果存在自定义元件，就在内置元件和自定义元件之间加一条分割线
    // (仅在第一次添加，或者清理后重新添加时)
    if (!fileList.isEmpty() && actionsToRemove.isEmpty()) {
        ui->toolBar_2->addSeparator();
    }

    for (const QFileInfo &fileInfo : fileList) {
        QAction *action = new QAction(this);
        action->setText(fileInfo.baseName()); // 文件名作为按钮文字
        action->setCheckable(true);
        action->setData(fileInfo.absoluteFilePath()); // 存储完整路径

        // 【核心标记】给这个 Action 打上 "isCustom" 标签，值为 true
        action->setProperty("isCustom", true);

        ui->toolBar_2->addAction(action);
        m_addComponentActionGroup->addAction(action);

        connect(action, &QAction::triggered, this, &MainWindow::onCustomComponentActionTriggered);
    }
}

// in mainwindow.cpp

/** 点击自定义元件按钮：读取JSON并设置场景为添加封装元件模式 */
void MainWindow::onCustomComponentActionTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) return;

    GraphicsScene* scene = currentScene();
    if (!scene) return;

    QString filePath = action->data().toString();

    QFile loadFile(filePath);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", "无法读取元件文件: " + filePath);
        action->setChecked(false); // 弹起按钮
        return;
    }
    QJsonDocument loadDoc = QJsonDocument::fromJson(loadFile.readAll());
    if (loadDoc.isNull() || !loadDoc.isObject()) {
        QMessageBox::warning(this, "错误", "元件文件格式无效: " + filePath);
        action->setChecked(false); // 弹起按钮
        return;
    }

    // 设置场景进入“添加封装元件”模式，并把 JSON 数据传递过去
    scene->setNameForNextComponent(action->text());
    scene->setMode(GraphicsScene::AddingComponent);
    scene->setComponentTypeToAdd(ComponentType::Encapsulated);
    scene->setJsonForNextComponent(loadDoc.object());
}
/** 在自定义元件工具栏上右键：提供删除该库元件的操作 */
void MainWindow::onCustomComponentToolbarContextMenuRequested(const QPoint &pos)
{
    // 1. 获取在指定位置的 Action (按钮)
    QAction* action = ui->toolBar_2->actionAt(pos);

    // 2. 检查这个 Action 是否有效，以及它是不是一个我们标记过的自定义元件
    if (action && action->property("isCustom").toBool()) {
        // 创建一个上下文菜单
        QMenu contextMenu(this);
        QAction *deleteAction = contextMenu.addAction("从库中删除");

        // 弹出菜单，并等待用户操作
        QAction *selectedAction = contextMenu.exec(ui->toolBar_2->mapToGlobal(pos));

        // 3. 如果用户点击了“删除”
        if (selectedAction == deleteAction) {
            // 弹出确认对话框
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "确认删除",
                                          QString("你确定要永久删除元件 '%1' 吗？\n这个操作无法撤销。").arg(action->text()),
                                          QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                // 4. 从 Action 中获取文件路径并执行删除
                QString filePath = action->data().toString();
                QFile file(filePath);
                if (file.remove()) {
                    ui->statusbar->showMessage(QString("元件 '%1' 已成功删除。").arg(action->text()), 3000);
                    // 5. 【关键】调用刷新函数，UI 上的按钮就会消失
                    populateCustomComponentToolbar();
                } else {
                    QMessageBox::critical(this, "删除失败", "无法从硬盘删除文件，请检查文件权限。");
                }
            }
        }
    }
}
