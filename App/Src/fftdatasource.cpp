/**
 * @file fftdatasource.cpp
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-22
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#include "fftdatasource.h"

#include <QApplication>
#include <QThread>
#include <ranges>

FFTDataSource::FFTDataSource(FFTWorkMode mode,
                             DataSource const* otherRegularSource,
                             QObject* parent)
    : workMode{mode}, DataSource{parent} {
    dataset.reserve(fftSize);

    connect(otherRegularSource, &DataSource::dataReceived, this,
            [this](qsizetype index, QVector<double> xs, QVector<double> ys) {
                if (index != currentSelectedChannel)
                    return;

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
            [this](qsizetype index, DataSource::DataControlWords c,
                   QByteArray DCWData) {
                if (index != currentSelectedChannel)
                    return;

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

        // copy a new dataset to use zero padding
        ComplexArray dataset = this->dataset;
        dataset.reserve(fftSize);
        for (auto i = dataset.size(); i < fftSize; ++i) {
            dataset.push_back({0, 0});
        }

        isDataUpdated = false;

        auto fftResult = Fourier::fft(dataset);
        QVector<double> x, y;
        x.reserve(fftResult.size());
        y.reserve(fftResult.size());

        uint64_t i;

        auto xVal = std::function<double(ComplexArray::const_iterator)>();
        auto yVal = std::function<double(ComplexArray::const_iterator)>();

        if (workMode == Amplitude) {
            xVal = [&i, this](auto it) { return 1e6 * (i++) / step / fftSize; };
            yVal = [this](auto it) {
                return std::pow(
                           std::pow(it->real(), 2) + std::pow(it->imag(), 2),
                           0.5) /
                       (fftSize / 2);
            };
        } else {
            xVal = [&i, this](auto it) { return 1e6 * (i++) / step / fftSize; };
            yVal = [this](auto it) {
                return std::atan2(it->imag(), it->real()) * 180 / M_PI;
            };
        }

        i = 0;
        for (auto pIt = fftResult.cbegin();
             pIt != fftResult.cend() - fftSize / 2; ++pIt) {
            x.append(xVal(pIt));
            y.append(yVal(pIt));
        }

        y[0] /= 2;

        emit controlWordReceived(currentSelectedChannel,
                                 DataControlWords::ClearDatas);
        clearQueuedData();
        appendData(x, y);
    }

    emit finished();
}

void FFTDataSource::clearAllData() {
    dataset.clear();
    isDataUpdated = false;
    DataSource::clearAllData();
}


void FFTDataSource::setFFTSize(uint32_t size) { fftSize = size; }
