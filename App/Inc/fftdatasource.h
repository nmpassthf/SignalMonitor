/**
 * @file fftdatasource.h
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-22
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#ifndef __M_FFTDATASOURCE_H__
#define __M_FFTDATASOURCE_H__

#include "datasource.h"
#include "fft.hpp"

class FFTDataSource : public DataSource {
    Q_OBJECT;

    constexpr static auto defalultFFTSize = 1024;

   public:
    using FFTWorkMode = enum {
        Amplitude,
        Phase,
    };

    explicit FFTDataSource(FFTWorkMode, DataSource const* otherRegularSource,
                           QObject* parent = nullptr);
    virtual ~FFTDataSource();

   public slots:
    virtual void run() override;
    void setFFTSize(uint32_t size);

   private:
    qreal step = 0;
    uint32_t fftSize = defalultFFTSize;
    ComplexArray dataset;
    bool isDataUpdated = false;
    FFTWorkMode workMode;
};

#endif /* __M_FFTDATASOURCE_H__ */
