#include "mainwindow.h"

#include <QApplication>
#include <QException>
#include <QSplashScreen>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow mainWindow;

    QPixmap pixmap(":/resources/SplashScreen.png");
    QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
    splash.show();

    if (false == mainWindow.Init(splash))
    {
        return EXIT_FAILURE;
    }

    splash.finish(&mainWindow);

    mainWindow.show();

    return app.exec();
}
