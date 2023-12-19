#include <cstdint>
#include <vector>

class ImageProcessor {
public:
  ImageProcessor();
  ~ImageProcessor();

  void processImage(const std::vector<uint8_t> &imageData, int width,
                    int height, int bitsPerPixel);

  std::vector<uint8_t> colorGray1(const std::vector<uint8_t> &imageData,
                                  int width, int height, int bitsPerPixel);

private:
};
