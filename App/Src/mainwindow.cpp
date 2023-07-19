#include "mainwindow.h"

#include <QFontDatabase>
#include <QHBoxLayout>
#include <ranges>

#include "fftdatasource.h"
#include "pch.h"
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

    currentSelectedPlot = ui->mainPlotWidget;
}

MainWindow::~MainWindow() {
    delete ui;

    for (auto thread : sourceToThreadMap | std::views::values) {
        if (thread == nullptr) {
            continue;
        }

        printCurrentTime() << "Thread" << thread->currentThreadId()
                           << "is running:" << thread->isRunning();

        if (thread->isRunning()) {
            thread->exit();
            thread->wait();

            printCurrentTime()
                << "Thread" << thread->currentThreadId() << "is exited.";
        }
    }
}

void MainWindow::closeEvent(QCloseEvent* e) {
    printCurrentTime() << "MainWindow::closeEvent()";

    emit windowExited();

    QWidget::closeEvent(e);
}

void MainWindow::bindPushButtons() {
    // serial config btn
    connect(ui->bSerialSettings, &QPushButton::clicked, this, [this]() {
        printCurrentTime() << "Serial settings button clicked";
        SerialSettingsDiag* serialSettingsDiag = new SerialSettingsDiag{};

        connect(serialSettingsDiag, &SerialSettingsDiag::settingsReceived, this,
                [this](std::optional<SerialSettings> settings) {
                    if (!settings.has_value()) {
                        printCurrentTime() << "Creative Serial Canceled";
                        return;
                    }

                    printCurrentTime() << "Serial settings received";

                    createSerialDataSource(settings.value(), PopUpNewWindow);
                });

        serialSettingsDiag->exec();
    });
}

void MainWindow::onSourceError(QString errorMsg) {
    printCurrentTime() << "Serial error:"
                       << errorMsg.toLocal8Bit().toStdString();
    // TODO
}

void MainWindow::onSourceControlWordReceived(
    DataSource::DataControlWords controlWord, QByteArray DCWData) {
    printCurrentTime() << "Control word received " << controlWord
                       << "with external DCWData" << DCWData.toStdString();
}

QCPGraph* MainWindow::createNewPlot(DataSource* source, QString title,
                                    QPen color, NewDataStrategy strategy) {
    CustomPlot* rPlot = nullptr;
    QCPGraph* rSeries = nullptr;

    switch (strategy) {
        case ReusePlot: {
            if (currentSelectedPlot == nullptr) {
                throw std::runtime_error{"currentSelectedPlot is nullptr"};
            }

            std::tie(rPlot, rSeries) =
                currentSelectedPlot->addPlot(source->getId());

        } break;
        case PopUpNewWindow: {
            auto newPlotWnd = new ChartWidget{};

            newPlotWnd->setWindowTitle(title);
            newPlotWnd->setWindowIcon(windowIcon());
            newPlotWnd->show();

            popUpPlots.push_back(newPlotWnd);

            std::tie(rPlot, rSeries) = newPlotWnd->addPlot(source->getId());

            currentSelectedPlot = newPlotWnd;
        } break;
        case InsertAtMainWindow: {
            std::tie(rPlot, rSeries) =
                ui->mainPlotWidget->addPlot(source->getId());

            currentSelectedPlot = ui->mainPlotWidget;
        } break;

        default:
            break;
    }

    rPlot->show();

    rSeries->setScatterStyle(QCPScatterStyle{QCPScatterStyle::ssCircle, 5});
    rSeries->setPen(color);

    return rSeries;
}

void MainWindow::bindDataSource(DataSource* source, QCPGraph* series) {
    connect(source, &DataSource::dataReceived, this,
            [this, series](const QVector<double> x, const QVector<double> y) {
                series->addData(x, y, true);

                series->rescaleAxes();

                series->parentPlot()->replot();
            });
}

void MainWindow::createSerialDataSource(SerialSettings settings,
                                        NewDataStrategy strategy) {
    printCurrentTime() << "Serial settings received";

    auto serialWorker = new SerialWorker{};
    serialWorker->setSerialSettings(settings);

    connect(serialWorker, &SerialWorker::error, this,
            &MainWindow::onSourceError);
    connect(serialWorker, &SerialWorker::controlWordReceived, this,
            &MainWindow::onSourceControlWordReceived);
    connect(serialWorker, &DataSource::finished, this, [this, serialWorker]() {
        auto& [workerRef, serialThreadRef] =
            sourceToThreadMap[serialWorker->getId()];

        workerRef = nullptr;

        if (serialThreadRef != nullptr && serialThreadRef->isRunning()) {
            serialThreadRef->quit();
            serialThreadRef->wait();
            serialThreadRef->deleteLater();
            serialThreadRef = nullptr;
        }

        printCurrentTime() << "Serial thread finished";
    });

    connect(this, &MainWindow::windowExited, serialWorker,
            &DataSource::requestStopDataSource);

    auto newPlot = createNewPlot(serialWorker, settings.portName,
                                 QPen{QColor{0xfe, 0x5a, 0x5b}}, strategy);
    bindDataSource(serialWorker, newPlot);

    auto th = new QThread{this};
    connect(th, &QThread::started, serialWorker, &DataSource::run);
    connect(th, &QThread::finished, serialWorker, &SerialWorker::deleteLater);
    serialWorker->moveToThread(th);
    sourceToThreadMap.insert(serialWorker->getId(), {serialWorker, th});
    th->start();

    // connect(fftSource, &DataSource::dataReceived, this,
    //         [this, newPlot](const QVector<double> x, const QVector<double> y)
    //         {
    //             newPlot->setData(x, y, true);

    //             newPlot->rescaleAxes();

    //             newPlot->parentPlot()->replot();
    //         });

    // th->start();

    // // TODO FFT plot

    // NewDataStrategy old = newDataStrategy;
    // newDataStrategy = NewDataStrategy::PopUpNewWindow;
    // newPlot = createNewSeries("FFT", {QColor{0xfe, 0x5a, 0x5b}});
    // auto fftSource = new FFTDataSource{serialWorker};
    // dataSources.insert(fftSource, newPlot);
    // connect(fftSource, &DataSource::dataReceived, this,
    //         [this, newPlot](const QVector<double> x, const QVector<double> y)
    //         {
    //             newPlot->setData(x, y, true);

    //             newPlot->rescaleAxes();

    //             newPlot->parentPlot()->replot();
    //         });
    // newDataStrategy = old;

    // if (serialThread == nullptr)
    //     serialThread = new QThread{this};

    // connect(serialThread, &QThread::started, serialWorker, &DataSource::run);
    // connect(serialThread, &QThread::finished, serialWorker,
    //         &SerialWorker::deleteLater);
    // serialWorker->moveToThread(serialThread);
    // serialThread->start();

    // // TODO FFT plot
    // auto th = new QThread{this};

    // connect(th, &QThread::started, fftSource, &DataSource::run);
    // connect(th, &QThread::finished, fftSource, &FFTDataSource ::deleteLater);
    // fftSource->moveToThread(th);
    // th->start();
}
