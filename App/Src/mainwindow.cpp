/**
 * @file mainwindow.cpp
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-15
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#include "mainwindow.h"

#include <QFontDatabase>
#include <QHBoxLayout>
#include <QScreen>
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

    // get current def screen size
    auto currentScreen = QGuiApplication::primaryScreen();

    auto currentScreenSize = currentScreen->availableSize();

    setGeometry(currentScreenSize.width() / 2 - aimWidth / 2,
                currentScreenSize.height() / 2 - aimHeight / 2, aimWidth,
                aimHeight);

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

        printCurrentTime() << "Thread" << thread
                           << "is running:" << thread->isRunning();

        if (thread->isRunning()) {
            thread->exit();
            thread->wait();

            printCurrentTime() << "Thread" << thread << "is exited.";
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

                    createSerialDataSource(settings.value(),
                                           InsertAtMainWindow);
                });

        serialSettingsDiag->exec();
    });

    // bClearPlots btn
    connect(ui->bClearPlots, &QPushButton::clicked, this, [this]() {
        printCurrentTime() << "Clear plots button clicked";

        ui->mainPlotWidget->clearPlots();
        for (auto plot : popUpPlots) {
            plot->clearPlots();
        }
    });
}

void MainWindow::onSourceError(QString errorMsg) {
    printCurrentTime() << "Serial error:" << errorMsg;
    // TODO
}

void MainWindow::onSourceControlWordReceived(
    qsizetype index, DataSource::DataControlWords controlWord,
    QByteArray DCWData) {
    printCurrentTime() << "Control word received in index:" << index
                       << "\n\twith Ctrl word: "
                       << DataSource::dataControlWordsToString(controlWord)
                       << "\n\twith external DCWData: "
                       << DCWData.toStdString();
}

QCPGraph* MainWindow::createNewPlot(DataSource* source, qsizetype index,
                                    QString title, QPen color,
                                    NewDataStrategy strategy,
                                    ChartWidget::PlotPos_t pos) {
    CustomPlot* rPlot = nullptr;
    QCPGraph* rSeries = nullptr;

    switch (strategy) {
        case ReusePlot: {
            if (currentSelectedPlot == nullptr) {
                throw std::runtime_error{"currentSelectedPlot is nullptr"};
            }

            std::tie(rPlot, rSeries) =
                currentSelectedPlot->addPlot(source, source->getId(index), pos);

        } break;
        case PopUpNewWindow: {
            auto newPlotWnd = new ChartWidget{};

            newPlotWnd->setWindowTitle(title);
            newPlotWnd->setWindowIcon(windowIcon());
            newPlotWnd->show();

            connect(this, &MainWindow::windowExited, newPlotWnd,
                    &ChartWidget::deleteLater);

            popUpPlots.push_back(newPlotWnd);

            std::tie(rPlot, rSeries) =
                newPlotWnd->addPlot(source, source->getId(index), pos);

            currentSelectedPlot = newPlotWnd;
        } break;
        case InsertAtMainWindow: {
            std::tie(rPlot, rSeries) =
                ui->mainPlotWidget->addPlot(source, source->getId(index), pos);

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
        for (auto ids : serialWorker->getIds()) {
            if (!sourceToThreadMap.contains(ids)) {
                continue;
            }

            auto& [workerRef, serialThreadRef] = sourceToThreadMap[ids];
            workerRef = nullptr;

            if (serialThreadRef != nullptr && serialThreadRef->isRunning()) {
                serialThreadRef->quit();
                serialThreadRef->wait();
                serialThreadRef->deleteLater();
                serialThreadRef = nullptr;
            }

            printCurrentTime() << "Serial thread finished";
        }
    });

    connect(this, &MainWindow::windowExited, serialWorker,
            &DataSource::requestStopDataSource);

    connect(ui->bClearPlots, &QPushButton::clicked, serialWorker,
            &SerialWorker::clearAllData);

    // TODO Pen color customable
    auto newPlot = createNewPlot(serialWorker, 0, settings.portName,
                                 QPen{QColor{0x57, 0xbe, 0x8a}}, strategy);
    auto serialCustomPlot = qobject_cast<CustomPlot*>(newPlot->parentPlot());
    if (settings.isTimeDomainData) {
        serialCustomPlot->yAxis->setLabel("Voltage (V)");
        serialCustomPlot->xAxis->setLabel("Time (us)");
    } else {
        serialCustomPlot->yAxis->setLabel("Amplitude");
        serialCustomPlot->xAxis->setLabel("Frequency (Hz)");
    }

    // 绑定数据源新增子图事件
    connect(serialWorker, &DataSource::newDataChannelCreated, this,
            [this, serialWorker, serialCustomPlot](qsizetype index,
                                                   DataSource::DSID id) {
                printCurrentTime()
                    << "New data channel created in index:" << index
                    << "\n\twith id:" << id;

                auto newPlot = createNewPlot(
                    serialWorker, index, QString{"Channel %1"}.arg(index),
                    QPen{QColor{0x57, 0xbe, 0x8a}}, ReusePlot, {-1, -1});
            });

    auto th = new QThread{this};
    connect(th, &QThread::started, serialWorker, &DataSource::run);
    connect(th, &QThread::finished, serialWorker, &SerialWorker::deleteLater);
    serialWorker->moveToThread(th);
    sourceToThreadMap.insert(serialWorker->getId(0), {serialWorker, th});
    th->start();

    // 自动添加FFT图像在其下方

    if (!settings.isTimeDomainData) {
        return;
    }

    auto serialWidget =
        qobject_cast<ChartWidget*>(serialCustomPlot->parentWidget());
    auto serialWidgetPos = serialWidget->getPlotPos(serialCustomPlot);

    // 幅度谱
    auto fftAmpSource =
        new FFTDataSource{FFTDataSource::Amplitude, serialWorker};

    connect(this, &MainWindow::windowExited, fftAmpSource,
            &DataSource::requestStopDataSource);

    connect(ui->bClearPlots, &QPushButton::clicked, fftAmpSource,
            &FFTDataSource::clearAllData);

    auto fftAmpPlot =
        createNewPlot(fftAmpSource, 0, "FFT with " + settings.portName,
                      QPen{QColor{0xfe, 0x5a, 0x5b}}, ReusePlot,
                      {serialWidgetPos.first + 1, serialWidgetPos.second});
    auto fftAmpCustomPlot = qobject_cast<CustomPlot*>(fftAmpPlot->parentPlot());
    fftAmpCustomPlot->xAxis->setLabel("Frequency (Hz)");
    fftAmpCustomPlot->yAxis->setLabel("Amptitute (V)");

    auto fftAmpThread = new QThread{this};
    connect(fftAmpThread, &QThread::started, fftAmpSource, &DataSource::run);
    connect(fftAmpThread, &QThread::finished, fftAmpSource,
            &FFTDataSource::deleteLater);
    connect(th, &QThread::finished, fftAmpThread, &QThread::quit);
    fftAmpSource->moveToThread(fftAmpThread);
    sourceToThreadMap.insert(fftAmpSource->getId(0),
                             {fftAmpSource, fftAmpThread});
    fftAmpThread->start();

    // 相位谱
    auto fftPhaseSource = new FFTDataSource{FFTDataSource::Phase, serialWorker};

    connect(this, &MainWindow::windowExited, fftPhaseSource,
            &DataSource::requestStopDataSource);

    connect(ui->bClearPlots, &QPushButton::clicked, fftPhaseSource,
            &FFTDataSource::clearAllData);

    auto fftPhasePlot =
        createNewPlot(fftPhaseSource, 0, "FFT with " + settings.portName,
                      QPen{QColor{0x66, 0xcc, 0xff}}, ReusePlot,
                      {serialWidgetPos.first + 2, serialWidgetPos.second});
    auto fftPhaseCustomPlot =
        qobject_cast<CustomPlot*>(fftPhasePlot->parentPlot());
    fftPhaseCustomPlot->xAxis->setLabel("Frequency (Hz)");
    fftPhaseCustomPlot->yAxis->setLabel("Phase (Angle)");

    auto fftPhaseThread = new QThread{this};
    connect(fftPhaseThread, &QThread::started, fftPhaseSource,
            &DataSource::run);
    connect(fftPhaseThread, &QThread::finished, fftPhaseSource,
            &FFTDataSource::deleteLater);
    connect(th, &QThread::finished, fftPhaseThread, &QThread::quit);
    fftPhaseSource->moveToThread(fftPhaseThread);
    sourceToThreadMap.insert(fftPhaseSource->getId(0),
                             {fftPhaseSource, fftPhaseThread});
    fftPhaseThread->start();
}
