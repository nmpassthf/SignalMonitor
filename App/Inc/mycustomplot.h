/**
 * @file mycustomPlot.h
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-21
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#ifndef __M_MYCUSTOMPLOT_H__
#define __M_MYCUSTOMPLOT_H__

#include <QEvent>
#include <QLabel>
#include <QMap>

#include "datasource.h"
#include "pch.h"
#include "qcustomplot.h"

class DataLabel : public QLabel {
    Q_OBJECT;

   public:
    DataLabel(QWidget* parent = nullptr);
    ~DataLabel();

    void setData(QPair<double, double> data);
};

class CustomPlot : public QCustomPlot {
    Q_OBJECT;

   public:
    CustomPlot(QWidget* parent = nullptr);
    ~CustomPlot();

   public:
    QCPGraph* addDataSource(DataSource::DSID source);
    void removeDataSource(DataSource::DSID source);
    QCPGraph* getGraph(DataSource::DSID source);

    bool isDataSourceExist(DataSource::DSID source) const;

    void setPlotUnit(QByteArray xUnit, QByteArray yUnit);

   private:
    void initChart();
    void initAxis(QStringList axesLabel);
    void initSeries();

   private:
    QMap<DataSource::DSID, QCPGraph*> sourceToGraphMap;
    DataLabel* dataLabel;
};

#endif /* __M_MYCUSTOMPLOT_H__ */
