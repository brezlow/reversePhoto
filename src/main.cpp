#include "imageReader.h"
#include <iostream>
#include <string>

int main() {
  ImageReader reader;

  // 指定要加载的图像文件的路径
  std::string filePath = "/home/breslow/work/cpp/tool/opencv/reversePhoto/assets/fern.bmp";

  // 使用 ImageReader 加载并处理图像
  bool result = reader.loadImage(filePath);

  if (!result) {
    std::cerr << "图像加载失败。" << std::endl;
    return 1; // 返回错误码
  }

  std::cout << "图像加载和处理成功。" << std::endl;
  return 0; // 成功执行
}
