#include <QApplication>
#include <QWidget>

#include "mainwindow.h"
#include "pch.h"

DESTROYER(EXIT_PRINT) {}

int main(int argc, char* argv[]) {
    QApplication app{argc, argv};

    printCurrentTime() << "Initializing" << PROGRAM_NAME << "...";
    printCurrentTime() << "Starting" << PROGRAM_NAME;
    printCurrentTime() << "Current version: " << VERSION;

    MainWindow w{nullptr};

    w.show();
    auto rVal = app.exec();

    printCurrentTime() << "Exiting" << PROGRAM_NAME << "...";
    return rVal;
}
