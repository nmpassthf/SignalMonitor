#include "mainwindow.h"

#include <QFontDatabase>
#include <QVBoxLayout>

#include "pch.h"
#include "serial.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QWidget{parent},
      ui{new Ui::MainWindow},
      threadPool{new QThreadPool{this}} {
    ui->setupUi(this);

    // Set App title & icon
    setWindowTitle(PROGRAM_NAME);
    setWindowIcon(QIcon{":/amamiya.ico"});

    // Load title fonts
    auto currentFont =
        QFontDatabase::addApplicationFont(":/Fonts/Shintyan.ttf");
    QFontDatabase::applicationFontFamilies(currentFont);

    ui->title->setFont(QFont{"Shintyan", 40, QFont::Bold});

    // Load chart widget
    // TODO
    ui->chartContainer->setLayout(new QVBoxLayout{});

    bindPushButtons();

    // init thread pool
    threadPool->setMaxThreadCount(8);
}

MainWindow::~MainWindow() {
    threadPool->waitForDone();

    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* e) {
    printCurrentTime() << "Close event" << std::endl;

    emit serialCloseRequest();

    QWidget::closeEvent(e);
}

void MainWindow::bindPushButtons() {
    // 串口设置按钮
    connect(ui->bSerialSettings, &QPushButton::clicked, this, [this]() {
        printCurrentTime() << "Serial settings button clicked" << std::endl;
        SerialSettingsDiag* serialSettingsDiag = new SerialSettingsDiag{};

        connect(serialSettingsDiag, &SerialSettingsDiag::settingsReceived, this,
                [this](std::optional<SerialSettings> settings) {
                    if (!settings.has_value()) {
                        printCurrentTime()
                            << "Serial settings not received" << std::endl;
                        return;
                    }

                    printCurrentTime()
                        << "Serial settings received" << std::endl;

                    auto serialWorker = new SerialWorker{nullptr};
                    serialWorker->setSerialSettings(settings.value());

                    connect(serialWorker, &SerialWorker::error, this,
                            &MainWindow::onSerialError);
                    connect(serialWorker, &SerialWorker::dataReceived, this,
                            &MainWindow::onSerialDataReceived);
                    connect(serialWorker, &SerialWorker::controlWordReceived,
                            this, &MainWindow::onSerialControlWordReceived);

                    connect(this, &MainWindow::serialCloseRequest, serialWorker,
                            &SerialWorker::closeSerial);

                    threadPool->start(serialWorker);
                });

        serialSettingsDiag->exec();
    });
}

void MainWindow::onSerialError(QString) {
    printCurrentTime() << "Serial error" << std::endl;
    // TODO
}
void MainWindow::onSerialDataReceived(QByteArray) {
    printCurrentTime() << "Serial data received" << std::endl;
    // TODO
}
void MainWindow::onSerialControlWordReceived(QByteArray) {
    printCurrentTime() << "Serial control word received" << std::endl;
    // TODO
}
