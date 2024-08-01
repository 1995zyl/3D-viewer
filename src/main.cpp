#include "main_window.h"
#include <QApplication>
#include <QTranslator>
#ifdef WIN32
#include <windows.h>
#endif

namespace
{
    static void ensureConsole()
    {
#ifdef WIN32
        // START_INFO_CONSOLE定义在cmake中，控制是否开启debug控制台，用于观察是否有报错日志
#ifndef START_INFO_CONSOLE
        HWND hwnd = GetForegroundWindow();
        if (hwnd)
            ShowWindow(hwnd, SW_HIDE);
#endif
#endif
    }
}

int main(int argc, char *argv[])
{
    ensureConsole();

    QApplication a(argc, argv);
    QTranslator qtTranslator;
    if (qtTranslator.load(":/ZH_CN.qm"))
        a.installTranslator(&qtTranslator);

    MainWindow w;
    w.show();
    return a.exec();
}
