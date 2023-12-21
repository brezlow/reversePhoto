#include "imageReader.h"
#include "imageProcessor.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdexcept>

namespace feiteng {

Image::Image() noexcept(true) {}

Image::Image(const std::string& filePath) {
	load(filePath);
}

Image::~Image() {}

Image& Image::push() {
	m_lastImage = std::unique_ptr<Image>(new Image); // std::make_unique<Image>() for c++14
	m_lastImage->m_isInitial 	= std::move(m_isInitial);
	m_lastImage->m_fileHeader 	= std::move(m_fileHeader);
	m_lastImage->m_infoHeader 	= std::move(m_infoHeader);
	m_lastImage->m_imageData 	= std::move(m_imageData);
	return *this;
}

Image& Image::pop() {
	m_isInitial 	= std::move(m_lastImage->m_isInitial);
	m_fileHeader 	= std::move(m_lastImage->m_fileHeader);
	m_infoHeader 	= std::move(m_lastImage->m_infoHeader);
	m_imageData 	= std::move(m_lastImage->m_imageData);
	m_lastImage		= nullptr;
}

Image& Image::load(const std::string &filePath) {
	if(m_isInitial)
		push();
	
	try{
		
		std::ifstream file(filePath, std::ios::binary);
		if (!file)
			throw std::runtime_error("Unable to open file: " + filePath + "\n");
		
		file.read(reinterpret_cast<char *>(&m_fileHeader), sizeof(m_fileHeader));
		if (m_fileHeader.fileType != 0x4D42)
			throw std::runtime_error("Not a BMP file\n");
		
		file.read(reinterpret_cast<char *>(&m_infoHeader), sizeof(m_infoHeader));
		if (m_infoHeader.bitsPerPixel != 24)
			throw std::runtime_error("Unsupported bit depth: " + std::to_string(m_infoHeader.bitsPerPixel) + " bits per pixel.\n");
		
		file.seekg(m_fileHeader.pixelDataOffset, std::ios::beg);
		
		int pixelDataSize = m_infoHeader.imageWidth * 3; // 每行的像素数据大小
		int rowSize = (pixelDataSize + 3) & (~3);      // 行大小对齐到4字节
		int padding = rowSize - pixelDataSize;         // 计算填充
		
		std::vector<uint8_t> row(rowSize);
		m_imageData.reserve(m_infoHeader.imageHeight * rowSize);
		for (int y = 0; y < m_infoHeader.imageHeight; ++y) {
			file.read(reinterpret_cast<char *>(row.data()), rowSize);
			m_imageData.insert(m_imageData.end(), row.begin(), row.begin() + pixelDataSize);
		}
		
		m_isInitial = true;
		
	} catch(const std::exception& e) {
		pop();
		throw e;
	} catch(...) {
		throw std::exception{};
	}
	return *this;
}

Image& Image::print() {
	if(!m_isInitial) 
		throw std::runtime_error("This object is empty, can't call Image::print function.\n");
		
	const int height = m_infoHeader.imageHeight;
	const int width = m_infoHeader.imageWidth;
	const int bitsPerPixel = m_infoHeader.bitsPerPixel;
	if(bitsPerPixel != 24)
		throw std::runtime_error("bitsPerPixel error.\n"); 
		
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

Image& Image::toGray() {
	if(!m_isInitial) 
		throw std::runtime_error("This object is empty, can't call Image::print function.\n");
	
	const std::vector<uint8_t>& imageData = m_lastImage->m_imageData;
	const int height = m_infoHeader.imageHeight;
	const int width = m_infoHeader.imageWidth;
	const int bitsPerPixel = m_infoHeader.bitsPerPixel;
	
	if (bitsPerPixel != 24) 
		std::runtime_error("bitsPerPixel error.\n");

  	std::vector<uint8_t> grayImageData;
  	grayImageData.reserve(width * height); // 假设灰度图像为8位深度
	
	push();
	
	try {
		
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
	  	// 其他颜色深度的处理逻辑
	  	
	  	m_imageData = std::move(grayImageData);
  	
	} catch(const std::exception& e) {
		pop();
		throw e;
	} catch(...) {
		throw std::exception{};
	}
	return *this;
}

Image& Image::saveGrayImage(std::string newFilePath) {
    const std::vector<uint8_t>& grayImageData = m_imageData;
    const int height = m_infoHeader.imageHeight;
	const int width = m_infoHeader.imageWidth;
	
	if(newFilePath == "") 
		newFilePath = "default" + std::to_string(std::rand());
	
	newFilePath += "_gray.bmp";

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
  return *this;
}

}
