#include "ImageProcessor.h"
#include "Maths.h"

#include "jpge.h"

#include <vector>
#include <iostream>
#include <fstream>
#include "libyuv/basic_types.h"
#include "CL/cl.h"

#define AVG(a, b) (((a) + (b)) >> 1)


static void BayerRowBG(const uint8* src_bayer0, int src_stride_bayer,
	uint8* dst_argb, int pix) {
	const uint8* src_bayer1 = src_bayer0 + src_stride_bayer;
	uint8 g = src_bayer0[1];
	uint8 r = src_bayer1[1];
	int x;
	for (x = 0; x < pix - 2; x += 2) {
		dst_argb[0] = src_bayer0[0];
		dst_argb[1] = AVG(g, src_bayer0[1]);
		dst_argb[2] = AVG(r, src_bayer1[1]);
		dst_argb[3] = 255U;
		dst_argb[4] = AVG(src_bayer0[0], src_bayer0[2]);
		dst_argb[5] = src_bayer0[1];
		dst_argb[6] = src_bayer1[1];
		dst_argb[7] = 255U;
		g = src_bayer0[1];
		r = src_bayer1[1];
		src_bayer0 += 2;
		src_bayer1 += 2;
		dst_argb += 8;
	}
	dst_argb[0] = src_bayer0[0];
	dst_argb[1] = AVG(g, src_bayer0[1]);
	dst_argb[2] = AVG(r, src_bayer1[1]);
	dst_argb[3] = 255U;
	if (!(pix & 1)) {
		dst_argb[4] = src_bayer0[0];
		dst_argb[5] = src_bayer0[1];
		dst_argb[6] = src_bayer1[1];
		dst_argb[7] = 255U;
	}
}

static void BayerRowRG(const uint8* src_bayer0, int src_stride_bayer,
	uint8* dst_argb, int pix) {
	const uint8* src_bayer1 = src_bayer0 + src_stride_bayer;
	uint8 g = src_bayer0[1];
	uint8 b = src_bayer1[1];

	int x;
	for (x = 0; x < pix - 2; x += 2) {

		dst_argb[0] = (AVG(b, src_bayer1[1]));
		dst_argb[1] = (AVG(g, src_bayer0[1]));
		dst_argb[2] = (src_bayer0[0]);
		dst_argb[3] = 255U;
		dst_argb[4] = (src_bayer1[1]);
		dst_argb[5] = (src_bayer0[1]);
		dst_argb[6] = (AVG(src_bayer0[0], src_bayer0[2]));
		dst_argb[7] = 255U;

		g = src_bayer0[1];
		b = src_bayer1[1];
		src_bayer0 += 2;
		src_bayer1 += 2;
		dst_argb += 8;

	}

	dst_argb[0] = (AVG(b, src_bayer1[1]));
	dst_argb[1] = (AVG(g, src_bayer0[1]));
	dst_argb[2] = (src_bayer0[0]);
	dst_argb[3] = 255U;

	if (!(pix & 1)) {
		dst_argb[4] = (src_bayer1[1]);
		dst_argb[5] = (src_bayer0[1]);
		dst_argb[6] = (src_bayer0[0]);
		dst_argb[7] = 255U;
	}
}

static void BayerRowGB(const uint8* src_bayer0, int src_stride_bayer,
	uint8* dst_argb, int pix) {
	const uint8* src_bayer1 = src_bayer0 + src_stride_bayer;
	uint8 b = src_bayer0[1];
	int x;

	for (x = 0; x < pix - 2; x += 2) {

		dst_argb[0] = (AVG(b, src_bayer0[1]));
		dst_argb[1] = (src_bayer0[0]);
		dst_argb[2] = (src_bayer1[0]);
		dst_argb[3] = 255U;
		dst_argb[4] = (src_bayer0[1]);
		dst_argb[5] = (AVG(src_bayer0[0], src_bayer0[2]));
		dst_argb[6] = (AVG(src_bayer1[0], src_bayer1[2]));
		dst_argb[7] = 255U;

		b = src_bayer0[1];
		src_bayer0 += 2;
		src_bayer1 += 2;
		dst_argb += 8;
	}

	dst_argb[0] = (AVG(b, src_bayer0[1]));
	dst_argb[1] = (src_bayer0[0]);
	dst_argb[2] = (src_bayer1[0]);
	dst_argb[3] = 255U;
	if (!(pix & 1)) {
		dst_argb[4] = (src_bayer0[1]);
		dst_argb[5] = (src_bayer0[0]);
		dst_argb[6] = (src_bayer1[0]);
		dst_argb[7] = 255U;
	}
}

