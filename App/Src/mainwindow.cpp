#include "mainwindow.h"

#include <QFontDatabase>
#include <QHBoxLayout>

#include "fftdatasource.h"
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

    ui->mainPlotWidget->setOpenGl(true);
    ui->mainPlotWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->mainPlotWidget->setAntialiasedElement(QCP::aeAll);
    ui->mainPlotWidget->axisRect()->setRangeZoom(Qt::Horizontal);
    connect(ui->mainPlotWidget, &QCustomPlot::beforeReplot, this, [this]() {
        // zoom y to Fix screen.
        for (int i = 0; i != ui->mainPlotWidget->graphCount(); ++i) {
            ui->mainPlotWidget->graph(i)->rescaleValueAxis(false, true);
        }
    });
}

MainWindow::~MainWindow() {
    delete ui;

    if (serialThread != nullptr) {
        serialThread->quit();
        serialThread->wait();
    }
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
                connect(serialWorker, &DataSource::finished, this,
                        [this, serialWorker]() {
                            if (serialThread->isRunning()) {
                                serialThread->quit();
                                serialThread->wait();
                                serialThread->deleteLater();
                                serialThread = nullptr;
                            }

                            isDataSourceRunning[serialWorker] = false;

                            printCurrentTime()
                                << "Serial thread finished" << std::endl;
                        });

                connect(this, &MainWindow::serialCloseRequest, serialWorker,
                        &DataSource::requestStopDataSource);

                auto newPlot =
                    createNewSeries("Serial", {QColor{0x26, 0xa1, 0xe0}});
                bindDataSource(serialWorker, newPlot);

                // TODO FFT plot

                NewDataStrategy old = newDataStrategy;
                newDataStrategy = NewDataStrategy::PopUpNewWindow;
                newPlot = createNewSeries("FFT", {QColor{0xfe, 0x5a, 0x5b}});
                auto fftSource = new FFTDataSource{serialWorker};
                dataSources.insert(fftSource, newPlot);
                connect(fftSource, &DataSource::dataReceived, this,
                        [this, newPlot](const QVector<double> x,
                                        const QVector<double> y) {
                            newPlot->setData(x, y, true);

                            newPlot->rescaleAxes();

                            newPlot->parentPlot()->replot();
                        });
                newDataStrategy = old;

                if (serialThread == nullptr)
                    serialThread = new QThread{this};

                connect(serialThread, &QThread::started, serialWorker,
                        &DataSource::run);
                connect(serialThread, &QThread::finished, serialWorker,
                        &SerialWorker::deleteLater);
                serialWorker->moveToThread(serialThread);
                serialThread->start();

                // TODO FFT plot
                auto th = new QThread{this};

                connect(th, &QThread::started, fftSource, &DataSource::run);
                connect(th, &QThread::finished, fftSource,
                        &FFTDataSource ::deleteLater);
                fftSource->moveToThread(th);
                th->start();

                isDataSourceRunning[serialWorker] = true;
            });

        serialSettingsDiag->exec();
    });
}

void MainWindow::onSourceError(QString errorMsg) {
    printCurrentTime() << "Serial error:"
                       << errorMsg.toLocal8Bit().toStdString() << std::endl;
    // TODO
}

void MainWindow::onSourceControlWordReceived(QByteArray controlWord) {
    printCurrentTime() << "Serial control word received"
                       << controlWord.toStdString() << std::endl;
}

QCPGraph* MainWindow::createNewSeries(QString title, QPen color) {
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

            newPlot->setOpenGl(true);
            newPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
            newPlot->setAntialiasedElement(QCP::aeAll);
            newPlot->axisRect()->setRangeZoom(Qt::Horizontal);
            connect(newPlot, &QCustomPlot::beforeReplot, this, [newPlot]() {
                // zoom y to Fix screen.
                for (int i = 0; i != newPlot->graphCount(); ++i) {
                    newPlot->graph(i)->rescaleValueAxis(false, true);
                }
            });

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

    rSeries->setScatterStyle(QCPScatterStyle{QCPScatterStyle::ssCircle, 5});
    rSeries->setPen(color);

    return rSeries;
}

void MainWindow::bindDataSource(DataSource* source, QCPGraph* series) {
    dataSources.insert(source, series);
    connect(source, &DataSource::dataReceived, this,
            [this, series](const QVector<double> x, const QVector<double> y) {
                series->addData(x, y, true);

                series->rescaleAxes();

                series->parentPlot()->replot();
            });
}
