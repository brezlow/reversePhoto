#include "Image.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace feiteng {

Image::Image() noexcept(true) : m_isInitial(false) {}

Image::Image(const std::string &filePath) { load(filePath); }

Image::~Image() {}

Image &Image::load(const std::string &filePath) {
  // 使用局部变量确保异常安全性
  Image loadedImage;

  try {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
      throw std::runtime_error("Unable to open file: " + filePath + "\n");
    }

    // 读取文件头和信息头
    file.read(reinterpret_cast<char *>(&loadedImage.m_fileHeader),
              sizeof(loadedImage.m_fileHeader));
    if (loadedImage.m_fileHeader.fileType != 0x4D42) {
      throw std::runtime_error("Not a BMP file\n");
    }

    file.read(reinterpret_cast<char *>(&loadedImage.m_infoHeader),
              sizeof(loadedImage.m_infoHeader));
    if (loadedImage.m_infoHeader.bitsPerPixel != 24) {
      throw std::runtime_error(
          "Unsupported bit depth: " +
          std::to_string(loadedImage.m_infoHeader.bitsPerPixel) +
          " bits per pixel.\n");
    }
    // 忽略color palette
    file.seekg(m_fileHeader.pixelDataOffset, std::ios::beg);

    // 读取像素数据
    int pixelDataSize = loadedImage.m_infoHeader.imageWidth * 3;
    int rowSize = (pixelDataSize + 3) & (~3);
    int padding = rowSize - pixelDataSize;

    std::vector<uint8_t> row(rowSize);
    loadedImage.m_imageData.reserve(loadedImage.m_infoHeader.imageHeight *
                                    rowSize);
    for (int y = 0; y < loadedImage.m_infoHeader.imageHeight; ++y) {
      file.read(reinterpret_cast<char *>(row.data()), rowSize);
      loadedImage.m_imageData.insert(loadedImage.m_imageData.end(), row.begin(),
                                     row.begin() + pixelDataSize);
    }

    loadedImage.m_isInitial = true;
    loadedImage.m_originalFilePath = filePath;

  } catch (const std::exception &e) {
    // 处理异常，确保资源得到正确释放
    throw e;
  }

  // 将局部变量的状态移动到当前对象
  *this = std::move(loadedImage);
  return *this;
}

Image &Image::print() {
  if (!m_isInitial) {
    throw std::runtime_error(
        "This object is empty, can't call Image::print function.\n");
  }

  // 输出像素信息
  const int height = m_infoHeader.imageHeight;
  const int width = m_infoHeader.imageWidth;
  const int bitsPerPixel = m_infoHeader.bitsPerPixel;

  if (bitsPerPixel != 24) {
    throw std::runtime_error("bitsPerPixel error.\n");
  }

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int pixelIndex = (y * width + x) * (bitsPerPixel / 8);

      uint8_t blue = m_imageData[pixelIndex];
      uint8_t green = m_imageData[pixelIndex + 1];
      uint8_t red = m_imageData[pixelIndex + 2];

      // 输出每个像素的颜色
      if (m_isHSV) {
        std::cout << "Pixel at (" << x << ", " << y << "): "
                  << "H: " << static_cast<int>(red)
                  << ", S: " << static_cast<int>(green)
                  << ", V: " << static_cast<int>(blue) << "\n";

      } else {
        std::cout << "Pixel at (" << x << ", " << y << "): "
                  << "Red: " << static_cast<int>(red)
                  << ", Green: " << static_cast<int>(green)
                  << ", Blue: " << static_cast<int>(blue) << "\n";
      }
    }
  }
  return *this;
}
Image &Image::RGBtoHSV() {
  if (!m_isInitial) {
    throw std::runtime_error(
        "This object is empty, can't call Image::print function.\n");
  }

  if (m_isHSV) {
    std::cerr << "This Image has hsv information"
              << "\n";
    return *this;
  }

  const int height = m_infoHeader.imageHeight;
  const int width = m_infoHeader.imageWidth;
  const int bitsPerPixel = m_infoHeader.bitsPerPixel;

  if (bitsPerPixel != 24) {
    throw std::runtime_error("bitsPerPixel error.\n");
  }

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int pixelIndex = (y * width + x) * (bitsPerPixel / 8);

      uint8_t blue = m_imageData[pixelIndex];
      uint8_t green = m_imageData[pixelIndex + 1];
      uint8_t red = m_imageData[pixelIndex + 2];

      // 归一化RGB的值到[0,1]
      double R = red / 255.0;
      double G = green / 255.0;
      double B = blue / 255.0;

      double Cmax = std::max({R, G, B});
      double Cmin = std::min({R, G, B});
      double delta = Cmax - Cmin;

      double H, S, V;

      V = Cmax;

      S = (Cmax > 0.0) ? delta / Cmax : 0.0;

      if (delta > 0.0) {
        if (Cmax == R) {
          H = 60.0 * std::fmod((G - B) / delta, 6.0);
        } else if (Cmax == G) {
          H = 60.0 * ((B - R) / delta + 2.0);
        } else if (Cmax == B) {
          H = 60.0 * ((R - G) / delta + 4.0);
        }

        H = (H < 0.0) ? H + 360.0 : H;
      } else {
        H = 0.0;
      }
      m_imageData[pixelIndex] = static_cast<uint8_t>(H * 255.0 / 360.0);
      m_imageData[pixelIndex + 1] = static_cast<uint8_t>(S * 255.0);
      m_imageData[pixelIndex + 2] = static_cast<uint8_t>(V * 255.0);
    }
  }
  m_isHSV = true;
  return *this;
}

