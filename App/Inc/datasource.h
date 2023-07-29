/**
 * @file datasource.h
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-20
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#ifndef __M_DATASOURCE_H__
#define __M_DATASOURCE_H__

#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QPair>
#include <QQueue>
#include <QTimer>
#include <QUuid>
#include <atomic>

/**
 * @brief Data source for chart
 *
 * Run in another thread
 */
class DataSource : public QObject {
    Q_OBJECT;

    constexpr static auto dataUpdateInterval = 1000 / 30;

   public:
    using DSID = QUuid;

    using DataControlWords = enum {
        DataStreamStart,
        DataStreamStop,
        SlelectSubplot,
        SetXAxisStep,
        SetXAxisRange,
        ClearDatas,
        SetUseLogAxis,
        SetPlotName,
        SetPlotUnit,
        UserDefined,
    };
    inline static constexpr const char* dataControlWordsToString(
        DataControlWords dcw) {
        switch (dcw) {
            case DataStreamStart:
                return "DataStreamStart";
            case DataStreamStop:
                return "DataStreamStop";
            case SlelectSubplot:
                return "SlelectSubplot";
            case SetXAxisStep:
                return "SetXAxisStep";
            case SetXAxisRange:
                return "SetXAxisRange";
            case ClearDatas:
                return "ClearDatas";
            case SetUseLogAxis:
                return "SetUseLogAxis";
            case SetPlotName:
                return "SetPlotName";
            case SetPlotUnit:
                return "SetPlotUnit";
            case UserDefined:
                return "UserDefined";
            default:
                return "Unknown";
        }
    }

    DataSource(QObject* parent = nullptr);
    virtual ~DataSource();

    DSID getId(qsizetype index);
    QVector<DSID> getIds();
    QPair<DataControlWords, QByteArray> parseControlWord(QByteArray data) const;

   public slots:
    virtual void run() = 0;
    inline void requestStopDataSource() { isTerminateSerial = true; };
    virtual void clearAllData();

   signals:
    void finished() const;
    void error(QString) const;
    void controlWordReceived(qsizetype index, DataControlWords,
                             [[maybe_unused]] QByteArray DCWData = {}) const;

    /**
     * @brief send data to chart, updated by updateData() with updateTimer
     *
     * @param index data source subplot index
     * @param x
     * @param y
     */
    void dataReceived(qsizetype index, const QVector<double> x,
                      const QVector<double> y);

    /**
     * @brief emit this signal when datasource request to create a new plot
     *
     * @param index
     * @param id
     */
    void newDataChannelCreated(qsizetype index, DSID id);

   protected:
    virtual void onControlWordReceived(qsizetype index, DataControlWords words,
                                       QByteArray data);

    void appendData(QVector<double> x, QVector<double> y);
    void clearQueuedData();

   protected:
    std::atomic<bool> isTerminateSerial = false;
    qsizetype currentSelectedChannel = 0;

   private slots:
    void updateData();

   private:
    QTimer* updateTimer;
    QVector<QVector<double>> dataX, dataY;
    QVector<DSID> uuid;
    QMutex uuidMutex;
};

#endif /* __M_DATASOURCE_H__ */
