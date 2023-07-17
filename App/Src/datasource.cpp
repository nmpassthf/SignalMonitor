#include "datasource.h"

DataSource::DataSource(QObject *parent) : QObject{parent} {
    updateTimer = new QTimer{this};
    updateTimer->setInterval(dataUpdateInterval);
    connect(updateTimer, &QTimer::timeout, this, &DataSource::updateData);
    updateTimer->start();
}
DataSource::~DataSource() {}

void DataSource::updateData() {
	static qreal x = 0;

    if (!dataQueue.isEmpty()) {
        // emit dataReceived(dataQueue);
		// TODO
		MySeries* series = new MySeries(100);
		for (auto& dataPoint : dataQueue) {
			series->append(x, dataPoint);
			x++;
		}
		emit dataReceivedSer(series);

        dataQueue.clear();
    }
}

void DataSource::appendData(qreal data) {
    dataQueue.append(data);
}

void DataSource::appendData(QVector<qreal> data) {
    dataQueue.append(data);
}
