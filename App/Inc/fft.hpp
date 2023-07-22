/**
 * @file fft.hpp
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2022-12-13
 *
 * @copyright Copyright (c) nmpassthf 2022
 *
 */
#ifndef __M_FFT_HPP__
#define __M_FFT_HPP__

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
};

#endif /* __M_FFT_HPP__ */
