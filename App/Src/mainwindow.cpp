#include "mainwindow.h"

#include <QFontDatabase>
#include <QHBoxLayout>

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

    // init chart widget layout
    ui->chartContainer->setLayout(new QHBoxLayout{});

    // connect push buttons
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
    // serial config btn
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
                            &MainWindow::onSourceError);
                    connect(serialWorker, &SerialWorker::controlWordReceived,
                            this, &MainWindow::onSourceControlWordReceived);

                    connect(this, &MainWindow::serialCloseRequest, serialWorker,
                            &SerialWorker::closeSerial);

                    // TODO
                    // threadPool->start(serialWorker);
                    serialThread = new QThread{};
                    serialWorker->moveToThread(serialThread);
                    connect(serialThread, &QThread::started, serialWorker,
                            &SerialWorker::run);
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

    auto sender = getDataSourceSender();

    if (controlWord == "%START") {
        if (dataSources.contains(sender)) {
            printCurrentTime()
                << "Data source already exists. %START twice" << std::endl;
            return;
        }

        auto newSeries = new MySeries{1};
        dataSources.insert(sender, newSeries);

        switch (newDataStrategy) {
            case NewDataStrategy::ReusePlot: {
                if (currentSelectedPlot == nullptr) {
                    // TODO
                    auto newPlot = createNewPlot(sender->objectName());
                    newPlot->show();
                    currentSelectedPlot = newPlot;
                    connect(newPlot, &ChartWidget::closed, this,
                            [this,sender](ChartWidget* plot) {
                                plots.removeOne(plot);
                                if (currentSelectedPlot == plot) {
                                    currentSelectedPlot = plots.isEmpty() ? nullptr : plots.last();
                                }

                                sender->deleteLater();
                            });
                }

                currentSelectedPlot->addSeries(newSeries);

                // connect data source to plot
                connect(sender, &DataSource::dataReceived, currentSelectedPlot,
                        [this, newSeries](QVector<qreal> data) {
                            if (currentSelectedPlot == nullptr) {
                                return;
                            }

                            currentSelectedPlot->addData(
                                data, currentSelectedPlot->getSeries().indexOf(
                                          newSeries));
                        });
            } break;

            default:
                break;
        }
    }

    if (controlWord.startsWith("%t")) {
        auto timeStep = controlWord.mid(2).toDouble();
        printCurrentTime() << "set Time step: " << timeStep << std::endl;

        if (!dataSources.contains(sender)) {
            return;
        }

        dataSources[sender]->setStep(timeStep);
    }

    // ui->chartContainer->layout()->addWidget(
    //     new ChartWidget{"CHART_TEST", this});
    // ui->chartContainer->layout()->addWidget(
    //     new ChartWidget{"CHART_TEST2", this});
}

ChartWidget* MainWindow::createNewPlot(QString title) {
    auto newPlot = new ChartWidget{title, nullptr};
    plots.push_back(newPlot);
    // ui->chartContainer->layout()->addWidget(newPlot);

    currentSelectedPlot = newPlot;

    return newPlot;
}
