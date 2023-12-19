#include <cstdint>
#include <string>
#include <vector>
#pragma pack(push, 1)
struct BitmapFileHeader {
  uint16_t fileType;
  uint32_t fileSize;
  uint16_t reserved1;
  uint16_t reserved2;
  uint32_t pixelDataOffset;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct BitmapInfoHeader {
  uint32_t headerSize;
  int32_t imageWidth;
  int32_t imageHeight;
  uint16_t planes;
  uint16_t bitsPerPixel;
  uint32_t compression;
  uint32_t imageSize;
  int32_t xPixelsPerMeter;
  int32_t yPixelsPerMeter;
  uint32_t totalColors;
  uint32_t importantColors;
};
#pragma pack(pop)

class ImageReader {
public:
  ImageReader();
  ~ImageReader();
  bool loadImage(const std::string &filePath);

  void saveGrayImage(const std::string &filePath,
                     const std::vector<uint8_t> &grayImageData, int width,
                     int height);

private:
};