#include "imageProcessor.h"
#include <cstdint>
#include <iostream>

ImageProcessor::ImageProcessor() {}

ImageProcessor::~ImageProcessor() {}

void ImageProcessor::processImage(const std::vector<uint8_t> &imageData,
                                  int width, int height, int bitsPerPixel) {
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int pixelIndex = (y * width + x) * (bitsPerPixel / 8);

      if (bitsPerPixel == 24) {
        // 24位BMP图像，每个像素由3字节表示：B, G, R
        uint8_t blue = imageData[pixelIndex];
        uint8_t green = imageData[pixelIndex + 1];
        uint8_t red = imageData[pixelIndex + 2];

        // 输出每个像素的颜色
        std::cout << "Pixel at (" << x << ", " << y << "): "
                  << "Red: " << static_cast<int>(red)
                  << ", Green: " << static_cast<int>(green)
                  << ", Blue: " << static_cast<int>(blue) << std::endl;
      }
      // 其他颜色深度的处理逻辑
    }
  }
}

std::vector<uint8_t>
ImageProcessor::colorGray1(const std::vector<uint8_t> &imageData, int width,
                           int height, int bitsPerPixel) {
  std::vector<uint8_t> grayImageData;
  grayImageData.reserve(width * height); // 假设灰度图像为8位深度

  if (bitsPerPixel == 24) {
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        int pixelIndex = (y * width + x) * 3;
        uint8_t blue = imageData[pixelIndex];
        uint8_t green = imageData[pixelIndex + 1];
        uint8_t red = imageData[pixelIndex + 2];

        uint8_t gray =
            static_cast<uint8_t>(red * 0.3 + green * 0.59 + blue * 0.11);
        grayImageData.push_back(gray);
      }
    }
  }
  // 其他颜色深度的处理逻辑

  return grayImageData;
}