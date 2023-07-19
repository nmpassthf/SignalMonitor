#include "fftdatasource.h"

#include <QApplication>
#include <QThread>
#include <ranges>

FFTDataSource::FFTDataSource(DataSource const* otherRegularSource,
                             QObject* parent)
    : DataSource{parent} {
    connect(otherRegularSource, &DataSource::dataReceived, this,
            [this](QVector<double> xs, QVector<double> ys) {
                for (auto& y : ys) {
                    if (dataset.size() < fftSize)
                        dataset.push_back({y, 0});
                    else {
                        dataset.erase(dataset.begin());
                        dataset.push_back({y, 0});
                    }
                    isDataUpdated = true;
                }
            });

    connect(otherRegularSource, &DataSource::controlWordReceived, this,
            [this](DataSource::DataControlWords c, QByteArray DCWData) {
                if (c == DataSource::DataControlWords::SetXAxisStep) {
                    this->step = DCWData.toDouble();
                }
            });
}

FFTDataSource::~FFTDataSource() {}

void FFTDataSource::run() {
    while (!isTerminateSerial) {
        QApplication::processEvents();

        if (!isDataUpdated) {
            continue;
        }

        if (dataset.size() < fftSize) {
            continue;
        }

        isDataUpdated = false;

        auto fftResult = Fourier::fft(dataset);
        QVector<double> x, y;
        x.reserve(fftResult.size());
        y.reserve(fftResult.size());

        uint64_t i;

        auto xVal = [&i, this](auto it) {
            return 1e6 * (i++) / step / fftSize;
        };
        auto yVal = [this](auto it) {
            return std::pow(std::pow(it->real(), 2) + std::pow(it->imag(), 2),
                            0.5) /
                   (fftSize / 2);
        };

        i = 0;
        for (auto pIt = fftResult.cbegin();
             pIt != fftResult.cend() - fftSize / 2; ++pIt) {
            x.append(xVal(pIt));
            y.append(yVal(pIt));
        }

        y[0] /= 2;

        appendData(x, y);
        emit controlWordReceived(DataControlWords::ClearDatas);
        clearQueuedData();
        QApplication::processEvents();
    }

    emit finished();
}

void FFTDataSource::setFFTSize(uint32_t size) { fftSize = size; }
