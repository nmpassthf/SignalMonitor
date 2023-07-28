/**
 * @file serialsettingsdiag.h
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-15
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#ifndef __M_SERIAL_H__
#define __M_SERIAL_H__

#include <QByteArray>
#include <QComboBox>
#include <QDialog>
#include <QMutex>
#include <QMutexLocker>
#include <QPushButton>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QCheckBox>
#include <optional>

#include "dataStreamParser.h"
#include "datasource.h"

struct SerialSettings {
    qint32 baudRate;
    QSerialPort::StopBits stopBits;
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::FlowControl flowControl;
    QSerialPortInfo port;
    QString portName;

    bool isTimeDomainData;
};

class SerialSettingsDiag : public QDialog {
    Q_OBJECT;

   public:
    SerialSettingsDiag(QDialog *parent = nullptr);
    ~SerialSettingsDiag();

   signals:
    void settingsReceived(std::optional<SerialSettings>);

   private:
    QPushButton *bOpenSerial, *bCancel;
    QComboBox *cActivatedPort;
    QComboBox *cBaudRate, *cStopBits, *cDataBits, *cParity, *cFlowControl;

    QCheckBox* cIsTimeDomainData;

   private:
    void initBtns();
    void initComboBox();
    void initCheckBox();
};

class SerialWorker : public DataSource {
    Q_OBJECT;

   public:
    SerialWorker(QObject *parent = nullptr);
    ~SerialWorker();

    void setSerialSettings(SerialSettings);

   public:
    virtual void run() override;

   private:
    bool openSerial();

    // shared data
   private:
    SerialSettings settings;

    // private non-shared data
   private:
    QSerialPort *serial;
    DataStreamParser *parser;
    QMutex mutex;
};

#endif /* __M_SERIAL_H__ */
