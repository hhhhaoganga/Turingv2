#include "mainwindow.h"  // 主窗口入口

#include <QApplication>   // Qt 应用对象

/**
 * @brief 程序入口：创建应用与主窗口。
 */

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
