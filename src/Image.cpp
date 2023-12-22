#include "Image.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

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
      std::cout << "Pixel at (" << x << ", " << y << "): "
                << "Red: " << static_cast<int>(red)
                << ", Green: " << static_cast<int>(green)
                << ", Blue: " << static_cast<int>(blue) << "\n";
    }
  }

  return *this;
}

Image &Image::toGray() {
  if (!m_isInitial) {
    throw std::runtime_error(
        "This object is empty, can't call Image::toGray function.\n");
  }

  // 创建灰度图像
  Image grayImage;
  grayImage.m_infoHeader = m_infoHeader;
  grayImage.m_infoHeader.bitsPerPixel = 8; // 灰度图像为8位

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

Image &Image::saveGrayImage(std::string newFilePath) {
  if (!m_isInitial) {
    throw std::runtime_error(
        "This object is empty, can't call Image::saveGrayImage function.\n");
  }

  // 如果未提供新文件路径，则使用默认命名规则
  if (newFilePath.empty()) {
    newFilePath = m_originalFilePath + "_gray.bmp";
  }

  // 创建并写入灰度图像文件
  std::ofstream outFile(newFilePath, std::ios::binary);
  if (!outFile) {
    throw std::runtime_error("Unable to open file for writing: " + newFilePath +
                             "\n");
  }

  // ... （与之前相似的文件头和信息头的写入）

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
