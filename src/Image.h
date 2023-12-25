#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace feiteng {

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

class Image {
public:
  Image() noexcept(true);
  Image(const std::string &filePath);
  ~Image();

  Image &load(const std::string &filePath);
  Image &saveGrayImage(std::string newFilePath = "");
  Image &print();
  Image &toGray();
  Image &RGBtoHSV();
  Image &HSVtoRGB();

protected:
  Image &push();
  Image &pop();

private:
  bool m_isInitial{false};
  bool m_isHSV{false};
  std::string m_originalFilePath;
  BitmapFileHeader m_fileHeader{};
  BitmapInfoHeader m_infoHeader{};
  std::vector<uint8_t> m_imageData{};

  std::shared_ptr<Image> m_lastImage{};
};

} // namespace feiteng
