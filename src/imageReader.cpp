#include "imageReader.h"
#include "imageProcessor.h"
#include <fstream>
#include <iostream>

ImageReader::ImageReader() {}

ImageReader::~ImageReader() {}

bool ImageReader::loadImage(const std::string &filePath) {

  std::ifstream file(filePath, std::ios::binary);
  if (!file) {
    std::cerr << "Unable to open file: " << filePath << std::endl;
    return false;
  }

  BitmapFileHeader fileHeader;

  file.read(reinterpret_cast<char *>(&fileHeader), sizeof(fileHeader));

  if (fileHeader.fileType != 0x4D42) {
    std::cerr << "Not a BMP file" << std::endl;
    return false;
  }

  BitmapInfoHeader infoHeader;
  file.read(reinterpret_cast<char *>(&infoHeader), sizeof(infoHeader));

  std::cout << "Width: " << infoHeader.imageWidth
            << ", Height: " << infoHeader.imageHeight << std::endl;

  // 检查颜色深度
  if (infoHeader.bitsPerPixel == 16) {
    // 处理 16 位图像
    // 忽略color palette
    file.seekg(fileHeader.pixelDataOffset, std::ios::beg);

    // 读取pixel data
    int rowSize =
        ((infoHeader.imageWidth * infoHeader.bitsPerPixel + 31) / 32) * 4;
    int padding = rowSize - (infoHeader.imageWidth * 2);

    std::vector<uint8_t> row(rowSize);

    std::vector<uint8_t> imageData;
    imageData.reserve(infoHeader.imageHeight * rowSize);

    for (int y = 0; y < infoHeader.imageHeight; ++y) {
      file.read(reinterpret_cast<char *>(row.data()), rowSize);
      imageData.insert(imageData.end(), row.begin(), row.end() - padding);
    }

    ImageProcessor processor;
    processor.processImage(imageData, infoHeader.imageWidth,
                           infoHeader.imageHeight, infoHeader.bitsPerPixel);

  } else {
    std::cerr << "Unsupported bit depth: " << infoHeader.bitsPerPixel
              << " bits per pixel." << std::endl;
  }

  return true;
}