Image &Image::HSVtoRGB() {
  if (!m_isInitial) {
    throw std::runtime_error(
        "This object is empty, can't call Image::print function.\n");
  }

  if (m_isHSV) {
    std::cerr << "This Image has no hsv information"
              << "\n";

    return *this;
  }

  const int height = m_infoHeader.imageHeight;
  const int width = m_infoHeader.imageWidth;
  const int bitsPerPixel = m_infoHeader.bitsPerPixel;

  if (bitsPerPixel != 24) {
    throw std::runtime_error("bitsPerPixel error.\n");
  }

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int pixelIndex = (y * width + x) * (bitsPerPixel / 8);

      uint8_t H8 = m_imageData[pixelIndex];
      uint8_t S8 = m_imageData[pixelIndex + 1];
      uint8_t V8 = m_imageData[pixelIndex + 2];

      // 将HSV的值从[0,1]映射到[0,360]和[0,100]
      double H = H8 * 360.0;
      double S = S8 * 100.0;
      double V = V8 * 100.0;

      double C = (V / 100) * (S / 100);
      double X = C * (1 - std::abs(fmod(H / 60.0, 2) - 1));
      double m = (V / 100) - C;

      double r, g, b;

      if (H >= 0 && H < 60) {
        r = C, g = X, b = 0;
      } else if (H >= 60 && H < 120) {
        r = X, g = C, b = 0;
      } else if (H >= 120 && H < 180) {
        r = 0, g = C, b = X;
      } else if (H >= 180 && H < 240) {
        r = 0, g = X, b = C;
      } else if (H >= 240 && H < 300) {
        r = X, g = 0, b = C;
      } else {
        r = C, g = 0, b = X;
      }

      // 将RGB的值从[0,1]映射到[0,255]
      uint8_t R = static_cast<uint8_t>(std::clamp((r + m) * 255, 0.0, 255.0));
      uint8_t G = static_cast<uint8_t>(std::clamp((g + m) * 255, 0.0, 255.0));
      uint8_t B = static_cast<uint8_t>(std::clamp((b + m) * 255, 0.0, 255.0));

      m_imageData[pixelIndex] = B;
      m_imageData[pixelIndex + 1] = G;
      m_imageData[pixelIndex + 2] = R;
    }
  }
  m_isHSV = false;
  return *this;
}

Image &Image::toGray() {
  if (!m_isInitial) {
    throw std::runtime_error(
        "This object is empty, can't call Image::toGray function.\n");
  }
  if (m_isHSV) {
    throw std::runtime_error(
        "This object is turn to HSV,please turn to RGB .\n");
  }

  // 创建灰度图像
  Image grayImage;
  grayImage.m_fileHeader.fileType = 0x4D42; //"BM"
  grayImage.m_fileHeader.reserved1 = 0;
  grayImage.m_fileHeader.reserved2 = 0;
  grayImage.m_fileHeader.pixelDataOffset =
      sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + 256 * 4;

  grayImage.m_infoHeader.headerSize = sizeof(BitmapInfoHeader);
  grayImage.m_infoHeader.imageWidth = m_infoHeader.imageWidth;
  grayImage.m_infoHeader.imageHeight = m_infoHeader.imageHeight;
  grayImage.m_infoHeader.bitsPerPixel = 8; // 灰度图像为8位
  grayImage.m_infoHeader.compression = 0;
  grayImage.m_infoHeader.imageSize =
      m_infoHeader.imageHeight * (m_infoHeader.imageWidth + 3) &
      (~3); // 计算行大小并四字节对齐
  grayImage.m_infoHeader.xPixelsPerMeter = 0;
  grayImage.m_infoHeader.yPixelsPerMeter = 0;
  grayImage.m_infoHeader.totalColors = 256;
  grayImage.m_infoHeader.importantColors = 256;

  grayImage.m_fileHeader.fileSize =
      grayImage.m_fileHeader.pixelDataOffset + grayImage.m_infoHeader.imageSize;

  // 遍历每个像素并计算灰度值
  for (size_t i = 0; i < m_imageData.size(); i += 3) {
    uint8_t blue = m_imageData[i];
    uint8_t green = m_imageData[i + 1];
    uint8_t red = m_imageData[i + 2];

    uint8_t gray = static_cast<uint8_t>(red * 0.3 + green * 0.59 + blue * 0.11);
    grayImage.m_imageData.push_back(gray);
  }

  grayImage.m_isInitial = true;

  // 将局部变量的状态移动到当前对象
  *this = std::move(grayImage);
  return *this;
}

