#include "discrete_convolution.h"
#include "bmp_parser.h"
#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <cstdlib>  // std::exit
#include <sstream>
#include <chrono>
#include <mpi.h>

void printUsage() {
    std::cerr << "Usage: ./program P|S|A input.bmp output.bmp [kernelWidth kernelHeight k1 k2 ...]\n";
    std::cerr << "Example: ./program S input.bmp output.bmp 3 3 0 -1 0 -1 5 -1 0 -1 0\n";
    exit(1);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printUsage();
        return 1;
    }

    std::string mode = argv[1];  // "P" for parallel, "S" for sequential, "D" for distributed
    std::string inputFileName = argv[2];
    std::string outputFileName = argv[3];

    int kernelWidth = 3, kernelHeight = 3;
    std::vector<double> kernel;

    // Check if the mode is valid
    if (argc >= 6) {
        kernelWidth = std::stoi(argv[4]);
        kernelHeight = std::stoi(argv[5]);

        if (argc != 6 + kernelWidth * kernelHeight) {
            std::cerr << "Invalid kernel size or missing elements.\n";
            printUsage();
        }

        for (int i = 6; i < argc; i++) {
            kernel.push_back(std::stod(argv[i]));
        }
    } else {
        // Default kernel (3x3 averaging kernel)
        kernel = {1.0 / 9, 1.0 / 9, 1.0 / 9, 1.0 / 9, 1.0 / 9,  1.0 / 9, 1.0 / 9,  1.0 / 9, 1.0 / 9};
    }

    try {
        BMPImage inputImage = loadBMP(inputFileName);
        BMPImage outputImage;

        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();

        if (mode == "S") {
            start = std::chrono::high_resolution_clock::now();
            outputImage = applyConvolution(inputImage, kernel, kernelWidth, kernelHeight);
            end = std::chrono::high_resolution_clock::now();
        } else if (mode == "P") {
            start = std::chrono::high_resolution_clock::now();
            outputImage = applyParallelConvolution(inputImage, kernel, kernelWidth, kernelHeight);
            end = std::chrono::high_resolution_clock::now();
        } else if (mode == "D") {
            MPI_Init(&argc, &argv);
            start = std::chrono::high_resolution_clock::now();
            outputImage = applyDistributedParallelConvolution(inputImage, kernel, kernelWidth, kernelHeight);
            end = std::chrono::high_resolution_clock::now();
            MPI_Finalize();
        } else {
            std::cerr << "Invalid mode.\n";
            return 1;
        }

        

        std::chrono::duration<double> elapsed = end - start;

        std::cout << elapsed.count();
        if (saveToFile(outputFileName, outputImage)) {
            // std::cout << "Output saved to " << outputFileName << "\n";
        } else {
            std::cerr << "Failed to save output image.\n";
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}