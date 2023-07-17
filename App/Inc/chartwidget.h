#ifndef __M_CHARTWIDGET_H__
#define __M_CHARTWIDGET_H__

#include "qcustomplot.h"

class ChartWidget : public QCustomPlot {
    Q_OBJECT;

   public:
    ChartWidget(QWidget* parent = nullptr);
    ~ChartWidget();

   public slots:
    void updateData(QVector<qreal>);

   private:
    void initChart();
    void initAxis();
    void initSeries();

   private:
};

#endif /* __M_CHARTWIDGET_H__ */
