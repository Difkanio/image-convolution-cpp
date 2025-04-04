# Discrete Convolution on Images (C++)

This project implements **discrete convolution** on images using **C++**, allowing users to apply custom convolution kernels to perform operations like **blurring**, **sharpening**, **edge detection**, and more.

## Features

- Supports 24-bit BMP image input and output
- Customizable convolution kernel (via code or CLI input)
- Handles edge cases with zero-padding
- Efficient implementation
- Optional OpenMP-based and MPI-based versions for better performance
- Modular code for easy kernel experimentation

## Requirements

- C++17 or later
- OpenMP (optional, for parallel version)
- MPI (optional, for parallel version)
- Tested on Linux
