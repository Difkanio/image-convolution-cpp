#pragma once


#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>

// BMP file format structures
#pragma pack(push, 1)
struct BMPFileHeader {
    uint16_t bfType;      // "BM" (0x4D42)
    uint32_t bfSize;      // Total file size in bytes
    uint16_t bfReserved1; // Reserved (must be 0)
    uint16_t bfReserved2; // Reserved (must be 0)
    uint32_t bfOffBits;   // Offset to start of pixel data
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BMPInfoHeader {
    uint32_t biSize;          // Size of this header (40 bytes)
    int32_t  biWidth;         // Width of the image in pixels
    int32_t  biHeight;        // Height of the image in pixels(positive value means top-down, negative value means bottom-up)
    uint16_t biPlanes;        // Number of color planes (must be 1)
    uint16_t biBitCount;      // Bits per pixel (1, 4, 8, or 24)
    uint32_t biCompression;   // Compression type (0 = none, 1 = RLE 8-bit, 2 = RLE 4-bit)
    uint32_t biSizeImage;     // Size of the pixel data (can be 0 for uncompressed images)
    int32_t  biXPelsPerMeter; // Horizontal resolution (pixels per meter)
    int32_t  biYPelsPerMeter; // Vertical resolution (pixels per meter)
    uint32_t biClrUsed;       // Number of colors in the color palette (0 = default 2^n)
    uint32_t biClrImportant;  // Number of important colors (0 = all colors are important)
};
#pragma pack(pop)

struct BMPImage {
    int32_t width;              // Image width
    int32_t height;             // Image height
    std::vector<uint8_t> data;  // Pixel data (RGB format)
};

// Loads a BMP image from a file. Throws std::runtime_error on error.
BMPImage loadBMP(const std::string &filename);

// Saves a BMP image to a file (24-bit uncompressed BMP format).
// Returns true if the operation is successful, otherwise false.
bool saveToFile(const std::string &filename, const BMPImage &image);

