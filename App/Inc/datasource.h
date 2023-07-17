#ifndef __M_DATASOURCE_H__
#define __M_DATASOURCE_H__

#include <QObject>
#include <QQueue>
#include <QTimer>

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

   signals:
    void error(QString);
    void controlWordReceived(QByteArray);

    // send data to chart, updated by updateData() with updateTimer
    void dataReceived(QVector<qreal>);

   protected:
    void appendData(QVector<qreal>);

   private slots:
    void updateData();

   private:
    QTimer* updateTimer;
    QVector<qreal> dataQueue;
};

#endif /* __M_DATASOURCE_H__ */
