#ifndef CAMERATRANSLATOR_H
#define CAMERATRANSLATOR_H

#include "Maths.h"

#include <math.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

class CameraTranslator {

public:
	//typedef float CameraMapItem;
	typedef int CameraMapItem;
	typedef std::vector <CameraMapItem> CameraMapRow;
	typedef std::vector <CameraMapRow> CameraMap;
	
	struct CameraMapSet {
		CameraMapSet(CameraMap x, CameraMap y) : x(x), y(y) {}

		CameraMap x;
		CameraMap y;
	};

	struct CameraMapChange {
		CameraMapChange(int row, int col, CameraMapItem xVal, CameraMapItem yVal) : row(row), col(col), xVal(xVal), yVal(yVal) {}

		int row;
		int col;
		CameraMapItem xVal;
		CameraMapItem yVal;
	};

	typedef std::vector <CameraMapChange> CameraMapChangeSet;

	struct WorldPosition {
		WorldPosition() : dx(0.0f), dy(0.0f), distance(0.0f), angle(0.0f), isValid(false) {}
		WorldPosition(float dx, float dy, float distance, float angle, bool isValid = true) : dx(dx), dy(dy), distance(distance), angle(angle), isValid(isValid) {}

		float dx;
		float dy;
		float distance;
		float angle;
		bool isValid;
	};

	struct CameraPosition {
		CameraPosition() : x(0), y(0) {}
		CameraPosition(int x, int y) : x(x), y(y) {}

		int x;
		int y;
	};

	typedef std::vector <CameraPosition> CameraPositionSet;

	CameraTranslator() : A(0.0f), B(0.0f), C(0.0f), k1(0.0f), k2(0.0f), k3(0.0f), horizon(0.0f), cameraWidth(0), cameraHeight(0) {}

	void setConstants(
		float A, float B, float C,
		float k1, float k2, float k3,
		float horizon, float distortionFocus,
		int cameraWidth, int cameraHeight);

	bool loadMapping(std::string xFilename, std::string yFilename, CameraMap& mapX, CameraMap& mapY);
	bool loadUndistortionMapping(std::string xFilename, std::string yFilename);
	bool loadDistortionMapping(std::string xFilename, std::string yFilename);
	CameraMapSet generateInverseMap(CameraMap& mapX, CameraMap& mapY);
	WorldPosition getWorldPosition(int cameraX, int cameraY);
	CameraPosition getCameraPosition(float dx, float dy);
	CameraPosition undistort(int x, int y);
	CameraPosition distort(int x, int y);
	CameraPosition getMappingPosition(int x, int y, CameraMap& mapX, CameraMap& mapY);
	//CameraPosition getAvgMappingPosition(int x, int y, CameraMap& mapX, CameraMap& mapY);
	CameraPositionSet CameraTranslator::getSpiral(int width, int height);
	Math::PointList getPointsBetween(float x1, float y1, float x2, float y2, float distanceStepMeters);
	std::string getJSON();

	float A;
	float B;
	float C;
	float k1;
	float k2;
	float k3;
	float horizon;
	float distortionFocus;

	CameraMap undistortMapX;
	CameraMap undistortMapY;
	CameraMap distortMapX;
	CameraMap distortMapY;

private:
	friend std::istream& operator >> (std::istream& inputStream, CameraMap& map);

	int cameraWidth;
	int cameraHeight;
};

#endif