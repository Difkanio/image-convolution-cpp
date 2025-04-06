#include "bmp_parser.h"
#include <fstream>
#include <sstream>

// Implementation of the function to load a BMP image from a file.
BMPImage loadBMP(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file)
        throw std::runtime_error("Greška: Ne mogu otvoriti fajl: " + filename);
    
    BMPFileHeader fileHeader;
    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    if (file.gcount() != sizeof(fileHeader))
        throw std::runtime_error("Greška: Neuspešno čitanje BMP file header-a.");
    
    // Check if the file is a BMP file by checking the magic number
    if (fileHeader.bfType != 0x4D42)
        throw std::runtime_error("Greška: Fajl nije validan BMP fajl.");
    
    BMPInfoHeader infoHeader;
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
    if (file.gcount() != sizeof(infoHeader))
        throw std::runtime_error("Greška: Neuspešno čitanje BMP info header-a.");
    
    // Check if the image is 24-bit and uncompressed
    if (infoHeader.biBitCount != 24)
        throw std::runtime_error("Greška: Podržane su samo 24-bitne BMP slike.");
    if (infoHeader.biCompression != 0)
        throw std::runtime_error("Greška: Kompresovane BMP slike nisu podržane.");
    
    BMPImage image;
    image.width  = infoHeader.biWidth;
    image.height = infoHeader.biHeight;
    
    // If the image height is negative, the image is in top-down format;
    // otherwise, we need to invert the rows to store them in top-down format.
    bool flipVertical = true;
    if (image.height < 0) {
        image.height = -image.height;
        flipVertical = false;
    }
    
    // Every row of pixel data is padded to a multiple of 4 bytes.
    int rowPadded = (image.width * 3 + 3) & (~3);
    image.data.resize(image.width * image.height * 3);
    
    // Seek to the offset where the pixel data starts
    file.seekg(fileHeader.bfOffBits, std::ios::beg);
    if (!file)
        throw std::runtime_error("Greška: Ne mogu se pomeriti do bitmap podataka.");
    
    std::vector<uint8_t> rowData(rowPadded);
    
    for (int i = 0; i < image.height; i++) {
        file.read(reinterpret_cast<char*>(rowData.data()), rowPadded);
        if (file.gcount() != rowPadded) {
            std::stringstream ss;
            ss << "Greška: Nedovoljno podataka pri čitanju reda " << i;
            throw std::runtime_error(ss.str());
        }
        
        int destRow = flipVertical ? (image.height - 1 - i) : i;
        for (int j = 0; j < image.width; j++) {
            // BMP format stores pixels in BGR order, so we need to swap them to RGB.
            uint8_t B = rowData[j * 3 + 0];
            uint8_t G = rowData[j * 3 + 1];
            uint8_t R = rowData[j * 3 + 2];
            
            int idx = (destRow * image.width + j) * 3;
            image.data[idx + 0] = R;
            image.data[idx + 1] = G;
            image.data[idx + 2] = B;
        }
    }
    
    return image;
}

// Implementation of the function to save a BMP image to a file.
bool saveToFile(const std::string &filename, const BMPImage &image) {

    // Calculating the size of a row with padding (row must be aligned to 4 bytes)
    int rowSize   = image.width * 3;
    int rowPadded = (rowSize + 3) & (~3);
    int dataSize  = rowPadded * image.height;
    
    // Preparing BMP file header
    BMPFileHeader fileHeader;
    fileHeader.bfType      = 0x4D42; // "BM"
    fileHeader.bfOffBits   = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
    fileHeader.bfSize      = fileHeader.bfOffBits + dataSize;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    
    BMPInfoHeader infoHeader;
    infoHeader.biSize          = sizeof(BMPInfoHeader);
    infoHeader.biWidth         = image.width;
    infoHeader.biHeight        = image.height;
    infoHeader.biPlanes        = 1;
    infoHeader.biBitCount      = 24;
    infoHeader.biCompression   = 0;
    infoHeader.biSizeImage     = dataSize;

    //Using arbitrary values for resolution (e.g., 2835 pixels/meter ~ 72 DPI)
    infoHeader.biXPelsPerMeter = 2835;
    infoHeader.biYPelsPerMeter = 2835;
    infoHeader.biClrUsed       = 0;
    infoHeader.biClrImportant  = 0;
    
    std::ofstream file(filename, std::ios::binary);
    if (!file)
        return false;
    
    // Write the file header and info header to the file
    file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    if (!file)
        return false;
    file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));
    if (!file)
        return false;
    
    // Preparing a temporary buffer for the row (with padding bytes)
    std::vector<uint8_t> rowData(rowPadded, 0);
    
    // BMP expects bottom-up order; since the data in BMPImage is in top-down order, 
    // we iterate in reverse order.
    for (int i = image.height - 1; i >= 0; i--) {
        for (int j = 0; j < image.width; j++) {
            int idx = (i * image.width + j) * 3;
            uint8_t R = image.data[idx + 0];
            uint8_t G = image.data[idx + 1];
            uint8_t B = image.data[idx + 2];
            // BMP format stores pixels in BGR order
            rowData[j * 3 + 0] = B;
            rowData[j * 3 + 1] = G;
            rowData[j * 3 + 2] = R;
        }
        file.write(reinterpret_cast<const char*>(rowData.data()), rowPadded);
        if (!file)
            return false;
    }
    
    return true;
}
