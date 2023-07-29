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

DataSource::DataSource(QObject* parent) : uuid{}, QObject{parent} {
    uuid.append(QUuid::createUuid());

    updateTimer = new QTimer{this};
    updateTimer->setInterval(dataUpdateInterval);
    connect(updateTimer, &QTimer::timeout, this, &DataSource::updateData);
    updateTimer->start();

    dataX.append(QVector<double>{});
    dataY.append(QVector<double>{});

    connect(this, &DataSource::controlWordReceived, this,
            &DataSource::onControlWordReceived);
}
DataSource::~DataSource() {}

DataSource::DSID DataSource::getId(qsizetype index) {
    QMutexLocker locker{&uuidMutex};
    if (index >= uuid.size())
        return {};
    return uuid[index];
}

QVector<DataSource::DSID> DataSource::getIds() {
    QMutexLocker locker{&uuidMutex};
    return uuid;
}

QPair<DataSource::DataControlWords, QByteArray> DataSource::parseControlWord(
    QByteArray data) const {
    // remove last char
    data.chop(1);

    if (data.startsWith("%START")) {
        return {DataControlWords::DataStreamStart, {}};
    } else if (data.startsWith("%STOP")) {
        return {DataControlWords::DataStreamStop, {}};
    } else if (data.startsWith("%SUBPLOT")) {
        if (data.split(' ').size() < 2) {
            emit error("Subplot index set but not specified");
            return {DataControlWords::UserDefined, data};
        }

        return {DataControlWords::SlelectSubplot, data.split(' ')[1]};
    } else if (data.startsWith("%T")) {
        if (data.split(' ').size() < 2) {
            emit error("X axis step set but not specified");
            return {DataControlWords::UserDefined, data};
        }

        return {DataControlWords::SetXAxisStep, data.split(' ')[1]};
    } else if (data.startsWith("%SETRANGE")) {
        if (data.split(' ').size() < 2) {
            emit error("X axis range set but not specified");
            return {DataControlWords::UserDefined, data};
        }

        return {DataControlWords::SetXAxisRange, data.split(' ')[1]};
    } else if (data.startsWith("%CLEAR")) {
        return {DataControlWords::ClearDatas, {}};
    } else if (data.startsWith("%USELOGAXES")) {
        return {DataControlWords::SetUseLogAxis, {}};
    } else if (data.startsWith("%SETPLOTNAME")) {
        if (data.split(' ').size() < 2) {
            emit error("Plot name set but not specified");
            return {DataControlWords::UserDefined, data};
        }

        if (data.split(' ')[1].split(';').size() < 2) {
            emit error(
                "Plot name set but not specified correctly, may loss ; "
                "character");
            return {DataControlWords::UserDefined, data};
        }

        return {DataControlWords::SetPlotName, data.split(' ')[1]};
    } else if (data.startsWith("%SETPLOTUNIT")) {
        if (data.split(' ').size() < 2) {
            emit error("Plot unit set but not specified");
            return {DataControlWords::UserDefined, data};
        }

        return {DataControlWords::SetPlotUnit, data.split(' ')[1]};
    } else {
        return {DataControlWords::UserDefined, data};
    }
}

void DataSource::clearAllData() {
    for (auto& xArr : dataX) {
        xArr.clear();
    }
    for (auto& yArr : dataY) {
        yArr.clear();
    }
}

void DataSource::onControlWordReceived(qsizetype index, DataControlWords words,
                                       QByteArray data) {
    switch (words) {
        case DataControlWords::DataStreamStart: {
            clearAllData();
        } break;

        case DataControlWords::SlelectSubplot: {
            currentSelectedChannel = data.toLongLong();

            if (dataX.size() <= currentSelectedChannel) {
                dataX.append(QVector<double>{});
                dataY.append(QVector<double>{});

                uuid.append(QUuid::createUuid());
                emit newDataChannelCreated(currentSelectedChannel,
                                           uuid.constLast());
            }
        } break;

        default:
            break;
    }
}

void DataSource::appendData(QVector<double> x, QVector<double> y) {
    dataX[currentSelectedChannel].append(x);
    dataY[currentSelectedChannel].append(y);
}

void DataSource::clearQueuedData() {
    for (auto& xArr : dataX) {
        xArr.clear();
    }
    for (auto& yArr : dataY) {
        yArr.clear();
    }
}

void DataSource::updateData() {
    if (dataX.isEmpty())
        return;

    for (auto i = 0; i < dataX.size(); ++i) {
        if (dataX[i].isEmpty())
            continue;

        emit dataReceived(i, dataX[i], dataY[i]);

        dataX[i].clear();
        dataY[i].clear();
    }
}
