#include "chartwidget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QValueAxis>
#include <ranges>

ChartWidget::ChartWidget(QWidget* parent) : QCustomPlot{parent} {}
ChartWidget::~ChartWidget() {}

void ChartWidget::updateData(QVector<qreal>) {}

void ChartWidget::initChart() {}
void ChartWidget::initAxis() {}
void ChartWidget::initSeries() {}