static void BayerRowGR(const uint8* src_bayer0, int src_stride_bayer,
	uint8* dst_argb, int pix) {
	const uint8* src_bayer1 = src_bayer0 + src_stride_bayer;
	uint8 r = src_bayer0[1];
	int x;
	for (x = 0; x < pix - 2; x += 2) {
		dst_argb[0] = src_bayer1[0];
		dst_argb[1] = src_bayer0[0];
		dst_argb[2] = AVG(r, src_bayer0[1]);
		dst_argb[3] = 255U;
		dst_argb[4] = AVG(src_bayer1[0], src_bayer1[2]);
		dst_argb[5] = AVG(src_bayer0[0], src_bayer0[2]);
		dst_argb[6] = src_bayer0[1];
		dst_argb[7] = 255U;
		r = src_bayer0[1];
		src_bayer0 += 2;
		src_bayer1 += 2;
		dst_argb += 8;
	}
	dst_argb[0] = src_bayer1[0];
	dst_argb[1] = src_bayer0[0];
	dst_argb[2] = AVG(r, src_bayer0[1]);
	dst_argb[3] = 255U;
	if (!(pix & 1)) {
		dst_argb[4] = src_bayer1[0];
		dst_argb[5] = src_bayer0[0];
		dst_argb[6] = src_bayer0[1];
		dst_argb[7] = 255U;
	}
}

