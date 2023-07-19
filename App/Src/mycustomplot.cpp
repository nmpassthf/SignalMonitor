#include "mycustomplot.h"

CustomPlot::CustomPlot(QWidget* parent) : QCustomPlot{parent} {
    initChart();
    initAxis();
    initSeries();
}
CustomPlot::~CustomPlot() {}

QCPGraph* CustomPlot::addDataSource(DataSource* source) {
    if (source == nullptr) {
        qDebug() << "CustomPlot::addDataSource: source is nullptr";
        return nullptr;
    }

    if (isDataSourceExist(source)) {
        qDebug() << "CustomPlot::addDataSource: source is already exist";
        return nullptr;
    }

    auto graph = addGraph();

    sourceToGraphMap.insert(source, graph);

    return graph;
}

void CustomPlot::removeDataSource(DataSource* source) {
    if (source == nullptr) {
        qDebug() << "CustomPlot::removeDataSource: source is nullptr";
        return;
    }

    if (isDataSourceExist(source) == false) {
        qDebug() << "CustomPlot::removeDataSource: source is not exist";
        return;
    }

    auto graph = getGraph(source);
    removeGraph(graph);
    sourceToGraphMap.remove(source);
}

QCPGraph* CustomPlot::getGraph(DataSource* source) {
    auto it = sourceToGraphMap.find(source);

    if (it == sourceToGraphMap.end()) {
        return nullptr;
    }

    return it.value();
}

bool CustomPlot::isDataSourceExist(DataSource* source) const {
    return sourceToGraphMap.contains(source);
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