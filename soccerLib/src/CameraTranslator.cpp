#include "CameraTranslator.h"
#include "Maths.h"

#include <iostream>

CameraTranslator::WorldPosition CameraTranslator::DEPRECATEDgetWorldPosition(int cameraX, int cameraY) {
	CameraPosition undistorted = undistort(cameraX, cameraY);

	//std::cout << "UNDISTORT " << cameraX << "x" << cameraY << " to " << undistorted.x << "x" << undistorted.y << std::endl;

	bool isValid = true;

	float pixelVerticalCoord = undistorted.y - this->horizon;
	int pixelRight = undistorted.x - this->cameraWidth / 2;

	float worldY = this->B + this->A / pixelVerticalCoord;
	float worldX = C * (float)pixelRight / pixelVerticalCoord;

	if (worldY < 0.0f || worldY > 30.0f) {
		isValid = false;
	}

	float worldDistance = sqrt(pow(worldX, 2) + pow(worldY, 2));
	float worldAngle = atan2(worldX, worldY);

	return WorldPosition(worldX, worldY, worldDistance, worldAngle, isValid);
}

Math::Vector CameraTranslator::getWorldPosition(CameraPosition cameraPosition)
{
	cameraPosition = undistort(cameraPosition.x, cameraPosition.y);

	float pixelVerticalCoord = cameraPosition.y - this->horizon;
	int pixelLeft = this->cameraWidth / 2 - cameraPosition.x;

	float worldX = this->B + this->A / pixelVerticalCoord;
	float worldY = C * (float)pixelLeft / pixelVerticalCoord;

	return Math::Vector(worldX, worldY);
}

CameraTranslator::CameraPosition CameraTranslator::DEPRECATEDgetCameraPosition(float worldX, float worldY) {
	float pixelVerticalCoord = this->A / (worldY - this->B);
	float pixelRight = worldX * pixelVerticalCoord / this->C;

	float cameraY = pixelVerticalCoord + this->horizon;
	float cameraX = pixelRight + this->cameraWidth / 2;

	return distort((int)Math::round(cameraX, 0), (int)Math::round(cameraY, 0));
}

CameraTranslator::CameraPosition CameraTranslator::getCameraPosition(Math::Vector worldPosition) {
	float pixelVerticalCoord = this->A / (worldPosition.x - this->B);
	float pixelLeft = worldPosition.y * pixelVerticalCoord / this->C;

	float cameraX = this->cameraWidth / 2 - pixelLeft;
	float cameraY = pixelVerticalCoord + this->horizon;

	return distort((int)Math::round(cameraX, 0), (int)Math::round(cameraY, 0));
}

void CameraTranslator::setConstants(
	float A, float B, float C,
	float k1, float k2, float k3,
	float horizon, float distortionFocus,
	int cameraWidth, int cameraHeight
) {
	this->A = A;
	this->B = B;
	this->C = C;
	this->k1 = k1;
	this->k2 = k2;
	this->k3 = k3;
	this->horizon = horizon;
	this->distortionFocus = distortionFocus;
	this->cameraWidth = cameraWidth;
	this->cameraHeight = cameraHeight;
}

CameraTranslator::CameraPosition CameraTranslator::getMappingPosition(int x, int y, CameraMap& mapX, CameraMap& mapY) {
	if (x < 0) x = 0;
	if (x > cameraWidth - 1) x = cameraWidth - 1;
	if (y < 0) y = 0;
	if (y > cameraHeight - 1) y = cameraHeight - 1;

	return CameraPosition(
		(int)mapX[y][x],
		(int)mapY[y][x]
	);
}

/*CameraTranslator::CameraPosition CameraTranslator::getAvgMappingPosition(int x, int y, CameraMap& mapX, CameraMap& mapY) {
	int brushSize = 10;
	int xSum = 0;
	int ySum = 0;
	int sampleCount = 0;

	for (int dx = -brushSize / 2; dx < brushSize / 2; dx++) {
		for (int dy = -brushSize / 2; dy < brushSize / 2; dy++) {
			CameraPosition sample = getMappingPosition(x + dx, y + dy, mapX, mapY);

			xSum += sample.x;
			ySum += sample.y;
			sampleCount++;
		}
	}

	return CameraPosition(
		xSum / sampleCount,
		ySum / sampleCount
	);
}*/

CameraTranslator::CameraPosition CameraTranslator::undistort(CameraPosition distorted) {
	return getMappingPosition(distorted.x, distorted.y, undistortMapX, undistortMapY);
	//return getAvgMappingPosition(distortedX, distortedY, undistortMapX, undistortMapY);
}