int BayerToI420(const uint8* src_bayer, int src_stride_bayer,
	uint8* dst_y, int dst_stride_y,
	uint8* dst_u, int dst_stride_u,
	uint8* dst_v, int dst_stride_v,
	int width, int height,
	uint32 src_fourcc_bayer) {
	void(*BayerRow0)(const uint8* src_bayer, int src_stride_bayer,
		uint8* dst_argb, int pix);
	void(*BayerRow1)(const uint8* src_bayer, int src_stride_bayer,
		uint8* dst_argb, int pix);

	void(*ARGBToUVRow)(const uint8* src_argb0, int src_stride_argb,
		uint8* dst_u, uint8* dst_v, int width) = libyuv::ARGBToUVRow_C;
	void(*ARGBToYRow)(const uint8* src_argb, uint8* dst_y, int pix) =
		libyuv::ARGBToYRow_C;
	// Negative height means invert the image.
	if (height < 0) {
		int halfheight;
		height = -height;
		halfheight = (height + 1) >> 1;
		dst_y = dst_y + (height - 1) * dst_stride_y;
		dst_u = dst_u + (halfheight - 1) * dst_stride_u;
		dst_v = dst_v + (halfheight - 1) * dst_stride_v;
		dst_stride_y = -dst_stride_y;
		dst_stride_u = -dst_stride_u;
		dst_stride_v = -dst_stride_v;
	}
#if defined(HAS_ARGBTOYROW_SSSE3) && defined(HAS_ARGBTOUVROW_SSSE3)
	if (libyuv::TestCpuFlag(libyuv::kCpuHasSSSE3)) {
		ARGBToUVRow = libyuv::ARGBToUVRow_Any_SSSE3;
		ARGBToYRow = libyuv::ARGBToYRow_Any_SSSE3;
		if (IS_ALIGNED(width, 16)) {
			ARGBToYRow = libyuv::ARGBToYRow_SSSE3;
			ARGBToUVRow = libyuv::ARGBToUVRow_SSSE3;
		}
	}
#endif
#if defined(HAS_ARGBTOYROW_NEON)
	if (TestCpuFlag(kCpuHasNEON)) {
		ARGBToYRow = ARGBToYRow_Any_NEON;
		if (IS_ALIGNED(width, 8)) {
			ARGBToYRow = ARGBToYRow_NEON;
		}
	}
#endif
#if defined(HAS_ARGBTOUVROW_NEON)
	if (TestCpuFlag(kCpuHasNEON)) {
		ARGBToUVRow = ARGBToUVRow_Any_NEON;
		if (IS_ALIGNED(width, 16)) {
			ARGBToUVRow = ARGBToUVRow_NEON;
		}
	}
#endif

	switch (src_fourcc_bayer) {
	case libyuv::FOURCC_BGGR:
		BayerRow0 = BayerRowBG;
		BayerRow1 = BayerRowGR;
		break;
	case libyuv::FOURCC_GBRG:
		BayerRow0 = BayerRowGB;
		BayerRow1 = BayerRowRG;
		break;
	case libyuv::FOURCC_GRBG:
		BayerRow0 = BayerRowGR;
		BayerRow1 = BayerRowBG;
		break;
	case libyuv::FOURCC_RGGB: //used
		BayerRow0 = BayerRowRG;
		BayerRow1 = BayerRowGB;
		break;
	default:
		return -1;  // Bad FourCC
	}

	{
		// Allocate 2 rows of ARGB.
		const int kRowSize = (width * 4 + 15) & ~15;
		align_buffer_64(row, kRowSize * 2);
		int y;
		for (y = 0; y < height - 1; y += 2) {
			BayerRow0(src_bayer, src_stride_bayer, row, width);
			BayerRow1(src_bayer + src_stride_bayer, -src_stride_bayer,
				row + kRowSize, width);

			

			ARGBToUVRow(row, kRowSize, dst_u, dst_v, width);
			ARGBToYRow(row, dst_y, width);
			ARGBToYRow(row + kRowSize, dst_y + dst_stride_y, width);
			src_bayer += src_stride_bayer * 2;
			dst_y += dst_stride_y * 2;
			dst_u += dst_stride_u;
			dst_v += dst_stride_v;
		}
		if (height & 1) {
			BayerRow0(src_bayer, src_stride_bayer, row, width);

			ARGBToUVRow(row, 0, dst_u, dst_v, width);
			ARGBToYRow(row, dst_y, width);
		}
		free_aligned_buffer_64(row);
	}
	return 0;
}


void ImageProcessor::bayerRGGBToI420(unsigned char* input, unsigned char* outputY, unsigned char* outputU, unsigned char* outputV, int width, int height) {
	int strideY = width;
	int strideU = (width + 1) / 2;
	int strideV = (width + 1) / 2;
	BayerToI420(
		input,
		width,
		outputY,
		strideY,
		outputU,
		strideU,
		outputV,
		strideV,
		width,
		height,
		libyuv::FOURCC_RGGB
	);
}


void ImageProcessor::I420ToYUYV(unsigned char* inputY, unsigned char* inputU, unsigned char* inputV, unsigned char* output, int width, int height) {
	int strideY = width;
	int strideU = (width + 1) / 2;
	int strideV = (width + 1) / 2;
	int outputStride = width * 2;

	libyuv::I420ToYUY2(
		inputY, strideY,
		inputU, strideU,
		inputV, strideV,
		output,
		outputStride,
		width, height
	);
}

void ImageProcessor::YUYVToARGB(unsigned char* input, unsigned char* output, int width, int height) {
	int inputStride = width * 2;
	int outputStride = width * 4;

	libyuv::YUY2ToARGB(
		input, inputStride,
		output, outputStride,
		width, height
	);
}

void ImageProcessor::ARGBToBGR(unsigned char* input, unsigned char* output, int width, int height) {
	int inputStride = width * 4;
	int outputStride = width * 3;

	libyuv::ARGBToRGB24(
		input, inputStride,
		output, outputStride,
		width, height
	);
}

