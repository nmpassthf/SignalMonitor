/**
 * @file fft.cpp
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2022-12-13
 *
 * @copyright Copyright (c) nmpassthf 2022
 *
 */
#include <pch.h>
#include "fft.hpp"

constexpr double __BBR_FFT_PI = 3.14159265358979323846;

ComplexArray Fourier::fft(const ComplexArray& inputData) {
    if (inputData.empty())
        return {};

    auto n{inputData.size()};

    // Handle the special case of a vector of size 1 (there's nothing to
    // transform)
    if (n == 1)
        return inputData;

    // Divide the input vector into two halves
    ComplexArray vec1(n / 2), vec2(n / 2);
    for (int i = 0; i < n / 2; ++i) {
        vec1[i] = inputData[i * 2];
        vec2[i] = inputData[i * 2 + 1];
    }

    // Recursively compute the FFT of each half
    vec1 = fft(vec1);
    vec2 = fft(vec2);

    // Compute the FFT of the whole vector using the FFTs of the two halves
    ComplexArray result(n);
    for (int i = 0; i < n; ++i) {
        ComplexDouble omega = exp(ComplexDouble(0, -2 * __BBR_FFT_PI * i / n));
        result[i] = vec1[i % (n / 2)] + omega * vec2[i % (n / 2)];
    }

    return result;
}

ComplexArray Fourier::ifft(const ComplexArray& inputData) {
    if (inputData.empty())
        return {};

    auto n{inputData.size()};

    // Handle the special case of a vector of size 1 (there's nothing to
    // transform)
    if (n == 1)
        return inputData;

    // Divide the input vector into two halves
    ComplexArray vec1(n / 2), vec2(n / 2);
    for (int i = 0; i < n / 2; ++i) {
        vec1[i] = inputData[i * 2];
        vec2[i] = inputData[i * 2 + 1];
    }

    // Recursively compute the IFFT of each half
    vec1 = ifft(vec1);
    vec2 = ifft(vec2);

    // Compute the IFFT of the whole vector using the IFFTs of the two halves
    ComplexArray result(n);
    for (int i = 0; i < n; ++i) {
        ComplexDouble omega = exp(ComplexDouble(0, 2 * __BBR_FFT_PI * i / n));
        result[i] = vec1[i % (n / 2)] + omega * vec2[i % (n / 2)];
    }

    return result;
}

std::string Fourier::pretty(const ComplexArray& inputData) {
    std::string result{"[ "};

    for (const auto& i : inputData) {
        result.append(prettyComplexDouble(i) + " ");
    }

    result.pop_back();

    result.append(" ]");

    return result;
}

std::string Fourier::prettyComplexDouble(const ComplexDouble& cDouble) {
    auto r = floatIsZero(cDouble.real());
    auto i = floatIsZero(cDouble.imag());

    auto real =
        std::to_string(floatIsZero(cDouble.real()) ? 0 : cDouble.real());
    real = real.erase(real.find_last_not_of('0'));
    if (real.ends_with('.'))
        real.pop_back();
    auto imag =
        std::to_string(floatIsZero(cDouble.imag()) ? 0 : cDouble.imag());
    imag = imag.erase(imag.find_last_not_of('0'));
    if (imag.ends_with('.'))
        imag.pop_back();

    if (real == "-0")
        real = "0";
    if (imag == "-0")
        imag = "0";

    if (real == "0" && imag == "0")
        return "0";
    if (real == "0")
        return imag + "j";
    if (imag == "0")
        return real;

    return real + (imag[0] == '-' ? "" : "+") + imag + "j";
}
