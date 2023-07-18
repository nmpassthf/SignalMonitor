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
            [this](QByteArray ctrl) {
                if (ctrl.contains("%t")) {
                    this->step = ctrl.mid(2).toDouble();
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
        uint64_t i = 0;
        for (auto const& cDouble : fftResult) {
            x.append(1e6 * (i++) / step);
            y.append(std::pow(
                std::pow(cDouble.real(), 2) + std::pow(cDouble.imag(), 2),
                0.5));
        }

        appendData(x, y);
        QApplication::processEvents();
    }

    emit finished();
}

void FFTDataSource::setFFTSize(uint32_t size) { fftSize = size; }
