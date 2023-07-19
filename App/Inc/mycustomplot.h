#ifndef __M_MYCUSTOMPLOT_H__
#define __M_MYCUSTOMPLOT_H__

#include <QMap>

#include "datasource.h"
#include "pch.h"
#include "qcustomplot.h"

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

   private:
    void initChart();
    void initAxis();
    void initSeries();

   private:
    QMap<DataSource::DSID, QCPGraph*> sourceToGraphMap;
};

#endif /* __M_MYCUSTOMPLOT_H__ */
