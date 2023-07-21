/**
 * @file datasource.cpp
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-20
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#include "datasource.h"

DataSource::DataSource(QObject* parent)
    : uuid(QUuid::createUuid()), QObject{parent} {
    updateTimer = new QTimer{this};
    updateTimer->setInterval(dataUpdateInterval);
    connect(updateTimer, &QTimer::timeout, this, &DataSource::updateData);
    updateTimer->start();
}
DataSource::~DataSource() {}

void DataSource::appendData(QVector<double> x, QVector<double> y) {
    dataX.append(x);
    dataY.append(y);
}

void DataSource::clearQueuedData() {
    dataX.clear();
    dataY.clear();
}

QPair<DataSource::DataControlWords, QByteArray> DataSource::parseControlWord(
    QByteArray data) const {
    if (data.startsWith("%START")) {
        return {DataControlWords::DataStreamStart, {}};
    } else if (data.startsWith("%STOP")) {
        return {DataControlWords::DataStreamStop, {}};
    } else if (data.startsWith("%CLEAR")) {
        return {DataControlWords::ClearDatas, {}};
    } else if (data.startsWith("%t")) {
        return {DataControlWords::SetXAxisStep, data.mid(2)};
    } else [[unlikely]] {
        return {DataControlWords::UserDefined, data};
    }
}

void DataSource::updateData() {
    if (dataX.isEmpty())
        return;

    emit dataReceived(dataX, dataY);

    dataX.clear();
    dataY.clear();
}
