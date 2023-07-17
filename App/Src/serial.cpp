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

    auto currentWidgetLayout = qobject_cast<QFormLayout *>(layout());

    currentWidgetLayout->addRow("Port", cActivatedPort);
    currentWidgetLayout->addRow("Baud rate", cBaudRate);
    currentWidgetLayout->addRow("Stop bits", cStopBits);
    currentWidgetLayout->addRow("Data bits", cDataBits);
    currentWidgetLayout->addRow("Parity", cParity);
    currentWidgetLayout->addRow("Flow control", cFlowControl);
    currentWidgetLayout->addRow(rButtonsLayout);

    // lock the size of the dialog
    setFixedSize(sizeHint());
}
SerialSettingsDiag::~SerialSettingsDiag() {
    printCurrentTime() << "SerialSettingsDiag::~SerialSettingsDiag()"
                       << std::endl;
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
    for (auto& port : ports) {
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

SerialWorker::SerialWorker(QObject *parent) : DataSource{parent} {
    printCurrentTime() << "SerialWorker::SerialWorker()" << std::endl;
}
SerialWorker::~SerialWorker() {
    printCurrentTime() << "SerialWorker::~SerialWorker()" << std::endl;
}

void SerialWorker::setSerialSettings(SerialSettings settings) {
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
    printCurrentTime() << "SerialWorker::run() @" << QThread::currentThreadId()
                       << std::endl;

    parser = new DataStreamParser{};

    serial = new QSerialPort{};

    serial->setBaudRate(settings.baudRate);
    serial->setStopBits(settings.stopBits);
    serial->setDataBits(settings.dataBits);
    serial->setParity(settings.parity);
    serial->setFlowControl(settings.flowControl);
    serial->setPort(settings.port);

    if (!openSerial())
        emit error("Can't open serial port");

    auto parseDataAndSend = [&](const QByteArray &&rawData) {
        auto [data, ctrl, err] = parser->parse(rawData);
        if (!data.isEmpty())
            appendData(data);

        if (!ctrl.isEmpty())
            for (auto& singleCtrl : ctrl) emit controlWordReceived(singleCtrl);

        if (!err.isEmpty())
            for (auto& singleErr : err) emit error(singleErr);
    };

    bool isStart = false;
    auto detectSerialStartFlag = [&]() {
        QByteArray inputBuffer{};
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

            parseDataAndSend(inputBuffer.mid(inputBuffer.indexOf("%START")));
            isStart = true;
        }
    };

    while (!isTerminateSerial && serial->isOpen()) {
        if (!isStart)
            detectSerialStartFlag();

        // process events
        QCoreApplication::processEvents();

        if (!serial->waitForReadyRead(200))
            continue;

        parseDataAndSend(serial->readAll());
    }

    // end of work
    serial->close();
    delete serial;
    delete parser;
}
