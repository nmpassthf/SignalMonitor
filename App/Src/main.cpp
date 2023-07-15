#include <QApplication>
#include <QWidget>

#include "mainwindow.h"
#include "pch.h"

INITIALIZER(INIT_PRINT) {
    printCurrentTime() << "Initializing " << PROGRAM_NAME << std::endl;
    printCurrentTime() << "Starting " << PROGRAM_NAME << std::endl;
    printCurrentTime() << "Current version: " << VERSION << std::endl;
}

DESTROYER(EXIT_PRINT) {
    printCurrentTime() << "Exiting " << PROGRAM_NAME << std::endl;
}

int main(int argc, char* argv[]) {
    QApplication app{argc, argv};
    MainWindow w{nullptr};

    w.show();

    return app.exec();
}
