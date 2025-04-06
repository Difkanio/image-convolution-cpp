#include "discrete_convolution.h"
#include <algorithm> // std::min, std::max
#include <cmath>     // std::round
#include <omp.h>    // OpenMP
#include <immintrin.h> // AVX
#include <vector>
#include <mpi.h>


BMPImage applyConvolution(const BMPImage& input, const std::vector<double>& kernel, int kernelWidth, int kernelHeight) {
    // Check if kernel size matches kernelWidth * kernelHeight
    if(kernel.size() != static_cast<size_t>(kernelWidth * kernelHeight)) {
        throw std::runtime_error("Veličina kernela se ne poklapa sa kernelWidth * kernelHeight.");
    }

    BMPImage output;
    output.width = input.width;
    output.height = input.height;
    output.data.resize(input.width * input.height * 3);


    // Iterate through each pixel of the input image
    for (int y = 0; y < input.height; y++) {
        for (int x = 0; x < input.width; x++) {
            double sumR = 0.0, sumG = 0.0, sumB = 0.0;

            // Iterate through the kernel
            for (int ky = 0; ky < kernelHeight; ky++) {
                for (int kx = 0; kx < kernelWidth; kx++) {
                    int imageX = x + kx - halfKernelWidth;
                    int imageY = y + ky - halfKernelHeight;

                    // If the pixel is out of bounds, use zero-padding (i.e., value 0)
                    if (imageX < 0 || imageX >= input.width || imageY < 0 || imageY >= input.height)
                        continue;

                    int pixelIndex = (imageY * input.width + imageX) * 3;
                    double pixelR = static_cast<double>(input.data[pixelIndex + 0]);
                    double pixelG = static_cast<double>(input.data[pixelIndex + 1]);
                    double pixelB = static_cast<double>(input.data[pixelIndex + 2]);

                    double kVal = kernel[ky * kernelWidth + kx];

                    sumR += pixelR * kVal;
                    sumG += pixelG * kVal;
                    sumB += pixelB * kVal;
                }
            }

            // Resulting values are rounded and clamped to the range [0, 255]
            int outIndex = (y * input.width + x) * 3;
            output.data[outIndex + 0] = static_cast<uint8_t>(std::min(std::max(static_cast<int>(std::round(sumR)), 0), 255));
            output.data[outIndex + 1] = static_cast<uint8_t>(std::min(std::max(static_cast<int>(std::round(sumG)), 0), 255));
            output.data[outIndex + 2] = static_cast<uint8_t>(std::min(std::max(static_cast<int>(std::round(sumB)), 0), 255));
        }
    }

    return output;
}

BMPImage applyParallelConvolution(const BMPImage& input, const std::vector<double>& kernel, int kernelWidth, int kernelHeight) {
    if(kernel.size() != static_cast<size_t>(kernelWidth * kernelHeight)) {
        throw std::runtime_error("Veličina kernela se ne poklapa sa kernelWidth * kernelHeight.");
    }

    BMPImage output;
    output.width = input.width;
    output.height = input.height;
    output.data.resize(input.width * input.height * 3);

    int halfKernelWidth = kernelWidth / 2;
    int halfKernelHeight = kernelHeight / 2;

    #pragma omp parallel for collapse(2) schedule(static) 
    for (int y = 0; y < input.height; y++) {
        for (int x = 0; x < input.width; x++) {
            double sumR = 0.0, sumG = 0.0, sumB = 0.0;

            // Optimised kernel iteration
            for (int ky = -halfKernelHeight; ky <= halfKernelHeight; ky++) {
                for (int kx = -halfKernelWidth; kx <= halfKernelWidth; kx++) {
                    int imageX = x + kx;
                    int imageY = y + ky;

                    // If the pixel is out of bounds, use zero-padding (i.e., value 0)
                    if (imageX < 0 || imageX >= input.width || imageY < 0 || imageY >= input.height) continue;

                    int pixelIndex = (imageY * input.width + imageX) * 3;
                    double pixelR = static_cast<double>(input.data[pixelIndex + 0]);
                    double pixelG = static_cast<double>(input.data[pixelIndex + 1]);
                    double pixelB = static_cast<double>(input.data[pixelIndex + 2]);

                    double kVal = kernel[(ky + halfKernelHeight) * kernelWidth + (kx + halfKernelWidth)];

                    sumR += pixelR * kVal;
                    sumG += pixelG * kVal;
                    sumB += pixelB * kVal;
                }
            }

            // Resulting values are rounded and clamped to the range [0, 255]
            int outIndex = (y * input.width + x) * 3;
            output.data[outIndex + 0] = static_cast<uint8_t>(std::min(std::max(static_cast<int>(std::round(sumR)), 0), 255));
            output.data[outIndex + 1] = static_cast<uint8_t>(std::min(std::max(static_cast<int>(std::round(sumG)), 0), 255));
            output.data[outIndex + 2] = static_cast<uint8_t>(std::min(std::max(static_cast<int>(std::round(sumB)), 0), 255));
        }
    }

    return output;
}


