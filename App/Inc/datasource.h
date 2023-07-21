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

#include <QObject>
#include <QPointF>
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
        ClearDatas,
        SetXAxisStep,
        UserDefined,
    };

    DataSource(QObject* parent = nullptr);
    virtual ~DataSource();

    inline DSID getId() const { return uuid; };

   public slots:
    virtual void run() = 0;
    inline void requestStopDataSource() { isTerminateSerial = true; };

   signals:
    void finished();

    void error(QString);
    void controlWordReceived(DataControlWords,
                             [[maybe_unused]] QByteArray DCWData = {});

    // send data to chart, updated by updateData() with updateTimer
    void dataReceived(const QVector<double> x, const QVector<double> y);

   protected:
    void appendData(QVector<double> x, QVector<double> y);
    void clearQueuedData();
    virtual QPair<DataControlWords, QByteArray> parseControlWord(
        QByteArray data) const;

   protected:
    std::atomic<bool> isTerminateSerial = false;

   private slots:
    void updateData();

   private:
    QTimer* updateTimer;
    QVector<double> dataX, dataY;
    const DSID uuid;
};

#endif /* __M_DATASOURCE_H__ */
