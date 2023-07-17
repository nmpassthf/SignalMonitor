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
}
ChartWidget::~ChartWidget() {}

const QVector<MySeries*> ChartWidget::getSeries() { return series; }
void ChartWidget::addSeries(MySeries* series) {
    series->setParent(this);

    this->series.push_back(series);
    chart->addSeries(series);

    chart->createDefaultAxes();
}
qreal ChartWidget::getStep(qsizetype index) { return series[index]->getStep(); }

void ChartWidget::addData(qreal y, qsizetype index) {
    auto currentSeries = series[index];

    currentSeries->pushAStep(y);

    chart->axes(Qt::Horizontal).first()->setRange(0, currentSeries->getMaxX());
    auto [min, max] = currentSeries->getMinMaxY();
    chart->axes(Qt::Vertical).first()->setRange(min, max);
}
void ChartWidget::addData(QVector<qreal> y, qsizetype index) {
    auto currentSeries = series[index];

    // for (auto& dataPoint : y) {
    //     currentSeries->pushAStep(dataPoint);
    // }
    static qreal x = 0;
    QVector<QPointF> points;
    for (auto& dataPoint : y) {
        points.append({x, dataPoint});
        x++;
    }
    currentSeries->pushAStep(points);
    // TODO 优化 一次性添加多个点 将x点更新位置改为源头处更新


    chart->axes(Qt::Horizontal).first()->setRange(0, currentSeries->getMaxX());
    auto [min, max] = currentSeries->getMinMaxY();
    chart->axes(Qt::Vertical).first()->setRange(min, max);
}

void ChartWidget::addData(MySeries* s, qsizetype index) {
    if(index != -1){auto oldSeries = series[index];
    series[index] = s;
    oldSeries->deleteLater();
    chart->removeSeries(oldSeries);
}
    chart->addSeries(s);

    



    chart->axes(Qt::Horizontal).first()->setRange(0, s->getMaxX());
    auto [min, max] = s->getMinMaxY();
    chart->axes(Qt::Vertical).first()->setRange(min, max);
}

void ChartWidget::setStep(qreal step, qsizetype index) {
    series[index]->setStep(step);
}
