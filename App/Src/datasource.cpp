#include "datasource.h"

DataSource::DataSource(QObject* parent) : QObject{parent} {
    updateTimer = new QTimer{this};
    updateTimer->setInterval(dataUpdateInterval);
    connect(updateTimer, &QTimer::timeout, this, &DataSource::updateData);
    updateTimer->start();
}
DataSource::~DataSource() {}

void DataSource::updateData() {
    if (dataQueue.isEmpty())
        return;


    emit dataReceived(dataQueue);

    dataQueue.clear();
}

void DataSource::appendData(QVector<qreal> data) { dataQueue.append(data); }
