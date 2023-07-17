#include "mainwindow.h"

#include <QFontDatabase>
#include <QHBoxLayout>

#include "pch.h"
#include "serial.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QWidget{parent}, ui{new Ui::MainWindow} {
    ui->setupUi(this);

    // Set App title & icon
    setWindowTitle(PROGRAM_NAME);
    setWindowIcon(QIcon{":/amamiya.ico"});

    // Load title fonts
    auto currentFont =
        QFontDatabase::addApplicationFont(":/Fonts/Shintyan.ttf");
    QFontDatabase::applicationFontFamilies(currentFont);

    ui->title->setFont(QFont{"Shintyan", 40, QFont::Bold});

    // connect push buttons
    bindPushButtons();

    serialThread = new QThread{this};

    ui->mainPlotWidget->setOpenGl(true);
}

MainWindow::~MainWindow() {
    delete ui;
    serialThread->quit();
    serialThread->wait();
}

void MainWindow::closeEvent(QCloseEvent* e) {
    printCurrentTime() << "Close event" << std::endl;

    emit serialCloseRequest();

    QWidget::closeEvent(e);
}

void MainWindow::bindPushButtons() {
    // serial config btn
    connect(ui->bSerialSettings, &QPushButton::clicked, this, [this]() {
        printCurrentTime() << "Serial settings button clicked" << std::endl;
        SerialSettingsDiag* serialSettingsDiag = new SerialSettingsDiag{};

        connect(
            serialSettingsDiag, &SerialSettingsDiag::settingsReceived, this,
            [this](std::optional<SerialSettings> settings) {
                if (!settings.has_value()) {
                    printCurrentTime()
                        << "Serial settings not received" << std::endl;
                    return;
                }

                printCurrentTime() << "Serial settings received" << std::endl;

                auto serialWorker = new SerialWorker{};
                serialWorker->setSerialSettings(settings.value());

                connect(serialWorker, &SerialWorker::error, this,
                        &MainWindow::onSourceError);
                connect(serialWorker, &SerialWorker::controlWordReceived, this,
                        &MainWindow::onSourceControlWordReceived);
                connect(serialWorker, &DataSource::finished, this, [this]() {
                    serialThread->quit();
                    serialThread->wait();
                    serialThread->deleteLater();
                    serialThread = nullptr;

                    printCurrentTime() << "Serial thread finished" << std::endl;
                });

                connect(this, &MainWindow::serialCloseRequest, serialWorker,
                        &DataSource::requestStopDataSource);

                auto newPlot = createNewSeries("Serial");
                bindDataSource(serialWorker, newPlot);

                connect(serialThread, &QThread::started, serialWorker,
                        &SerialWorker::run);
                serialWorker->moveToThread(serialThread);
                serialThread->start();
            });

        serialSettingsDiag->exec();
    });
}

void MainWindow::onSourceError(QString errorMsg) {
    printCurrentTime() << "Serial error:" << errorMsg.toStdString()
                       << std::endl;
    // TODO
}

void MainWindow::onSourceControlWordReceived(QByteArray controlWord) {
    printCurrentTime() << "Serial control word received"
                       << controlWord.toStdString() << std::endl;
}

QCPGraph* MainWindow::createNewSeries(QString title) {
    QCPGraph* rSeries = nullptr;

    switch (newDataStrategy) {
        case NewDataStrategy::ReusePlot: {
            if (currentSelectedPlot == nullptr) {
                throw std::runtime_error{"currentSelectedPlot is nullptr"};
            }

            rSeries = currentSelectedPlot->addGraph();
        } break;
        case NewDataStrategy::PopUpNewWindow: {
            auto newPlot = new QCustomPlot{};
            newPlot->setWindowTitle(title);
            newPlot->show();

            popUpPlots.push_back(newPlot);

            rSeries = newPlot->addGraph();

            currentSelectedPlot = newPlot;
        } break;
        case NewDataStrategy::InsertAtMainWindow: {
            rSeries = ui->mainPlotWidget->addGraph();
            currentSelectedPlot = ui->mainPlotWidget;
        } break;

        default:
            break;
    }

    return rSeries;
}

void MainWindow::bindDataSource(DataSource* source, QCPGraph* series) {
    dataSources.insert(source, series);
    connect(source, &DataSource::dataReceived, this,
            [this, series](const QVector<double> x, const QVector<double> y) {
                series->addData(x, y, true);

                series->rescaleAxes();

                currentSelectedPlot->replot();
            });
}
