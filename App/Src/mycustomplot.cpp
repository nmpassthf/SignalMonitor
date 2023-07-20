
#include "mycustomplot.h"

#include <qlabel.h>

#include <QMouseEvent>
#include <QTimer>

#include "globalSettings.h"

DataLabel::DataLabel(QWidget* parent) : QLabel{parent} {
    setAlignment(Qt::AlignCenter);

    // 禁用鼠标事件
    setAttribute(Qt::WA_TransparentForMouseEvents);
}
DataLabel::~DataLabel() {}

void DataLabel::setData(QPair<double, double> data) {
    setText(QString::number(data.first) + "," + QString::number(data.second));

    // 设置宽度足够显示
    adjustSize();
}

CustomPlot::CustomPlot(QWidget* parent) : QCustomPlot{parent} {
    initChart();
    initAxis({});
    initSeries();

    if (g_isUsingOpenGL) {
        setOpenGl(true);

        // 定期replot避免OpenGl导致显示错误
        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this]() { replot(); });
        timer->start(1000 / 60);
    }

    dataLabel = new DataLabel{this};
    dataLabel->hide();
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

    graph->setSelectable(QCP::stNone);

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

    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    setAntialiasedElement(QCP::aeAll);
    axisRect()->setRangeZoom(Qt::Horizontal);

    connect(this, &QCustomPlot::beforeReplot, this, [this]() {
        // zoom y to Fix screen.
        for (int i = 0; i != graphCount(); ++i) {
            graph(i)->rescaleValueAxis(false, true);
        }
    });

    connect(this, &QCustomPlot::mouseMove, this, [this](QMouseEvent* event) {
        QVariant variant;
        graph()->selectTest(event->pos(), false, &variant);
        auto res = static_cast<QCPDataSelection*>(variant.data());

        // show data label move with mouse
        if (res != nullptr && res->dataPointCount() > 0) {
            auto dataIndex = res->dataRange().begin();
            auto dataValue = *graph()->data()->at(dataIndex);
            auto dataPos = graph()->dataPixelPosition(dataIndex);

            dataLabel->setData({dataValue.key, dataValue.value});

            auto xAxisRect = plotLayout()->elements(false).first()->rect();
            auto currentY = dataPos.y();
            if (currentY + dataLabel->height() >
                xAxisRect.y() + xAxisRect.height())
                currentY =
                    xAxisRect.y() + xAxisRect.height() - dataLabel->height();
            dataLabel->move(dataPos.x() + 5, currentY);

            dataLabel->raise();

            if (dataLabel->isHidden())
                dataLabel->show();
        }
    });
}

void CustomPlot::initAxis(QStringList axesLabel) {
    if (axesLabel.isEmpty())
        axesLabel = {"x", "y"};
    xAxis->setLabel(axesLabel[0]);
    yAxis->setLabel(axesLabel[1]);

    xAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
    yAxis->setUpperEnding(QCPLineEnding::esSpikeArrow);
}

void CustomPlot::initSeries() {
    // do nothing
}