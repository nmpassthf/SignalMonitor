#ifndef __M_MYCUSTOMPLOT_H__
#define __M_MYCUSTOMPLOT_H__

#include <QMap>

#include "qcustomplot.h"
#include "datasource.h"

class CustomPlot : public QCustomPlot {
    Q_OBJECT;

   public:
    CustomPlot(QWidget* parent = nullptr);
    ~CustomPlot();

   public:
    QCPGraph* addDataSource(DataSource* source);
    void removeDataSource(DataSource* source);
    QCPGraph* getGraph(DataSource* source);

    bool isDataSourceExist(DataSource* source) const;

   private:
    void initChart();
    void initAxis();
    void initSeries();

   private:
    QMap<DataSource*, QCPGraph*> sourceToGraphMap;
};


#endif /* __M_MYCUSTOMPLOT_H__ */
