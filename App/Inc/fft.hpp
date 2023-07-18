#ifndef __M_FFT_HPP__
#define __M_FFT_HPP__
#ifndef __BBR_FFT_H__
#define __BBR_FFT_H__

#include <complex>
#include <string>
#include <vector>

using ComplexDouble = std::complex<double>;
using ComplexArray = std::vector<ComplexDouble>;


class Fourier {
   private:
   public:
    Fourier() = default;
    ~Fourier() = default;

    static ComplexArray fft(const ComplexArray &inputData);
    static ComplexArray ifft(const ComplexArray &inputData);

    static std::string pretty(const ComplexArray &);
    static std::string prettyComplexDouble(const ComplexDouble &);
    inline static bool floatIsZero(const float& f) {return f <= FLT_EPSILON && f >= -FLT_EPSILON;}
    inline static bool floatIsZero(const double& f) {return f <= DBL_EPSILON && f >= -DBL_EPSILON;}
};

#endif  // !__BBR_FFT_H__


#endif /* __M_FFT_HPP__ */
