#ifndef __M_FFTDATASOURCE_H__
#define __M_FFTDATASOURCE_H__

#include "datasource.h"
#include "fft.hpp"

class FFTDataSource : public DataSource {
    Q_OBJECT;

    constexpr static auto defalultFFTSize = 1024;

   public:
    explicit FFTDataSource(DataSource const* otherRegularSource,
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
};

#endif /* __M_FFTDATASOURCE_H__ */
