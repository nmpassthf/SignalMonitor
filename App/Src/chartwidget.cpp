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
#include <QHBoxLayout>
#include <ranges>

#include "ui_chartwidget.h"

ChartWidgetToolBar::ChartWidgetToolBar(QWidget* parent)
    : QWidget{parent}, ui{new Ui::ChartWidgetToolBar} {
    ui->setupUi(this);

    connect(ui->bToggleScopeMode, &QPushButton::clicked, this,
            [this](bool isEnabeld) {
                if (isEnabeld) {
                    ui->bToggleScopeMode->setText("Scope Mode");

                    // show slider bar
                    ui->sSlider->show();
                    ui->sliderLabel->show();
                } else {
                    ui->bToggleScopeMode->setText("Normal Mode");

                    // hide slider bar
                    ui->sSlider->hide();
                    ui->sliderLabel->hide();
                }
            });

    ui->bToggleScopeMode->click();
}
ChartWidgetToolBar::~ChartWidgetToolBar() { delete ui; }

ChartWidget::ChartWidget(QWidget* parent)
    : QWidget{parent}, toolBar{new ChartWidgetToolBar{this}} {
    initLayout();
    initToolBar();
}
ChartWidget::~ChartWidget() {}

QPair<CustomPlot*, QCPGraph*> ChartWidget::addPlot(DataSource* ds,
                                                   DataSource::DSID id,
                                                   PlotPos_t pos) {
    if (pos.second == -1) {
        if (subplots.isEmpty()) {
            pos.second = 0;
        } else {
            pos.second = getSubplotCount().second + 1;
        }
    }

    if (pos.first == -1) {
        // find the max row in this column
        auto it = std::ranges::max_element(
            std::ranges::find_if(
                subplots,
                [pos](auto& p) { return p.second.second == pos.second; }),
            subplots.cend(), {}, [](auto& p) { return p.second.first; });

        pos.first = it == subplots.cend() ? 0 : it->second.first + 1;
    }

    auto plot = new CustomPlot{this};
    auto series = plot->addDataSource(id);

    auto chartwidgetHWidgetCount = chartWidgetLayout->count();
    auto chartwidgetVLayout =
        pos.second >= chartwidgetHWidgetCount
            ? nullptr
            : dynamic_cast<QVBoxLayout*>(
                  chartWidgetLayout->itemAt(pos.second)->layout());

    if (chartwidgetVLayout == nullptr) {
        chartwidgetVLayout = new QVBoxLayout{};
        chartWidgetLayout->addLayout(chartwidgetVLayout);
    }

    if (chartwidgetVLayout->count() > pos.first) {
        chartwidgetVLayout->insertWidget(pos.first, plot);

        // change other pos in this row
        for (auto& [subplot, subplotPos] : subplots) {
            if (subplotPos.second == pos.second &&
                subplotPos.first >= pos.first) {
                ++subplotPos.first;
            }
        }
    } else {
        chartwidgetVLayout->addWidget(plot);
    }

    subplots.push_back({plot, pos});

    if (toolBar->isHidden()) {
        toolBar->show();
    }

    // connect with data source
    connect(ds, &DataSource::dataReceived, this,
            [series, ds, id](qsizetype index, QVector<double> x,
                             const QVector<double> y) {
                auto requestId = ds->getId(index);

                if (requestId != id) {
                    return;
                }

                series->addData(x, y, true);

                series->rescaleAxes();

                series->parentPlot()->replot();
            });
    connect(ds, &DataSource::controlWordReceived, this,
            [this, series, ds, id](qsizetype index,
                                   DataSource::DataControlWords controlWord,
                                   QByteArray DCWData = {}) {
                auto requestId = ds->getId(index);

                if (requestId != id) {
                    return;
                }

                switch (controlWord) {
                    case DataSource::DataControlWords::SetXAxisRange:
                        series->parentPlot()->xAxis->setRangeUpper(
                            DCWData.toDouble());
                        break;

                    case DataSource::DataControlWords::SetUseLogAxis:
                        series->parentPlot()->yAxis->setScaleType(
                            QCPAxis::stLogarithmic);
                        break;

                    case DataSource::DataControlWords::SetPlotName: {
                        auto names = DCWData.split(';');
                        if (names[0] != "{}")
                            series->parentPlot()->xAxis->setLabel(names[0]);
                        if (names[1] != "{}")
                            series->parentPlot()->yAxis->setLabel(names[1]);
                    } break;

                    case DataSource::DataControlWords::SetPlotUnit: {
                        auto units = DCWData.split(';');
                        qobject_cast<CustomPlot*>(series->parentPlot())
                            ->setPlotUnit(units[0], units[1]);
                    } break;

                    case DataSource::DataControlWords::ClearDatas:
                        series->setData({}, {});
                        break;
                }
            });

    return {plot, series};
}

QCPGraph* ChartWidget::insertAtPlot(DataSource::DSID id, PlotPos_t pos) {
    auto plot = getPlot(pos);

    if (plot == nullptr) {
        return nullptr;
    }

    if (toolBar->isHidden()) {
        toolBar->show();
    }

    return plot->addDataSource(id);
}

void ChartWidget::removePlot(PlotPos_t pos) {
    auto plot = getPlot(pos);

    if (plot == nullptr) {
        return;
    }

    plot->deleteLater();
    subplots.erase(std::ranges::find_if(
        subplots, [plot](auto& p) { return p.first == plot; }));

    if (subplots.isEmpty()) {
        toolBar->hide();
    }
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

CustomPlot* ChartWidget::getPlot(PlotPos_t pos) const {
    auto it = std::ranges::find_if(subplots,
                                   [pos](auto& p) { return p.second == pos; });

    if (it == subplots.end()) {
        return nullptr;
    }

    return it->first;
}

void ChartWidget::clearPlot(PlotPos_t pos) {
    auto plot = getPlot(pos);

    if (plot == nullptr) {
        return;
    }

    plot->graph()->data()->clear();

    // replot
    plot->replot();
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

QVector<CustomPlot*> ChartWidget::getPlots() const {
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
        // 最大行数为最大y坐标
        maxRow = std::max(maxRow, pos.first);
        // 最大列数为最大x坐标
        maxCol = std::max(maxCol, pos.second);
    }

    return {maxRow, maxCol};
}

ChartWidget::PlotPos_t ChartWidget::getAvailablePos() const {
    auto [row, col] = getSubplotCount();

    for (auto currRow : std::views::iota(0, row)) {
        for (auto currCol : std::views::iota(0, col)) {
            if (getPlot({currRow, currCol}) == nullptr) {
                return {currRow, currCol};
            }
        }
    }

    // 如果所有位置都被占用了，那么添加新的一列
    return {0, col + 1};
}

ChartWidget::PlotPos_t ChartWidget::getPlotPos(CustomPlot* plot) const {
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

void ChartWidget::initLayout() {
    chartWidgetLayout = new QHBoxLayout{};
    chartWidgetLayout->setSpacing(0);

    auto layout = new QVBoxLayout{this};
    layout->addWidget(toolBar);
    layout->addLayout(chartWidgetLayout);

    layout->setStretch(1, 1);

    setLayout(layout);
}

void ChartWidget::initToolBar() {
    // set toolbar hide when chartwidgetlayout is empty
    toolBar->hide();
}
