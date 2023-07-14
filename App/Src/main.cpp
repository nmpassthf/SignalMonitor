#include <iostream>
#include <QWidget>
#include <QApplication>

#include "pch.h"


int main(int argc, char *argv[]) {
    std::cout << "Starting " << PROGRAM_NAME << std::endl;
    std::cout << "Current version: " << VERSION << std::endl;

    QApplication app{argc, argv};
    QWidget w{nullptr};

    w.show();

    return app.exec();
}
