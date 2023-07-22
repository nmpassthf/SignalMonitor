/**
 * @file chartwidget.cpp
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-20
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
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

QPair<CustomPlot*, QCPGraph*> ChartWidget::addPlot(DataSource::DSID id,
                                                   QPoint pos) {
    if (pos == QPoint{-1, -1}) {
        pos = {0, getSubplotCount().second};
    }

    auto plot = new CustomPlot{this};
    auto series = plot->addDataSource(id);
    subplots.push_back({plot, pos});
    chartWidgetLayout->addWidget(plot, pos.y(), pos.x());

    return {plot, series};
}

QCPGraph* ChartWidget::insertAtPlot(DataSource::DSID id, QPoint pos) {
    auto plot = getPlot(pos);

    if (plot == nullptr) {
        return nullptr;
    }

    return plot->addDataSource(id);
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

void ChartWidget::removePlot(DataSource::DSID id) {
    if (id == DataSource::DSID{}) {
        printCurrentTime() << "ChartWidget::removePlot: id is empty";
        return;
    }

    if (isDataSourceExist(id) == false) {
        printCurrentTime() << "ChartWidget::removePlot: id is not exist";
        return;
    }

    for (const auto& [plot, pos] : subplots) {
        if (plot->isDataSourceExist(id)) {
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

void ChartWidget::clearPlot(QPoint pos) {
    auto plot = getPlot(pos);

    if (plot == nullptr) {
        return;
    }

    plot->graph()->data()->clear();
}
void ChartWidget::clearPlot(DataSource::DSID id) {
    if (id == DataSource::DSID{}) {
        printCurrentTime() << "ChartWidget::clearPlot: id is empty";
        return;
    }

    if (isDataSourceExist(id) == false) {
        printCurrentTime() << "ChartWidget::clearPlot: id is not exist";
        return;
    }

    for (const auto& [plot, pos] : subplots) {
        if (plot->isDataSourceExist(id)) {
            clearPlot(pos);
        }
    }
}
void ChartWidget::clearPlots() {
    for (const auto& plotPos : subplots | std::views::values) {
        clearPlot(plotPos);
    }
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
        // 最大行数为最大y坐标+1
        maxRow = std::max(maxRow, pos.y());
        // 最大列数为最大x坐标+1
        maxCol = std::max(maxCol, pos.x());
    }

    return {maxRow + 1, maxCol + 1};
}
// TODO 避免行列和xy混用
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

QPoint ChartWidget::getPlotPos(CustomPlot* plot) const {
    auto it = std::ranges::find_if(subplots,
                                   [plot](auto& p) { return p.first == plot; });

    if (it == subplots.end()) {
        return {-1, -1};
    }

    return it->second;
}

bool ChartWidget::isDataSourceExist(DataSource::DSID id) const {
    if (id == DataSource::DSID{}) {
        printCurrentTime() << "ChartWidget::isDataSourceExist: id is empty";
        return false;
    }

    return std::ranges::find_if(subplots, [id](auto& p) {
               return p.first->isDataSourceExist(id);
           }) != subplots.end();
}
