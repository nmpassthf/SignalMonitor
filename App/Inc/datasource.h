#ifndef __M_DATASOURCE_H__
#define __M_DATASOURCE_H__

#include <QObject>
#include <QPointF>
#include <QQueue>
#include <QTimer>
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
    DataSource(QObject* parent = nullptr);
    virtual ~DataSource();

   public slots:
    virtual void run() = 0;
    inline void requestStopDataSource() { isTerminateSerial = true; };

   signals:
    void finished();

    void error(QString);
    void controlWordReceived(QByteArray);

    // send data to chart, updated by updateData() with updateTimer
    void dataReceived(const QVector<double> x, const QVector<double> y);

   protected:
    void appendData(QVector<double> x, QVector<double> y);

   protected:
    std::atomic<bool> isTerminateSerial = false;

   private slots:
    void updateData();

   private:
    QTimer* updateTimer;
    QVector<double> dataX, dataY;
};

#endif /* __M_DATASOURCE_H__ */
