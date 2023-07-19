#include "mycustomplot.h"

CustomPlot::CustomPlot(QWidget* parent) : QCustomPlot{parent} {
    initChart();
    initAxis();
    initSeries();

    setOpenGl(true);
}
CustomPlot::~CustomPlot() {}

void CustomPlot::connectDataSource(DataSource* source) {
    if (source == nullptr) {
        printCurrentTime()
            << "CustomPlot::connectDataSource: source is nullptr";
        return;
    }

    if (isDataSourceExist(source->getId()) == false) {
        printCurrentTime() << "CustomPlot::connectDataSource: id is not added";
        return;
    }

    connect(source, &DataSource::dataReceived, this,
            [this, source](QVector<double> x, QVector<double> y) {
                auto graph = getGraph(source->getId());
                if (graph == nullptr) {
                    printCurrentTime()
                        << "CustomPlot::connectDataSource: graph is nullptr";
                    return;
                }

                graph->addData(x, y);
                replot();
            });
}

QCPGraph* CustomPlot::addDataSource(DataSource::DSID id) {
    if (id == DataSource::DSID{}) {
        printCurrentTime() << "CustomPlot::addDataSource: id is empty";
        return nullptr;
    }

    if (isDataSourceExist(id)) {
        printCurrentTime() << "CustomPlot::addDataSource: id is already exist";
        return nullptr;
    }

    auto graph = addGraph();

    sourceToGraphMap.insert(id, graph);

    return graph;
}

void CustomPlot::removeDataSource(DataSource::DSID id) {
    if (id == DataSource::DSID{}) {
        printCurrentTime() << "CustomPlot::removeDataSource: id is empty";
        return;
    }

    if (isDataSourceExist(id) == false) {
        printCurrentTime() << "CustomPlot::removeDataSource: id is not exist";
        return;
    }

    auto graph = getGraph(id);
    removeGraph(graph);
    sourceToGraphMap.remove(id);
}

QCPGraph* CustomPlot::getGraph(DataSource::DSID id) {
    auto it = sourceToGraphMap.find(id);

    if (it == sourceToGraphMap.end()) {
        return nullptr;
    }

    return it.value();
}

bool CustomPlot::isDataSourceExist(DataSource::DSID id) const {
    return sourceToGraphMap.contains(id);
}

void CustomPlot::initChart() {
    setBackground(QBrush{QColor{Qt::GlobalColor::white}});
    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                    QCP::iSelectLegend | QCP::iSelectPlottables);

    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems);
    setAntialiasedElement(QCP::aeAll);
    axisRect()->setRangeZoom(Qt::Horizontal);
    connect(this, &QCustomPlot::beforeReplot, this, [this]() {
        // zoom y to Fix screen.
        for (int i = 0; i != graphCount(); ++i) {
            graph(i)->rescaleValueAxis(false, true);
        }
    });
}

void CustomPlot::initAxis() {
    xAxis->setLabel("x: time (us)");
    yAxis->setLabel("y");
}

void CustomPlot::initSeries() {
    // do nothing
}