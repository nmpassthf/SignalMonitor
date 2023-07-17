#ifndef __M_SERIAL_H__
#define __M_SERIAL_H__
/**
 * @file serialsettingsdiag.h
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-15
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */

#include <QByteArray>
#include <QComboBox>
#include <QDialog>
#include <QMutex>
#include <QMutexLocker>
#include <QPushButton>
#include <QRunnable>
#include <QSerialPort>
#include <QSerialPortInfo>
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

   private:
    void initBtns();
    void initComboBox();
};

class SerialWorker : public DataSource {
    Q_OBJECT;

   public:
    SerialWorker(QObject *parent = nullptr);
    ~SerialWorker();

   public:
    void setSerialSettings(SerialSettings);

   public:
    void run();

   public slots:
    inline void closeSerial() { isTerminateSerial = true; };

   private:
    // shared data
    bool isTerminateSerial = false;
    SerialSettings settings;

   private:
    QSerialPort *serial;
    DataStreamParser *parser;

   private:
    bool openSerial();
};

#endif /* __M_SERIAL_H__ */
