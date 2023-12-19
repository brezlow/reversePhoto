#include "ImageProcessor.h"
#include <cstdint>
#include <iostream>

ImageProcessor::ImageProcessor() {}

ImageProcessor::~ImageProcessor() {}

void ImageProcessor::processImage(const std::vector<uint8_t> &imageData,
                                  int width, int height, int bitsPerPixel) {
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int pixelIndex = (y * width + x) * (bitsPerPixel / 8);
      if (bitsPerPixel == 16) {
        uint16_t pixel =
            *reinterpret_cast<const uint16_t *>(&imageData[pixelIndex]);
        this->processPixel(pixel); // 使用 this-> 来明确调用成员方法
      }
      // 可以添加其他位深度的处理逻辑
    }
  }
}

void ImageProcessor::processPixel(uint16_t pixel) {
  // 处理单个像素的逻辑
  std::cout << "Processing pixel: " << pixel << std::endl;
}