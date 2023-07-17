#ifndef __M_DATASOURCE_H__
#define __M_DATASOURCE_H__

#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QQueue>
#include <QTimer>

#include <QLineSeries>
#include "chartwidget.h"

class DataSource : public QObject {
    Q_OBJECT;

    constexpr static auto dataUpdateInterval = 3000;

   public:
    DataSource(QObject* parent = nullptr);
    virtual ~DataSource();

   private slots:
    void updateData();

   private:
    QTimer* updateTimer;

   signals:
    void error(QString);
    void controlWordReceived(QByteArray);

    // send data to chart, updated by updateData() with updateTimer
    void dataReceived(QVector<qreal>);
    void dataReceivedSer(MySeries*);

   protected:
    void appendData(qreal);
    void appendData(QVector<qreal>);
    QVector<qreal> dataQueue;
};

#endif /* __M_DATASOURCE_H__ */
