/**
 * @file serial.cpp
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-15
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#include "serial.h"

#include <QCoreApplication>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QThread>

#include "pch.h"

SerialSettingsDiag::SerialSettingsDiag(QDialog *parent) : QDialog{parent} {
    // set delete when close
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle("Serial settings");
    setWindowIcon(QIcon{":/amamiya.ico"});

    setLayout(new QFormLayout{});

    initBtns();

    QHBoxLayout *rButtonsLayout = new QHBoxLayout{};
    rButtonsLayout->addWidget(bOpenSerial);
    rButtonsLayout->addWidget(bCancel);

    initComboBox();

    initCheckBox();

    auto currentWidgetLayout = qobject_cast<QFormLayout *>(layout());

    currentWidgetLayout->addRow("Port", cActivatedPort);
    currentWidgetLayout->addRow("Baud rate", cBaudRate);
    currentWidgetLayout->addRow("Stop bits", cStopBits);
    currentWidgetLayout->addRow("Data bits", cDataBits);
    currentWidgetLayout->addRow("Parity", cParity);
    currentWidgetLayout->addRow("Flow control", cFlowControl);
    currentWidgetLayout->addRow(cIsTimeDomainData);
    currentWidgetLayout->addRow(rButtonsLayout);

    // lock the size of the dialog
    setFixedSize(sizeHint());
}
SerialSettingsDiag::~SerialSettingsDiag() {
    printCurrentTime() << "SerialSettingsDiag::~SerialSettingsDiag()";
}

void SerialSettingsDiag::initBtns() {
    bOpenSerial = new QPushButton{"Open serial", this};
    bCancel = new QPushButton{"Cancel", this};

    connect(bOpenSerial, &QPushButton::clicked, this, [this]() {
        SerialSettings settings;
        bool ok = true;
        auto errorMessageBox = [this](QString errorMsg) {
            QMessageBox::critical(this, "Error", errorMsg);
        };

        settings.port = QSerialPortInfo::availablePorts().at(
            cActivatedPort->currentIndex());

        settings.baudRate = cBaudRate->currentText().toInt(&ok);
        if (!ok) {
            errorMessageBox("Baud rate is not a number");
            return;
        }

        settings.stopBits = static_cast<QSerialPort::StopBits>(
            cStopBits->currentData().toInt());

        settings.dataBits = static_cast<QSerialPort::DataBits>(
            cDataBits->currentText().toInt());

        settings.parity =
            static_cast<QSerialPort::Parity>(cParity->currentData().toInt());

        settings.flowControl = static_cast<QSerialPort::FlowControl>(
            cFlowControl->currentData().toInt());

        settings.portName = settings.port.portName();

        settings.isTimeDomainData = cIsTimeDomainData->isChecked();

        emit settingsReceived(settings);
        close();
    });
    connect(bCancel, &QPushButton::clicked, this, [this]() {
        emit settingsReceived(std::nullopt);
        close();
    });
}

void SerialSettingsDiag::initComboBox() {
    cActivatedPort = new QComboBox{this};
    cActivatedPort->setEditable(false);
    cActivatedPort->setInsertPolicy(QComboBox::InsertAtBottom);
    cActivatedPort->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto ports = QSerialPortInfo::availablePorts();
    for (auto &port : ports) {
        cActivatedPort->addItem(port.portName());
    }

    cBaudRate = new QComboBox{this};
    cBaudRate->setEditable(true);

    for (auto baudRate :
         {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}) {
        cBaudRate->addItem(QString::number(baudRate));
    }
    // set defalult baud rate
    cBaudRate->setCurrentText("115200");

    cStopBits = new QComboBox{this};
    cStopBits->setEditable(false);

    cStopBits->addItem("1", QSerialPort::OneStop);
    cStopBits->addItem("1.5", QSerialPort::OneAndHalfStop);
    cStopBits->addItem("2", QSerialPort::TwoStop);

    cDataBits = new QComboBox{this};
    cDataBits->setEditable(false);

    for (auto dataBits : {QSerialPort::Data5, QSerialPort::Data6,
                          QSerialPort::Data7, QSerialPort::Data8}) {
        cDataBits->addItem(QString::number(dataBits));
    }
    // set default data bits
    cDataBits->setCurrentText("8");

    cParity = new QComboBox{this};
    cParity->setEditable(false);

    cParity->addItem("No Parity", QSerialPort::NoParity);
    cParity->addItem("Even Parity", QSerialPort::EvenParity);
    cParity->addItem("Odd Parity", QSerialPort::OddParity);
    cParity->addItem("Space Parity", QSerialPort::SpaceParity);
    cParity->addItem("Mark Parity", QSerialPort::MarkParity);

    cFlowControl = new QComboBox{this};
    cFlowControl->setEditable(false);

    cFlowControl->addItem("No Flow Control", QSerialPort::NoFlowControl);
    cFlowControl->addItem("Hardware Control", QSerialPort::HardwareControl);
    cFlowControl->addItem("Software Control", QSerialPort::SoftwareControl);
}

void SerialSettingsDiag::initCheckBox() {
    cIsTimeDomainData = new QCheckBox{"is Time domain data", this};

    cIsTimeDomainData->setChecked(true);
}

SerialWorker::SerialWorker(QObject *parent)
    : DataSource{parent},
      DataStreamParser{DataStreamParser::SourceType::StringStream} {
    printCurrentTime() << "SerialWorker::SerialWorker()";
}
SerialWorker::~SerialWorker() {
    printCurrentTime() << "SerialWorker::~SerialWorker()";
}

void SerialWorker::setSerialSettings(SerialSettings settings) {
    QMutexLocker locker{&mutex};
    this->settings = settings;
}

bool SerialWorker::openSerial() {
    if (serial->open(QIODevice::ReadWrite)) {
        serial->write("Ok!");
        return true;
    } else {
        emit error(serial->errorString());
        return false;
    }
}

void SerialWorker::run() {
    printCurrentTime() << "SerialWorker::run() @" << QThread::currentThreadId();

    bool isStoped = false;

    serial = new QSerialPort{this};

    auto tagToExit = [&]() {
        if (!(isStoped = !isStoped))
            return;

        requestStopDataSource();
        emit finished();
        printCurrentTime() << "SerialWorker::run() end";
    };

    connect(serial, &QSerialPort::aboutToClose, this,
            &SerialWorker::requestStopDataSource);

    connect(serial, &QSerialPort::errorOccurred, this,
            [this, tagToExit](QSerialPort::SerialPortError e) {
                if (e == QSerialPort::NoError || e == QSerialPort::TimeoutError)
                    return;

                emit error(serial->errorString());
                tagToExit();
            });

    {
        QMutexLocker locker{&mutex};
        serial->setBaudRate(settings.baudRate);
        serial->setStopBits(settings.stopBits);
        serial->setDataBits(settings.dataBits);
        serial->setParity(settings.parity);
        serial->setFlowControl(settings.flowControl);
        serial->setPort(settings.port);
    }

    if (!openSerial()) {
        emit error("Can't open serial port:" + serial->errorString());
        tagToExit();
        return;
    }

    auto parseDataAndSend = [&]() {
        auto result = parseData();
        if (result == std::nullopt) {
            return false;
        }

        switch (result->first) {
            case DataStreamParser::RDataType::RDataPointF: {
                auto data = result->second.toPointF();
                DataSource::appendData({data.x()}, {data.y()});
            } break;

            case DataStreamParser::RDataType::RDataControlWord: {
                auto [controlWord, controlWordData] =
                    DataSource::parseControlWord(result->second.toByteArray());
                emit controlWordReceived(currentSelectedChannel, controlWord,
                                         controlWordData);
            } break;

            case DataStreamParser::RDataType::RDataErrorString: {
                emit error(result->second.toString());
            } break;
        }

        return true;
    };

    bool isStart = false;
    auto detectSerialStartFlag = [&]() {
        QByteArray inputBuffer{};
        QCoreApplication::processEvents();
        while (!isStart) {
            QCoreApplication::processEvents();

            if (isTerminateSerial) {
                break;
            }

            if (!serial->waitForReadyRead(200))
                continue;

            inputBuffer.append(serial->readAll());

            if (!inputBuffer.contains("%START")) {
                inputBuffer = inputBuffer.last(
                    inputBuffer.size() < 5 ? inputBuffer.size() : 5);
                continue;
            }

            DataStreamParser::appendData(
                inputBuffer.mid(inputBuffer.indexOf("%START")));
            parseDataAndSend();
            isStart = true;
        }
    };

    while (!isTerminateSerial && serial->isOpen()) {
        // process events
        QCoreApplication::processEvents();

        if (isTerminateSerial)
            break;

        if (!isStart)
            detectSerialStartFlag();

        if (!serial->waitForReadyRead(200))
            continue;

        auto d = serial->readAll();
        DataStreamParser::appendData(d);
        while (parseDataAndSend())
            ;
    }

    tagToExit();
    serial->close();
}

void SerialWorker::clearAllData() {
    QMutexLocker locker{&mutex};
    x.clear();
    x.append(0.0);

    step.clear();
    step.append(1.0);

    currentSelectIndex = 0;

    DataSource::clearAllData();
}

void SerialWorker::onControlWordReceived(qsizetype index,
                                         DataControlWords words,
                                         QByteArray data) {
    switch (words) {
        case DataControlWords::SetXAxisStep: {
            step[currentSelectIndex] = data.toDouble();
        } break;

        case DataControlWords::SlelectSubplot: {
            currentSelectIndex = data.toInt();

            if (currentSelectIndex >= step.size()) {
                step.append(1.0);
                x.append(0.0);
            }
        } break;

        default:
            break;
    }
    DataSource::onControlWordReceived(index, words, data);
}
