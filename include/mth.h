#pragma once

template<size_t N>
double calcStdDev(const std::array<double, N>& data) {
    double sum = 0.0;
    for (double d : data) sum += d;
    double mean = sum / N;

    double a = 0.0;
    for (double d : data) a += ((d - mean) * (d - mean));

    return std::sqrt(a / N);
}