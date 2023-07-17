#include "datasource.h"

DataSource::DataSource(QObject* parent) : QObject{parent} {
    updateTimer = new QTimer{this};
    updateTimer->setInterval(dataUpdateInterval);
    connect(updateTimer, &QTimer::timeout, this, &DataSource::updateData);
    updateTimer->start();
}
DataSource::~DataSource() {}

void DataSource::updateData() {
    if (dataX.isEmpty())
        return;

    emit dataReceived(dataX, dataY);

    dataX.clear();
    dataY.clear();
}

void DataSource::appendData(QVector<double> x, QVector<double> y) {
    dataX.append(x);
    dataY.append(y);
}
