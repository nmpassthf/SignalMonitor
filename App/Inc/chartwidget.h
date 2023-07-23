/**
 * @file chartwidget.h
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-20
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#ifndef __M_CHARTWIDGET_H__
#define __M_CHARTWIDGET_H__

#include <pch.h>

#include <QPair>
#include <QVector>
#include <QWidget>

#include "datasource.h"
#include "mycustomplot.h"

namespace Ui {
class ChartWidgetToolBar;
}

class ChartWidgetToolBar : public QWidget {
    Q_OBJECT;

   public:
    ChartWidgetToolBar(QWidget* parent = nullptr);
    ~ChartWidgetToolBar();

    Ui::ChartWidgetToolBar* ui;
};

class ChartWidget : public QWidget {
    Q_OBJECT;

   public:
    /**
     * @brief 子图位置(row, col), 从0开始计数
     *
     */
    using PlotPos_t = QPair<int, int>;

    ChartWidget(QWidget* parent = nullptr);
    ~ChartWidget();

   signals:
    // TODO
    void windowExited(QVector<DataSource::DSID> ids);
    void plotRemoved(DataSource::DSID id);

   public:
    QPair<CustomPlot*, QCPGraph*> addPlot(DataSource::DSID id,
                                          PlotPos_t pos = {-1, -1});
    QCPGraph* insertAtPlot(DataSource::DSID id, PlotPos_t pos);

    void removePlot(PlotPos_t pos);
    void removePlot(DataSource::DSID id);
    void removeAllPlots();

    CustomPlot* getPlot(PlotPos_t pos) const;
    QVector<CustomPlot*> getPlots() const;
    void clearPlot(PlotPos_t pos);

    /**
     * @brief 返回Widget中已使用最大的行数和列数
     *
     * @return PlotPos_t
     */
    PlotPos_t getSubplotCount() const;

    PlotPos_t getAvailablePos() const;
    PlotPos_t getPlotPos(CustomPlot* plot) const;

    bool isDataSourceExist(DataSource::DSID id) const;

   public slots:
    void clearPlot(DataSource::DSID id);
    void clearPlots();

   private:
    void initLayout();
    void initToolBar();

   private:
    QVector<QPair<CustomPlot*, PlotPos_t>> subplots;
    QGridLayout* chartWidgetLayout;

    ChartWidgetToolBar* toolBar;
};

#endif /* __M_CHARTWIDGET_H__ */
