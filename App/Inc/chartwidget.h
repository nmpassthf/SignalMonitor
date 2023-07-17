#ifndef __M_CHARTWIDGET_H__
#define __M_CHARTWIDGET_H__

#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QMouseEvent>
#include <QQueue>
#include <QWidget>
#include <QPointF>

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
    inline void pushAStep(qreal y) {
        append(maxX, y);
        maxX += step;
        // std::tie(minY, maxY) = std::minmax<qreal>({minY, y, maxY});
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
    }
    inline void pushAStep(const QVector<QPointF>& y) {
        append(y);
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

class ChartWidget : public QWidget {
    Q_OBJECT

   public:
    ChartWidget(QString title, QWidget *parent = nullptr);
    ~ChartWidget();

    const QVector<MySeries *> getSeries();
    void addSeries(MySeries *);

    qreal getStep(qsizetype index);

   public slots:
    // add data point to series & update chart
    void addData(qreal y, qsizetype index);
    void addData(QVector<qreal> y, qsizetype index);
    void addData(MySeries* s, qsizetype index); // TODO test QLineSeries*
    void setStep(qreal step, qsizetype index);

   private:
    QChart *chart = nullptr;
    QVector<MySeries *> series;
    MyChartView *chartView = nullptr;
};

#endif /* __M_CHARTWIDGET_H__ */
