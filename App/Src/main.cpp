﻿/**
 * @file main.cpp
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-15
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#include <QApplication>
#include <QWidget>

#include "globalSettings.h"
#include "mainwindow.h"
#include "pch.h"

extern bool g_isUsingOpenGL = false;

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
