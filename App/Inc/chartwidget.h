#ifndef __M_CHARTWIDGET_H__
#define __M_CHARTWIDGET_H__

#include <QChart>
#include <QChartView>
#include <QCloseEvent>
#include <QLineSeries>
#include <QMouseEvent>
#include <QPointF>
#include <QQueue>
#include <QRunnable>
#include <QThread>
#include <QThreadPool>
#include <QWidget>

class MyChartView : public QChartView {
    Q_OBJECT

   public:
    MyChartView(QChart *chart, QWidget *parent = nullptr);

   protected:
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
};

class MySeries : public QLineSeries {
    Q_OBJECT

   public:
    MySeries(qreal step, QObject *parent = nullptr);
    ~MySeries();

    inline qreal getStep() const { return step; }
    inline void setStep(qreal value) { step = value; }

    inline void addStepValues(const QVector<qreal> &y) {
        QVector<QPointF> newPoints;
        std::for_each(y.begin(), y.end(), [&](qreal y) {
            newPoints.push_back(QPointF(maxX, y));
            maxX += step;
            // std::tie(minY, maxY) = std::minmax<qreal>({minY, y, maxY});
            minY = std::min(minY, y);
            maxY = std::max(maxY, y);
        });

        append(newPoints);
    }

    inline auto getMinMaxY() const { return std::make_pair(minY, maxY); }
    inline auto getMaxX() const { return maxX; }

   private:
    // x axis increase step (us)
    qreal step = 0;

    // x axis current max value
    qreal maxX = 0;

    // y axis current minmax value
    qreal minY = 0;
    qreal maxY = 0;
};

class SeriesInsertDataWorker : public QObject, public QRunnable {
    Q_OBJECT;

   public:
    SeriesInsertDataWorker(MySeries *newSeries, MySeries const *oldSeries,
                           QVector<QPointF> oldPoints, QVector<qreal> y)
        : QObject(nullptr),
          newSeries(newSeries),
          oldSeries(oldSeries),
          oldPoints{oldPoints},
          y(y),
          QRunnable{} {
        setAutoDelete(true);
    }
    ~SeriesInsertDataWorker() { ; }

   public:
    virtual void run() override {
        newSeries->addStepValues(y);
        emit done(newSeries, oldSeries);
    }

   signals:
    void done(MySeries *newSeries, MySeries const *oldSeries);

   private:
    MySeries *newSeries;
    MySeries const *oldSeries;
    QVector<qreal> y;
    QVector<QPointF> oldPoints;
};

class ChartWidget : public QWidget {
    Q_OBJECT

   public:
    ChartWidget(QString title, QWidget *parent = nullptr);
    ~ChartWidget();

    const QVector<MySeries *> getSeries();
    void addSeries(MySeries *);
    bool removeSeries(MySeries *);

    qreal getStep(qsizetype index);

   signals:
    void closed(ChartWidget *);

   public slots:
    // add data point to series & update chart
    void addData(QVector<qreal> y, qsizetype index);
    void setStep(qreal step, qsizetype index);

   protected:
    virtual void closeEvent(QCloseEvent *event) override;

   private slots:
    void onAddDataThreadDone(MySeries *newSeries, MySeries const *oldSeries);

   private:
    void updatePendingData();

   private:
    QChart *chart = nullptr;
    QVector<MySeries *> series;
    MyChartView *chartView = nullptr;

    QThreadPool *threadPool = nullptr;
    bool isThreadDone = true;
    QMap<MySeries *, QVector<qreal>> pendingData;
};

#endif /* __M_CHARTWIDGET_H__ */
