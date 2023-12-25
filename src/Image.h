#include <algorithm>
#include <cstdint>
#include <math.h>
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
  Image &saveColorImage(std::string newFilePath = "");
  Image &print();
  Image &toGray();
  Image &RGBtoHSV();
  Image &HSVtoRGB();
  Image &gaussianBlur(double sigma);

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

  // 计算高斯滤波
  int calculateKernelSize(double sigma) {
    int ksize =
        std::max(3, static_cast<int>(ceil(sigma * 3) * 2 +
                                     1)); // Dynamic kernel size calculation
    if (ksize % 2 == 0) {
      ksize++;
    }
    return ksize;
  }

  std::vector<double> calculateGaussianKernel(double sigma, int ksize) {
    std::vector<double> kernel(ksize);
    const double PI = 3.14159265358979323846;
    const double sigmaSqr = sigma * sigma;
    const double twoSigmaSqr = 2 * sigmaSqr;
    double sum = 0.0;

    for (int i = 0; i < ksize; ++i) {
      int x = i - ksize / 2;
      kernel[i] = exp(-static_cast<double>(x * x) / twoSigmaSqr) /
                  (sqrt(PI * twoSigmaSqr));
      sum += kernel[i];
    }

    // Normalize the kernel
    for (int i = 0; i < ksize; ++i) {
      kernel[i] /= sum;
    }

    return kernel;
  }

  void applyHorizontalBlur(const std::vector<double> &kernel, int height,
                           int width) {
    std::vector<uint8_t> tempData(height * width *
                                  (m_infoHeader.bitsPerPixel / 8));

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        applyBlurToPixel(kernel, tempData, height, width, x, y, true);
      }
    }

    tempData.swap(m_imageData);
  }

  void applyVerticalBlur(const std::vector<double> &kernel, int height,
                         int width) {
    std::vector<uint8_t> tempData(height * width *
                                  (m_infoHeader.bitsPerPixel / 8));

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        applyBlurToPixel(kernel, tempData, height, width, x, y, false);
      }
    }

    tempData.swap(m_imageData);
  }

  void applyBlurToPixel(const std::vector<double> &kernel,
                        std::vector<uint8_t> &tempData, int height, int width,
                        int x, int y, bool horizontal) {
    int pixelIndex = (y * width + x) * (m_infoHeader.bitsPerPixel / 8);
    double weightSum = 0.0;
    double rSum = 0.0;
    double gSum = 0.0;
    double bSum = 0.0;

    for (size_t i = 0; i < kernel.size(); ++i) {
      int idx =
          horizontal ? x - kernel.size() / 2 + i : y - kernel.size() / 2 + i;
      idx = std::clamp(idx, 0, horizontal ? width - 1 : height - 1);

      int neighborIndex =
          horizontal ? (y * width + idx) * (m_infoHeader.bitsPerPixel / 8)
                     : (idx * width + x) * (m_infoHeader.bitsPerPixel / 8);

      uint8_t blue = m_imageData[neighborIndex];
      uint8_t green = m_imageData[neighborIndex + 1];
      uint8_t red = m_imageData[neighborIndex + 2];

      double weight = kernel[i];

      weightSum += weight;
      rSum += red * weight;
      gSum += green * weight;
      bSum += blue * weight;
    }
    uint8_t newBlue = round(bSum / weightSum);
    uint8_t newGreen = round(gSum / weightSum);
    uint8_t newRed = round(rSum / weightSum);
    newBlue =
        std::clamp(newBlue, static_cast<uint8_t>(0), static_cast<uint8_t>(255));
    newGreen = std::clamp(newGreen, static_cast<uint8_t>(0),
                          static_cast<uint8_t>(255));
    newRed =
        std::clamp(newRed, static_cast<uint8_t>(0), static_cast<uint8_t>(255));

    tempData[pixelIndex] = newBlue;
    tempData[pixelIndex + 1] = newGreen;
    tempData[pixelIndex + 2] = newRed;
  }
};

} // namespace feiteng
