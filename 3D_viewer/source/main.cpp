#include "main_window.h"
#include <QApplication>
#include <QTranslator>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
// START_INFO_CONSOLE定义在cmake中，控制是否开启debug控制台，用于观察是否有报错日志
#ifndef START_INFO_CONSOLE
    HWND hwnd = GetForegroundWindow();
    if (hwnd)
        ShowWindow(hwnd, SW_HIDE);
#endif
#endif

    QApplication a(argc, argv);
    QTranslator qtTranslator;
    if (qtTranslator.load(":translator/ZH_CN.qm"))
        a.installTranslator(&qtTranslator);

    MainWindow w;
    w.show();
    return a.exec();
}
