#include "serial.h"

#include <QCoreApplication>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>

#include "pch.h"

SerialSettingsDiag::SerialSettingsDiag(QDialog *parent) : QDialog{parent} {
    // set delete when close
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle("Serial settings");
    setWindowIcon(QIcon{":/amamiya.ico"});

	// TODO fix lb
    setLayout(new QVBoxLayout{});

    initBtns();

    QHBoxLayout *rButtonsLayout = new QHBoxLayout{};
    rButtonsLayout->addWidget(bOpenSerial);
    rButtonsLayout->addWidget(bCancel);

    initComboBox();

    layout()->addWidget(cActivatedPort);
    layout()->addWidget(cBaudRate);
    layout()->addWidget(cStopBits);
    layout()->addWidget(cDataBits);
    layout()->addWidget(cParity);
    layout()->addWidget(cFlowControl);
    layout()->addItem(rButtonsLayout);
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
        auto errorMessageBox = [ok, this](QString errorMsg) {
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
            cStopBits->currentText().toInt(&ok));

        settings.dataBits = static_cast<QSerialPort::DataBits>(
            cDataBits->currentText().toInt(&ok));

        settings.parity =
            static_cast<QSerialPort::Parity>(cParity->currentText().toInt(&ok));

        settings.flowControl = static_cast<QSerialPort::FlowControl>(
            cFlowControl->currentText().toInt(&ok));

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
    for (auto port : ports) {
        cActivatedPort->addItem(port.portName());
    }

    cBaudRate = new QComboBox{this};
    cBaudRate->setEditable(true);

    for (auto baudRate :
         {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200}) {
        cBaudRate->addItem(QString::number(baudRate));
    }

    cStopBits = new QComboBox{this};
    cStopBits->setEditable(false);

    for (auto stopBits : {QSerialPort::OneStop, QSerialPort::OneAndHalfStop,
                          QSerialPort::TwoStop}) {
        cStopBits->addItem(QString::number(stopBits));
    }

    cDataBits = new QComboBox{this};
    cDataBits->setEditable(false);

    for (auto dataBits : {QSerialPort::Data5, QSerialPort::Data6,
                          QSerialPort::Data7, QSerialPort::Data8}) {
        cDataBits->addItem(QString::number(dataBits));
    }

    cParity = new QComboBox{this};
    cParity->setEditable(false);

    for (auto parity : {QSerialPort::NoParity, QSerialPort::EvenParity,
                        QSerialPort::OddParity, QSerialPort::SpaceParity,
                        QSerialPort::MarkParity}) {
        cParity->addItem(QString::number(parity));
    }

    cFlowControl = new QComboBox{this};
    cFlowControl->setEditable(false);

    for (auto flowControl :
         {QSerialPort::NoFlowControl, QSerialPort::HardwareControl,
          QSerialPort::SoftwareControl}) {
        cFlowControl->addItem(QString::number(flowControl));
    }
}

SerialWorker::SerialWorker(QObject *parent) : QObject{parent} {
    printCurrentTime() << "SerialWorker::SerialWorker()" << std::endl;

    connect(&parser, &DataStreamParser::dataReceived, this,
            &SerialWorker::dataReceived);
    connect(&parser, &DataStreamParser::controlWordReceived, this,
            &SerialWorker::controlWordReceived);
    connect(&parser, &DataStreamParser::error, this, &SerialWorker::error);
}
SerialWorker::~SerialWorker() {
    printCurrentTime() << "SerialWorker::~SerialWorker()" << std::endl;

    if (serial->isOpen())
        serial->close();
    delete serial;
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
    // TODO
    emit dataReceived("Serial worker started");

    serial = new QSerialPort{};

    connect(serial, &QSerialPort::readyRead, this,
            [this]() { parser.parse(serial->readAll()); });

    serial->setBaudRate(settings.baudRate);
    serial->setStopBits(settings.stopBits);
    serial->setDataBits(settings.dataBits);
    serial->setParity(settings.parity);
    serial->setFlowControl(settings.flowControl);
    serial->setPort(settings.port);

    if (!openSerial())
        emit error("Can't open serial port");

    while (!isTerminateSerial && serial->isOpen()) {
        parser.parse(serial->readAll());

        // process events
        QCoreApplication::processEvents();
    }
}