// 高斯滤波函数
Image &Image::gaussianBlur(double sigma) {
  if (!m_isInitial) {
    throw std::runtime_error(
        "This object is empty, can't call Image::gaussianBlur function.\n");
  }

  // 获取图像的宽度、高度和每像素的位数
  const int height = m_infoHeader.imageHeight;
  const int width = m_infoHeader.imageWidth;
  const int bitsPerPixel = m_infoHeader.bitsPerPixel;

  if (bitsPerPixel != 24) {
    throw std::runtime_error("bitsPerPixel error.\n");
  }

  int ksize = calculateKernelSize(sigma);

  std::vector<double> kernel = calculateGaussianKernel(sigma, ksize);

  // Apply horizontal Gaussian blur
  applyHorizontalBlur(kernel, height, width);

  // Apply vertical Gaussian blur
  applyVerticalBlur(kernel, height, width);

  return *this;
}

Image &Image::saveGrayImage(std::string newFilePath) {
  if (!m_isInitial) {
    throw std::runtime_error(
        "This object is empty, can't call Image::saveGrayImage function.\n");
  }

  // 如果未提供新文件路径，则使用默认命名规则
  if (newFilePath.empty()) {
    newFilePath = m_originalFilePath;
  }
  size_t extensionPos = newFilePath.rfind('.');
  if (extensionPos != std::string::npos) {
    newFilePath.replace(extensionPos, std::string::npos, "_gray.bmp");
  } else {
    newFilePath += "_gray.bmp";
  }

  // 创建并写入灰度图像文件
  std::ofstream outFile(newFilePath, std::ios::binary);
  if (!outFile) {
    throw std::runtime_error("Unable to open file for writing: " + newFilePath +
                             "\n");
  }
  outFile.write(reinterpret_cast<const char *>(&m_fileHeader),
                sizeof(m_fileHeader));
  outFile.write(reinterpret_cast<const char *>(&m_infoHeader),
                sizeof(m_infoHeader));

  // 写入256色的灰度调色板
  for (int i = 0; i < 256; ++i) {
    uint8_t paletteEntry[4] = {static_cast<uint8_t>(i), static_cast<uint8_t>(i),
                               static_cast<uint8_t>(i), 0};
    outFile.write(reinterpret_cast<const char *>(paletteEntry), 4);
  }

  // 写入图像数据
  int rowSize = (m_infoHeader.imageWidth + 3) & (~3); // 行大小对齐到4字节
  for (int y = 0; y < m_infoHeader.imageHeight; ++y) {
    outFile.write(reinterpret_cast<const char *>(m_imageData.data() +
                                                 y * m_infoHeader.imageWidth),
                  m_infoHeader.imageWidth);

    // 写入每行的填充字节（如果有）
    if (rowSize > m_infoHeader.imageWidth) {
      static const char padding[3] = {0, 0, 0};
      outFile.write(padding, rowSize - m_infoHeader.imageWidth);
    }
  }
  outFile.close();

  return *this;
}

Image &Image::saveColorImage(std::string newFilePath) {
  if (!m_isInitial) {
    throw std::runtime_error(
        "This object is empty, can't call Image::saveColorImage function.\n");
  }

  // 如果未提供新文件路径，则使用默认命名规则
  if (newFilePath.empty()) {
    newFilePath = m_originalFilePath;
  }
  size_t extensionPos = newFilePath.rfind('.');
  if (extensionPos != std::string::npos) {
    newFilePath.replace(extensionPos, std::string::npos, "_color.bmp");
  } else {
    newFilePath += "_color.bmp";
  }

  // 创建并写入彩色图像文件
  std::ofstream outFile(newFilePath, std::ios::binary);
  if (!outFile) {
    throw std::runtime_error("Unable to open file for writing: " + newFilePath +
                             "\n");
  }
  outFile.write(reinterpret_cast<const char *>(&m_fileHeader),
                sizeof(m_fileHeader));
  outFile.write(reinterpret_cast<const char *>(&m_infoHeader),
                sizeof(m_infoHeader));

  // 写入图像数据
  int rowSize = (m_infoHeader.imageWidth * 3 + 3) & (~3); // 行大小对齐到4字节
  for (int y = 0; y < m_infoHeader.imageHeight; ++y) {
    outFile.write(
        reinterpret_cast<const char *>(m_imageData.data() + y * rowSize),
        m_infoHeader.imageWidth * 3);

    // 写入每行的填充字节（如果有）
    if (rowSize > m_infoHeader.imageWidth * 3) {
      static const char padding[3] = {0, 0, 0};
      outFile.write(padding, rowSize - m_infoHeader.imageWidth * 3);
    }
  }
  outFile.close();

  return *this;
}

Image &Image::push() {
  // 使用局部变量确保异常安全性
  std::shared_ptr<Image> tempImage = std::make_shared<Image>(*this);
  m_lastImage = tempImage;
  return *this;
}

Image &Image::pop() {
  if (m_lastImage) {
    *this = *m_lastImage;
    m_lastImage.reset();
  }
  return *this;
}

} // namespace feiteng