void ImageProcessor::ARGBToRGB(unsigned char* input, unsigned char* output, int width, int height) {
	int inputStride = width * 4;
	int outputStride = width * 3;

	libyuv::ARGBToRAW(
		input, inputStride,
		output, outputStride,
		width, height
	);
}

bool ImageProcessor::rgbToJpeg(unsigned char* input, unsigned char* output, int& bufferSize, int width, int height, int channels) {
	return jpge::compress_image_to_jpeg_file_in_memory(output, bufferSize, width, height, channels, input);
}

ImageProcessor::YUYV* ImageProcessor::getYuyvPixelAt(unsigned char* dataY, unsigned char* dataU, unsigned char* dataV, int width, int height, int x, int y) {
	if (
		x < 0
		|| x > width - 1
		|| y < 0
		|| y > height - 1
		) {
		return NULL;
	}

	YUYV* pixel = new YUYV();

	int strideY = width;
	int strideU = (width + 1) / 2;
	int strideV = (width + 1) / 2;

	int yPos = strideY * y + x;
	int uvPos = strideU * (y / 2) + (x / 2);

	int stride = width * 1;

	pixel->y1 = dataY[yPos];
	pixel->u = dataU[uvPos];
	pixel->y2 = dataY[yPos];
	pixel->v = dataV[uvPos];

	return pixel;
}

ImageProcessor::YUYVRange ImageProcessor::extractColorRange(unsigned char* dataY, unsigned char* dataU, unsigned char* dataV, int imageWidth, int imageHeight, int centerX, int centerY, int brushRadius, float stdDev) {
	int Y, U, V;
	std::vector<float> yValues;
	std::vector<float> uValues;
	std::vector<float> vValues;

	for (int x = -brushRadius; x < brushRadius; x++) {
		int height = (int)::sqrt(brushRadius * brushRadius - x * x);

		for (int y = -height; y < height; y++) {
			if (
				x + centerX < 0
				|| x + centerX > imageWidth - 1
				|| y + centerY < 0
				|| y + centerY > imageHeight - 1
				) {
				continue;
			}

			YUYV* pixel = getYuyvPixelAt(dataY, dataU, dataV, imageWidth, imageHeight, x + centerX, y + centerY);

			if (pixel != NULL) {
				Y = (pixel->y1 + pixel->y2) / 2;
				U = pixel->u;
				V = pixel->v;

				delete pixel;

				yValues.push_back((float)Y);
				uValues.push_back((float)U);
				vValues.push_back((float)V);
			}
			else {
				std::cout << "- Didn't get pixel at " << (x + centerX) << "x" << (y + centerY) << std::endl;
			}
		}
	}

	float yMean, uMean, vMean;
	float yStdDev = Math::standardDeviation(yValues, yMean);
	float uStdDev = Math::standardDeviation(uValues, uMean);
	float vStdDev = Math::standardDeviation(vValues, vMean);

	YUYVRange range;

	range.minY = (int)(yMean - (float)yStdDev * stdDev);
	range.maxY = (int)(yMean + (float)yStdDev * stdDev);
	range.minU = (int)(uMean - (float)uStdDev * stdDev);
	range.maxU = (int)(uMean + (float)uStdDev * stdDev);
	range.minV = (int)(vMean - (float)vStdDev * stdDev);
	range.maxV = (int)(vMean + (float)vStdDev * stdDev);

	return range;
}

bool ImageProcessor::saveBitmap(unsigned char* data, std::string filename, int size) {
	try {
		std::ofstream file(filename, std::ios::binary);
		file.write((char*)data, size);

		return true;
	}
	catch (...) {
		return false;
	}
}

bool ImageProcessor::loadBitmap(std::string filename, unsigned char* buffer, int size) {
	try {
		std::ifstream file(filename, std::ios::in | std::ios::binary);
		file.read((char*)buffer, size);

		return true;
	}
	catch (...) {
		return false;
	}
}

bool ImageProcessor::saveJPEG(unsigned char* data, std::string filename, int width, int height, int channels) {
	return jpge::compress_image_to_jpeg_file(filename.c_str(), width, height, channels, data);
}