#include "chartwidget.h"

#include <QDebug>
#include <QGridLayout>
#include <ranges>

ChartWidget::ChartWidget(QWidget* parent) : QWidget{parent} {
    chartWidgetLayout = new QGridLayout{this};
    chartWidgetLayout->setSpacing(0);
    // layout->setContentsMargins(0, 0, 0, 0);
    setLayout(chartWidgetLayout);
}
ChartWidget::~ChartWidget() {}

void ChartWidget::addPlot(DataSource* source, QPoint pos) {
    if (pos == QPoint{-1, -1}) {
        pos = {0, getSubplotCount().second};
    }

    auto plot = new CustomPlot{this};
    plot->addDataSource(source);
    subplots.push_back({plot, pos});
    chartWidgetLayout->addWidget(plot, pos.x(), pos.y());
}

QCPGraph* ChartWidget::insertAtPlot(DataSource* source, QPoint pos) {
    auto plot = getPlot(pos);

    if (plot == nullptr) {
        return nullptr;
    }

    return plot->addDataSource(source);
}

void ChartWidget::removePlot(QPoint pos) {
    auto plot = getPlot(pos);

    if (plot == nullptr) {
        return;
    }

    plot->deleteLater();
    subplots.erase(std::ranges::find_if(
        subplots, [plot](auto& p) { return p.first == plot; }));
}

void ChartWidget::removePlot(DataSource* source) {
    if (source == nullptr) {
        qDebug() << "ChartWidget::removePlot: source is nullptr";
        return;
    }

    if (isDataSourceExist(source) == false) {
        qDebug() << "ChartWidget::removePlot: source is not exist";
        return;
    }

    for (const auto& [plot, pos] : subplots) {
        if (plot->isDataSourceExist(source)) {
            removePlot(pos);
        }
    }
}

void ChartWidget::removeAllPlots() {
    for (const auto& plotPos : subplots | std::views::values) {
        removePlot(plotPos);
    }
    subplots.clear();
}

CustomPlot* ChartWidget::getPlot(QPoint pos) const {
    auto it = std::ranges::find_if(subplots,
                                   [pos](auto& p) { return p.second == pos; });

    if (it == subplots.end()) {
        return nullptr;
    }

    return it->first;
}

QVector<CustomPlot*> ChartWidget::getPlots() {
    QVector<CustomPlot*> plots;
    plots.reserve(subplots.size());

    for (auto plot : subplots | std::views::keys) {
        plots.push_back(plot);
    }

    return plots;
}

QPair<int, int> ChartWidget::getSubplotCount() const {
    int maxRow = 0;
    int maxCol = 0;

    for (const auto& pos : subplots | std::views::values) {
        maxRow = std::max(maxRow, pos.x());
        maxCol = std::max(maxCol, pos.y());
    }

    return {maxRow + 1, maxCol + 1};
}

QPoint ChartWidget::getAvailablePos() const {
    auto [row, col] = getSubplotCount();

    for (auto currRow : std::views::iota(0, row)) {
        for (auto currCol : std::views::iota(0, col)) {
            if (getPlot({currRow, currCol}) == nullptr) {
                return {currRow, currCol};
            }
        }
    }

    // 如果所有位置都被占用了，那么添加新的一列
    return {0, col};
}

bool ChartWidget::isDataSourceExist(DataSource* source) const {
    if (source == nullptr) {
        qDebug() << "ChartWidget::isDataSourceExist: source is nullptr";
        return false;
    }

    return std::ranges::find_if(subplots, [source](auto& p) {
               return p.first->isDataSourceExist(source);
           }) != subplots.end();
}
