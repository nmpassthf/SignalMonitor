#include "chartwidget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QValueAxis>
#include <ranges>

MyChartView::MyChartView(QChart* chart, QWidget* parent)
    : QChartView(chart, parent) {
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    setRubberBand(QChartView::HorizontalRubberBand);
}

void MyChartView::mouseReleaseEvent(QMouseEvent* event) {
    if (chart()->axes().isEmpty() || chart()->series().isEmpty()) {
        QChartView::mouseReleaseEvent(event);
        return;
    }

    // TODO 可选锁定Y轴绑定第一个序列
    auto dynamicZoomY = [&](bool yLockedOnFirstSeries = false) {
        QList<QValueAxis*> yAxes;
        for (auto axis : chart()->axes().mid(1)) {
            yAxes.append(qobject_cast<QValueAxis*>(axis));
        }

        if (yLockedOnFirstSeries) {
            auto [minY, maxY] =
                qobject_cast<MySeries*>(chart()->series().first())
                    ->getMinMaxY();
            for (auto yAxis : yAxes) {
                yAxis->setRange(minY, maxY);
            }

            return;
        }

        // get chart max val in current range.
        for (auto s : chart()->series()) {
            MySeries* series = qobject_cast<MySeries*>(s);

            for (auto yAxis : yAxes) {
                if (series->attachedAxes().contains(yAxis)) {
                    auto [minY, maxY] = series->getMinMaxY();
                    yAxis->setRange(minY, maxY);
                }
            }
        }
    };

    if (event->button() != Qt::RightButton) {
        QChartView::mouseReleaseEvent(event);

        if (event->button() == Qt::LeftButton) {
            dynamicZoomY();
        }
        return;
    } else {
        auto series = chart()->series();

        auto maxX = qobject_cast<MySeries*>(series.first())->getMaxX();
        for (auto s : series) {
            maxX = std::max(maxX, qobject_cast<MySeries*>(s)->getMaxX());
        }

        // 保证缩放时坐标轴原点大于0
        QValueAxis* xAxis = static_cast<QValueAxis*>(chart()->axes()[0]);
        auto vw = xAxis->max() - xAxis->min();
        xAxis->setRange(qMax(xAxis->min() - vw / 2, (qreal)0),
                        qMin(xAxis->max() + vw / 2, maxX));

        dynamicZoomY();

        QGraphicsView::mouseReleaseEvent(event);
    }
}

MySeries::MySeries(qreal step, QObject* parent)
    : QLineSeries(parent), step(step) {
    useOpenGL();
}
MySeries::~MySeries() {}

ChartWidget::ChartWidget(QString title, QWidget* parent) : QWidget(parent) {
    if (parent == nullptr)
        setWindowTitle(title);

    auto currentLayout = new QHBoxLayout{};
    setLayout(currentLayout);

    chart = new QChart{};
    chartView = new MyChartView{chart, this};

    currentLayout->addWidget(new QLabel{title, this});
    currentLayout->addWidget(chartView);

    threadPool = new QThreadPool{this};
    threadPool->setMaxThreadCount(1);
}
ChartWidget::~ChartWidget() {}

const QVector<MySeries*> ChartWidget::getSeries() { return series; }
void ChartWidget::addSeries(MySeries* series) {
    series->setParent(this);

    this->series.push_back(series);
    chart->addSeries(series);

    chart->createDefaultAxes();
}
bool ChartWidget::removeSeries(MySeries* series) {
    if (!this->series.contains(series))
        return false;

    this->series.removeOne(series);
    chart->removeSeries(series);

    return true;
}
qreal ChartWidget::getStep(qsizetype index) { return series[index]->getStep(); }

void ChartWidget::addData(QVector<qreal> y, qsizetype index) {
    auto currentSeries = series[index];

    // TODO 工作线程中返回旧折线图对象和新折线图对象，旧折线图对象在主线程中删除

    if (pendingData.contains(currentSeries)) {
        pendingData[currentSeries].append(y);
    } else {
        pendingData.insert(currentSeries, y);
    }

    updatePendingData();
}

void ChartWidget::setStep(qreal step, qsizetype index) {
    series[index]->setStep(step);
}

void ChartWidget::closeEvent(QCloseEvent* event) {
    // if (thread != nullptr) {
    //     thread->quit();
    //     thread->wait();
    //     delete thread;
    //     thread = nullptr;
    // }

    // for (auto worker : workers) {
    //     delete worker;
    // }

    // workers.clear();

    // emit closed(this);;

    // TODO
}

void ChartWidget::onAddDataThreadDone(MySeries* newSeries,
                                      MySeries const* oldSeries) {
    auto allSeries = chart->series();

    removeSeries(const_cast<MySeries*>(oldSeries));

    if (pendingData.contains(const_cast<MySeries*>(oldSeries))) {
        auto y = pendingData[const_cast<MySeries*>(oldSeries)];
        pendingData.remove(const_cast<MySeries*>(oldSeries));

        pendingData.insert(newSeries, y);
    }

    addSeries(newSeries);
    chart->createDefaultAxes();

    chart->axes(Qt::Horizontal).first()->setRange(0, newSeries->getMaxX());
    auto [min, max] = newSeries->getMinMaxY();
    chart->axes(Qt::Vertical).first()->setRange(min, max);

    isThreadDone = true;

    updatePendingData();
}

void ChartWidget::updatePendingData() {
    if (threadPool->activeThreadCount() != 0 || !isThreadDone)
        return;

    if (pendingData.isEmpty())
        return;

    auto currentSeries = pendingData.firstKey();
    auto y = pendingData[currentSeries];
    pendingData.remove(currentSeries);

    auto newSeries = new MySeries(currentSeries->getStep());

    for(const auto& p : currentSeries->points()) {
        newSeries->append(p);
    }

    SeriesInsertDataWorker* worker = new SeriesInsertDataWorker{
        newSeries, currentSeries, currentSeries->points(), y};
    connect(worker, &SeriesInsertDataWorker::done, this,
            &ChartWidget::onAddDataThreadDone);

    threadPool->start(worker);
    isThreadDone = false;
}