BMPImage applyDistributedParallelConvolution(const BMPImage& input, const std::vector<double>& kernel, int kernelWidth, int kernelHeight) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(kernel.size() != static_cast<size_t>(kernelWidth * kernelHeight)) {
        throw std::runtime_error("Veličina kernela se ne poklapa sa kernelWidth * kernelHeight.");
    }

    BMPImage output;
    output.width = input.width;
    output.height = input.height;
    output.data.resize(input.width * input.height * 3);

    int halfKernelWidth = kernelWidth / 2;
    int halfKernelHeight = kernelHeight / 2;

    int rowsPerProcess = input.height / size;
    int startRow = rank * rowsPerProcess;
    int endRow = (rank == size - 1) ? input.height : startRow + rowsPerProcess;

    #pragma omp parallel for collapse(2) schedule(static)
    for (int y = startRow; y < endRow; y++) {
        for (int x = 0; x < input.width; x++) {
            double sumR = 0.0, sumG = 0.0, sumB = 0.0;

            for (int ky = 0; ky < kernelHeight; ky++) {
                for (int kx = 0; kx < kernelWidth; kx++) {
                    int imageX = x + kx - halfKernelWidth;
                    int imageY = y + ky - halfKernelHeight;

                    if (imageX < 0 || imageX >= input.width || imageY < 0 || imageY >= input.height)
                        continue;

                    int pixelIndex = (imageY * input.width + imageX) * 3;
                    double pixelR = static_cast<double>(input.data[pixelIndex + 0]);
                    double pixelG = static_cast<double>(input.data[pixelIndex + 1]);
                    double pixelB = static_cast<double>(input.data[pixelIndex + 2]);

                    double kVal = kernel[ky * kernelWidth + kx];

                    sumR += pixelR * kVal;
                    sumG += pixelG * kVal;
                    sumB += pixelB * kVal;
                }
            }

            int outIndex = (y * input.width + x) * 3;
            output.data[outIndex + 0] = static_cast<uint8_t>(std::min(std::max(static_cast<int>(std::round(sumR)), 0), 255));
            output.data[outIndex + 1] = static_cast<uint8_t>(std::min(std::max(static_cast<int>(std::round(sumG)), 0), 255));
            output.data[outIndex + 2] = static_cast<uint8_t>(std::min(std::max(static_cast<int>(std::round(sumB)), 0), 255));
        }
    }

    if (rank != 0) {
        MPI_Send(output.data.data() + startRow * input.width * 3, (endRow - startRow) * input.width * 3, MPI_UINT8_T, 0, 0, MPI_COMM_WORLD);
    } else {
        for (int i = 1; i < size; i++) {
            int recvStartRow = i * rowsPerProcess;
            int recvEndRow = (i == size - 1) ? input.height : recvStartRow + rowsPerProcess;
            MPI_Recv(output.data.data() + recvStartRow * input.width * 3, (recvEndRow - recvStartRow) * input.width * 3, MPI_UINT8_T, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

    return output;
}