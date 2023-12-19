#include "imageReader.h"
#include "imageProcessor.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

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
  if (infoHeader.bitsPerPixel == 24) {
    // 处理 24 位图像
    // 忽略color palette
    file.seekg(fileHeader.pixelDataOffset, std::ios::beg);

    // 计算每行的像素数据大小和对齐后的行大小
    int pixelDataSize = infoHeader.imageWidth * 3; // 每行的像素数据大小
    int rowSize = (pixelDataSize + 3) & (~3);      // 行大小对齐到4字节
    int padding = rowSize - pixelDataSize;         // 计算填充

    std::vector<uint8_t> row(rowSize);

    std::vector<uint8_t> imageData;
    imageData.reserve(infoHeader.imageHeight * rowSize);

    for (int y = 0; y < infoHeader.imageHeight; ++y) {
      file.read(reinterpret_cast<char *>(row.data()), rowSize);
      imageData.insert(imageData.end(), row.begin(),
                       row.begin() + pixelDataSize);
    }

    ImageProcessor processor;
    std::vector<uint8_t> grayImageData =
        processor.colorGray1(imageData, infoHeader.imageWidth,
                             infoHeader.imageHeight, infoHeader.bitsPerPixel);
    // 保存灰度图像到新的文件
    saveGrayImage(filePath, grayImageData, infoHeader.imageWidth,
                  infoHeader.imageHeight);

  } else {
    std::cerr << "Unsupported bit depth: " << infoHeader.bitsPerPixel
              << " bits per pixel." << std::endl;
  }

  return true;
}

void ImageReader::saveGrayImage(const std::string &originalFilePath,
                                const std::vector<uint8_t> &grayImageData,
                                int width, int height) {
  // 修改文件路径
  std::string newFilePath = originalFilePath;
  size_t extensionPos = newFilePath.rfind('.');
  if (extensionPos != std::string::npos) {
    newFilePath.replace(extensionPos, std::string::npos, "_gray.bmp");
  } else {
    newFilePath += "_gray.bmp"; // 如果没有找到扩展名，则直接添加
  }

  std::ofstream outFile(newFilePath, std::ios::binary);

  // 初始化并写入位图文件头
  BitmapFileHeader fileHeader;
  fileHeader.fileType = 0x4D42; // "BM"
  fileHeader.reserved1 = 0;
  fileHeader.reserved2 = 0;
  fileHeader.pixelDataOffset = sizeof(BitmapFileHeader) +
                               sizeof(BitmapInfoHeader) +
                               256 * 4; // 调色板大小为256*4

  // 初始化并写入位图信息头
  BitmapInfoHeader infoHeader;
  infoHeader.headerSize = sizeof(BitmapInfoHeader);
  infoHeader.imageWidth = width;
  infoHeader.imageHeight = height;
  infoHeader.planes = 1;
  infoHeader.bitsPerPixel = 8; // 灰度图像为8位
  infoHeader.compression = 0;
  infoHeader.imageSize =
      height * ((width + 3) & (~3)); // 计算行大小并四字节对齐
  infoHeader.xPixelsPerMeter = 0;
  infoHeader.yPixelsPerMeter = 0;
  infoHeader.totalColors = 256;
  infoHeader.importantColors = 256;

  fileHeader.fileSize = fileHeader.pixelDataOffset + infoHeader.imageSize;

  outFile.write(reinterpret_cast<const char *>(&fileHeader),
                sizeof(fileHeader));
  outFile.write(reinterpret_cast<const char *>(&infoHeader),
                sizeof(infoHeader));

  // 写入256色的灰度调色板
  for (int i = 0; i < 256; ++i) {
    uint8_t paletteEntry[4] = {static_cast<uint8_t>(i), static_cast<uint8_t>(i),
                               static_cast<uint8_t>(i), 0};
    outFile.write(reinterpret_cast<const char *>(paletteEntry), 4);
  }

  // 写入图像数据
  int rowSize = (width + 3) & (~3); // 行大小对齐到4字节
  for (int y = 0; y < height; ++y) {
    outFile.write(
        reinterpret_cast<const char *>(grayImageData.data() + y * width),
        width);

    // 写入每行的填充字节（如果有）
    if (rowSize > width) {
      static const char padding[3] = {0, 0, 0};
      outFile.write(padding, rowSize - width);
    }
  }

  outFile.close();
}