CameraTranslator::CameraPosition CameraTranslator::distort(CameraPosition undistorted) {
	return getMappingPosition(undistorted.x, undistorted.y, distortMapX, distortMapY);
}

// equasion-based solution
/*CameraTranslator::CameraPosition CameraTranslator::distort(int undistortedX, int undistortedY) {
	// conversion for distorting  (normalization?)
	float x = ((float)undistortedX - (float)cameraWidth / 2.0f) / distortionFocus;
	float y = ((float)undistortedY - (float)cameraHeight / 2.0f) / distortionFocus;

	// distort
	float r2 = x * x + y * y; // distance squared
	float multipler = 1 + 
		k1 *  r2 +
		k2 *  r2 * r2 + 
		k3 *  r2 * r2 * r2;

	x *= multipler;
	y *= multipler;

	// convert back
	int distortedX = (int)(x * distortionFocus + cameraWidth / 2.0f);
	int distortedY = (int)(y * distortionFocus + cameraHeight / 2.0f);

	return CameraPosition(
		distortedX,
		distortedY
	);
}*/

bool CameraTranslator::loadUndistortionMapping(std::string xFilename, std::string yFilename){
	return loadMapping(xFilename, yFilename, undistortMapX, undistortMapY);
}

bool CameraTranslator::loadDistortionMapping(std::string xFilename, std::string yFilename){
	return loadMapping(xFilename, yFilename, distortMapX, distortMapY);
}

bool CameraTranslator::loadMapping(std::string xFilename, std::string yFilename, CameraMap& mapX, CameraMap& mapY){
	std::ifstream fileStream;

	fileStream.open(xFilename);

	if (fileStream.is_open()) {
		fileStream >> mapX;

		if (!fileStream.eof()) {
			std::cout << "- Failed to load x map from " << xFilename << std::endl;

			return false;
		}
	}
	else {
		std::cout << "- Failed to open map file " << xFilename << std::endl;
	}

	fileStream.close();
	fileStream.open(yFilename);

	if (fileStream.is_open()) {
		fileStream >> mapY;

		if (!fileStream.eof()) {
			std::cout << "- Failed to load y map from " << yFilename << std::endl;

			return false;
		}
	}
	else {
		std::cout << "- Failed to open y map file " << yFilename << std::endl;
	}

	fileStream.close();

	return true;
}

std::istream& operator >> (std::istream& inputStream, CameraTranslator::CameraMap& map) {
	map.clear();
	CameraTranslator::CameraMapRow mapRow;

	std::string lineString;
	CameraTranslator::CameraMapItem field;

	while (getline(inputStream, lineString)) {
		mapRow.clear();

		std::stringstream lineStream(lineString);
		std::string fieldString;

		while (getline(lineStream, fieldString, ',')) {
			std::stringstream fieldStream(fieldString);

			fieldStream >> field;

			mapRow.push_back(field);
		}

		map.push_back(mapRow);
	}

	return inputStream;  
}

