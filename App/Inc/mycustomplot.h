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

    void setData(QPair <double,double> data);
};

class CustomPlot : public QCustomPlot {
    Q_OBJECT;

   public:
    CustomPlot(QWidget* parent = nullptr);
    ~CustomPlot();

   public:
    /**
     * @brief call after addDataSource
     *
     * @param source
     */
    void connectDataSource(DataSource* source);

    QCPGraph* addDataSource(DataSource::DSID source);
    void removeDataSource(DataSource::DSID source);
    QCPGraph* getGraph(DataSource::DSID source);

    bool isDataSourceExist(DataSource::DSID source) const;

   protected:
   private:
    void initChart();
    void initAxis(QStringList axesLabel);
    void initSeries();

   private:
    QMap<DataSource::DSID, QCPGraph*> sourceToGraphMap;
    DataLabel* dataLabel;
};

#endif /* __M_MYCUSTOMPLOT_H__ */
