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

                    createSerialDataSource(settings.value(),
                                           InsertAtMainWindow);
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
                                    QPen color, NewDataStrategy strategy,
                                    QPoint pos) {
    CustomPlot* rPlot = nullptr;
    QCPGraph* rSeries = nullptr;

    switch (strategy) {
        case ReusePlot: {
            if (currentSelectedPlot == nullptr) {
                throw std::runtime_error{"currentSelectedPlot is nullptr"};
            }

            std::tie(rPlot, rSeries) =
                currentSelectedPlot->addPlot(source->getId(), pos);

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
                newPlotWnd->addPlot(source->getId(), pos);

            currentSelectedPlot = newPlotWnd;
        } break;
        case InsertAtMainWindow: {
            std::tie(rPlot, rSeries) =
                ui->mainPlotWidget->addPlot(source->getId(), pos);

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
    connect(source, &DataSource::dataReceived, series,
            [series](const QVector<double> x, const QVector<double> y) {
                series->addData(x, y, true);

                series->rescaleAxes();

                series->parentPlot()->replot();
            });
    connect(
        source, &DataSource::controlWordReceived, series,
        [series](DataSource::DataControlWords controlWord, QByteArray DCWData) {
            switch (controlWord) {
                case DataSource::DataControlWords::ClearDatas:
                    series->setData({}, {});
                    break;

                default:
                    break;
            }
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

    // TODO Pen color customable
    auto newPlot = createNewPlot(serialWorker, settings.portName,
                                 QPen{QColor{0xfe, 0x5a, 0x5b}}, strategy);
    auto serialCustomPlot = qobject_cast<CustomPlot*>(newPlot->parentPlot());
    serialCustomPlot->yAxis->setLabel("Voltage (V)");
    serialCustomPlot->xAxis->setLabel("Time (us)");
    bindDataSource(serialWorker, newPlot);

    auto th = new QThread{this};
    connect(th, &QThread::started, serialWorker, &DataSource::run);
    connect(th, &QThread::finished, serialWorker, &SerialWorker::deleteLater);
    serialWorker->moveToThread(th);
    sourceToThreadMap.insert(serialWorker->getId(), {serialWorker, th});
    th->start();

    // 自动添加FFT图像在其下方
    auto fftSource = new FFTDataSource{serialWorker};

    connect(this, &MainWindow::windowExited, fftSource,
            &DataSource::requestStopDataSource);

    auto serialWidget =
        qobject_cast<ChartWidget*>(serialCustomPlot->parentWidget());
    auto serialWidgetPos = serialWidget->getPlotPos(serialCustomPlot);
    auto fftPlotPos = QPoint{serialWidgetPos.x(), serialWidgetPos.y() + 1};

    auto fftPlot =
        createNewPlot(fftSource, "FFT with " + settings.portName,
                      QPen{QColor{0xfe, 0x5a, 0x5b}}, ReusePlot, fftPlotPos);
    auto fftCustomPlot = qobject_cast<CustomPlot*>(fftPlot->parentPlot());
    fftCustomPlot->xAxis->setLabel("Frequency (Hz)");
    fftCustomPlot->yAxis->setLabel("Voltage (V)");
    bindDataSource(fftSource, fftPlot);

    auto fftTh = new QThread{this};
    connect(fftTh, &QThread::started, fftSource, &DataSource::run);
    connect(fftTh, &QThread::finished, fftSource, &FFTDataSource::deleteLater);
    connect(th, &QThread::finished, fftTh, &QThread::quit);
    fftSource->moveToThread(fftTh);
    sourceToThreadMap.insert(fftSource->getId(), {fftSource, fftTh});
    fftTh->start();
}