CameraTranslator::CameraMapSet CameraTranslator::generateInverseMap(CameraMap& mapX, CameraMap& mapY) {
	CameraMap inverseMapX;
	CameraMap inverseMapY;
	CameraMapItem NaN = INT_MAX;
	//CameraMapItem x, y;
	CameraTranslator::CameraMapRow mapRowX;
	CameraTranslator::CameraMapRow mapRowY;
	CameraPosition distorted;

	unsigned int rowCount = mapX.size();
	unsigned int colCount = mapX[0].size();

	for (unsigned int row = 0; row < rowCount; row++) {
		mapRowX.clear();
		mapRowY.clear();

		for (unsigned int col = 0; col < colCount; col++) {
			//mapRowX.push_back(col);
			//mapRowY.push_back(row);
			mapRowX.push_back(NaN);
			mapRowY.push_back(NaN);
		}

		inverseMapX.push_back(mapRowX);
		inverseMapY.push_back(mapRowY);
	}

	for (unsigned int row = 0; row < rowCount; row++) {
		for (unsigned int col = 0; col < colCount; col++) {
			//x = mapX[row][col];
			//y = mapY[row][col];

			//std::cout << "D " << col << "x" << row << ".. ";
			distorted = distort(CameraPosition(col, row));

			//std::cout << distorted.x << "x" << distorted.y << std::endl;

			if (distorted.y >= 0 && distorted.y < (int)rowCount && distorted.x >= 0 && distorted.x < (int)colCount) {
				inverseMapX[distorted.y][distorted.x] = col;
				inverseMapY[distorted.y][distorted.x] = row;
			}
		}
	}

	// fix NaN's
	CameraPositionSet spiralPositions = getSpiral(120, 120);
	CameraMapChangeSet mapChangeSet;
	int spiralPosCount = spiralPositions.size();
	int dx, dy, senseX, senseY;
	bool substituteFound;
	int nanCount = 0;
	int failCount = 0;

	for (unsigned int row = 0; row < rowCount; row++) {
		for (unsigned int col = 0; col < colCount; col++) {
			if (inverseMapX[row][col] != NaN && inverseMapY[row][col] != NaN) {
				continue;
			}

			nanCount++;

			substituteFound = false;

			for (int i = 0; i < spiralPosCount; i++) {
				CameraPosition spiralPos = spiralPositions[i];

				dx = spiralPos.x;
				dy = spiralPos.y;
				senseX = col + dx;
				senseY = row + dy;

				if (senseX < 0 || senseX >(int)colCount - 1 || senseY < 0 || senseY >(int)rowCount - 1) {
					continue;
				}

				if (inverseMapX[senseY][senseX] != NaN && inverseMapY[senseY][senseX] != NaN) {
					// store the changes in a vector and play it back later not to affect other NaN values from generated values
					mapChangeSet.push_back(CameraMapChange(row, col, inverseMapX[senseY][senseX], inverseMapY[senseY][senseX]));

					substituteFound = true;

					break;
				}
			}

			if (!substituteFound) {
				failCount++;
			}
		}
	}

	while (mapChangeSet.size() > 0) {
		CameraMapChange mapChange = mapChangeSet.back();

		inverseMapX[mapChange.row][mapChange.col] = mapChange.xVal;
		inverseMapY[mapChange.row][mapChange.col] = mapChange.yVal;

		mapChangeSet.pop_back();
	}

	std::cout << "there were " << nanCount << " invalid values, failed to get subtitite for " << failCount << " values.. ";

	return CameraMapSet(inverseMapX, inverseMapY);
}

CameraTranslator::CameraPositionSet CameraTranslator::getSpiral(int width, int height) {
	CameraTranslator::CameraPositionSet positions;

	int x, y, dx, dy;
	x = y = dx = 0;
	dy = -1;
	int t = std::max(width, height);
	int maxI = t*t;
	for (int i = 0; i < maxI; i++){
		if ((-width / 2 <= x) && (x <= width / 2) && (-height / 2 <= y) && (y <= height / 2)){
			positions.push_back(CameraPosition(x, y));
		}
		if ((x == y) || ((x < 0) && (x == -y)) || ((x > 0) && (x == 1 - y))){
			t = dx;
			dx = -dy;
			dy = t;
		}
		x += dx;
		y += dy;
	}

	return positions;
}

Math::PointList CameraTranslator::getPointsBetween(float x1, float y1, float x2, float y2, float step) {
	Math::PointList points;

	/*if (x1 < x2) {
		float tempX = x1;
		float tempY = y1;

		x1 = x2;
		y1 = y2;
		x2 = tempX;
		y2 = tempY;
	}*/

	float dx = x2 - x1;
	float dy = y2 - y1;
	float a = dy / dx;
	float stepX = step * Math::sqrt(1 / (1 + Math::pow(a, 2))) * Math::sign(dx);
	float stepY = a * stepX;
	float posX = x1, posY = y1;

	while (posX * Math::sign(stepX) < x2 * Math::sign(stepX)) {
		points.push_back(Math::Vector(posX, posY));

		posX += stepX;
		posY += stepY;
	}

	//std::cout << "@ getPointsBetween x1: " << x1 << ", y1: " << y1 << ", x2: " << x2 << ", y2: " << y2 << ", stepX: " << stepX << ", stepY: " << stepY << std::endl;

	return points;
}

std::string CameraTranslator::getJSON() {
	std::stringstream stream;

	stream << "{";

	stream << "\"A\": " << A << ",";
	stream << "\"B\": " << B << ",";
	stream << "\"C\": " << C << ",";
	stream << "\"k1\": " << k1 << ",";
	stream << "\"k2\": " << k2 << ",";
	stream << "\"k3\": " << k3 << ",";
	stream << "\"horizon\": " << horizon << ",";
	stream << "\"distortionFocus\": " << distortionFocus << "";

	stream << "}";

	return stream.str();
}