#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>
#include "bmp_parser.h"

// Function applies discrete convolution on the input BMP image.
// Parameters:
// - input: input BMP image
// - kernel: convolution kernel (vector of double values, size kernelWidth * kernelHeight)
// - kernelWidth: width of the kernel
// - kernelHeight: height of the kernel
//
// Function returns a new BMP image with the convolution applied.
// If the kernel size is invalid, it throws std::runtime_error.
BMPImage applyConvolution(const BMPImage& input, const std::vector<double>& kernel, int kernelWidth, int kernelHeight);

// Function applies parallel discrete convolution on the input BMP image.
// Parameters:
// - input: input BMP image
// - kernel: convolution kernel (vector of double values, size kernelWidth * kernelHeight)
// - kernelWidth: width of the kernel
// - kernelHeight: height of the kernel
//
// Function returns a new BMP image with the convolution applied.
BMPImage applyParallelConvolution(const BMPImage& input, const std::vector<double>& kernel, int kernelWidth, int kernelHeight);

// Function applies distributed parallel discrete convolution on the input BMP image.
// Parameters:
// - input: input BMP image
// - kernel: convolution kernel (vector of double values, size kernelWidth * kernelHeight)
// - kernelWidth: width of the kernel
// - kernelHeight: height of the kernel
//
// Function returns a new BMP image with the convolution applied.
BMPImage applyDistributedParallelConvolution(const BMPImage& input, const std::vector<double>& kernel, int kernelWidth, int kernelHeight);