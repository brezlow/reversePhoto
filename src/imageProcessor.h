#include <cstdint>
#include <vector>

class ImageProcessor {
public:
  ImageProcessor();
  ~ImageProcessor();

  void processImage(const std::vector<uint8_t> &imageData, int width,
                    int height, int bitsPerPixel);

private:
  void processPixel(uint16_t piexl);
};
