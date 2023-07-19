#ifndef __M_CHARTWIDGET_H__
#define __M_CHARTWIDGET_H__

#include <QPair>
#include <QPoint>
#include <QVector>
#include <QWidget>

#include <pch.h>
#include "datasource.h"
#include "mycustomplot.h"

class ChartWidget : public QWidget {
    Q_OBJECT;

   public:
    ChartWidget(QWidget* parent = nullptr);
    ~ChartWidget();

   public:
    QPair<CustomPlot*, QCPGraph*> addPlot(DataSource::DSID id,
                                          QPoint pos = {-1, -1});
    QCPGraph* insertAtPlot(DataSource::DSID id, QPoint pos);

    void removePlot(QPoint pos);
    void removePlot(DataSource::DSID id);
    void removeAllPlots();

    CustomPlot* getPlot(QPoint pos) const;
    QVector<CustomPlot*> getPlots();

    /**
     * @brief 返回当前子图的行数和列数
     *
     * @return QPair<int, int>
     */
    QPair<int, int> getSubplotCount() const;

    QPoint getAvailablePos() const;

    bool isDataSourceExist(DataSource::DSID id) const;

   private:
    QVector<QPair<CustomPlot*, QPoint>> subplots;
    QGridLayout* chartWidgetLayout;
};

#endif /* __M_CHARTWIDGET_H__ */
