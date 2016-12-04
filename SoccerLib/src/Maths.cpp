#include "Maths.h"

#include <iostream>
#include <sstream>

namespace Math {

// 3x3 Matrix
Matrix3x3::Matrix3x3(
    float a11, float a12, float a13,
    float a21, float a22, float a23,
    float a31, float a32, float a33
) : a11(a11), a12(a12), a13(a13),
    a21(a21), a22(a22), a23(a23),
    a31(a31), a32(a32), a33(a33) {}

Matrix3x3::Matrix3x3() :
    a11(0), a12(0), a13(0),
    a21(0), a22(0), a23(0),
    a31(0), a32(0), a33(0) {}

float Matrix3x3::getDeterminant() const {
	return a11 * (a33 * a22 - a32 * a23)
		- a21 * (a33 * a12 - a32 * a13)
		+ a31 * (a23 * a12 - a22 * a13);
}

const Matrix3x3 Matrix3x3::getMultiplied(float scalar) const {
    return Matrix3x3(
        a11 * scalar, a12 * scalar, a13 * scalar,
        a21 * scalar, a22 * scalar, a23 * scalar,
        a31 * scalar, a32 * scalar, a33 * scalar
    );
}

const Matrix3x3 Matrix3x3::getMultiplied(const Matrix3x3& b) const {
    return Matrix3x3(
        a11 * b.a11 + a12 * b.a21 + a13 * b.a31, a11 * b.a12 + a12 * b.a22 + a13 * b.a32, a11 * b.a13 + a12 * b.a23 + a13 * b.a33,
        a21 * b.a11 + a22 * b.a21 + a23 * b.a31, a21 * b.a12 + a22 * b.a22 + a23 * b.a32, a21 * b.a13 + a22 * b.a23 + a23 * b.a33,
        a31 * b.a11 + a32 * b.a21 + a33 * b.a31, a31 * b.a12 + a32 * b.a22 + a33 * b.a32, a31 * b.a13 + a32 * b.a23 + a33 * b.a33
    );
}

const Matrix3x1 Matrix3x3::getMultiplied(const Matrix3x1& b) const {
    return Matrix3x1(
        a11 * b.a11 + a12 * b.a21 + a13 * b.a31,
        a21 * b.a11 + a22 * b.a21 + a23 * b.a31,
        a31 * b.a11 + a32 * b.a21 + a33 * b.a31
    );
}

const Matrix3x3 Matrix3x3::getInversed() const {
    float d = getDeterminant();

    return Matrix3x3(
        (a33 * a22 - a32 * a23) / d, -(a33 * a12 - a32 * a13) / d, (a23 * a12 - a22 * a13) / d,
        -(a33 * a21 - a31 * a23) / d, (a33 * a11 - a31 * a13) / d, -(a23 * a11 - a21 * a13) / d,
        (a32 * a21 - a31 * a22) / d, -(a32 * a11 - a31 * a12) / d, (a22 * a11 - a21 * a12) / d
    );
}


// 3x1 Matrix
Matrix3x1::Matrix3x1(
    float a11,
    float a21,
    float a31
) : a11(a11),
    a21(a21),
    a31(a31) {}

const Matrix3x1 Matrix3x1::getMultiplied(float scalar) const {
    return Matrix3x1(
        a11 * scalar,
        a21 * scalar,
        a31 * scalar
    );
}


// 4x3 Matrix
Matrix4x3::Matrix4x3(
    float a11, float a12, float a13,
    float a21, float a22, float a23,
    float a31, float a32, float a33,
    float a41, float a42, float a43
) : a11(a11), a12(a12), a13(a13),
    a21(a21), a22(a22), a23(a23),
    a31(a31), a32(a32), a33(a33),
    a41(a41), a42(a42), a43(a43) {}

Matrix4x3::Matrix4x3() :
    a11(0), a12(0), a13(0),
    a21(0), a22(0), a23(0),
    a31(0), a32(0), a33(0),
    a41(0), a42(0), a43(0) {}

const Matrix4x3 Matrix4x3::getMultiplied(float scalar) const {
    return Matrix4x3(
        a11 * scalar, a12 * scalar, a13 * scalar,
        a21 * scalar, a22 * scalar, a23 * scalar,
        a31 * scalar, a32 * scalar, a33 * scalar,
        a41 * scalar, a42 * scalar, a43 * scalar
    );
}

const Matrix4x1 Matrix4x3::getMultiplied(const Matrix3x1& b) const {
    return Matrix4x1(
        a11 * b.a11 + a12 * b.a21 + a13 * b.a31,
        a21 * b.a11 + a22 * b.a21 + a23 * b.a31,
        a31 * b.a11 + a32 * b.a21 + a33 * b.a31,
        a41 * b.a11 + a42 * b.a21 + a43 * b.a31
    );
}


// 4x1 Matrix
Matrix4x1::Matrix4x1(
    float a11,
    float a21,
    float a31,
    float a41
) : a11(a11),
    a21(a21),
    a31(a31),
    a41(a41) {}


// 2D Vector
Vector Vector::fromPolar(float dir, float magnitude) {
    return Vector(
        Math::cos(dir) * magnitude,
        Math::sin(dir) * magnitude
    );
}

float Vector::getLength() const {
    return sqrt(pow(x, 2) + pow(y, 2));
}

float Vector::getAngle() const
{
    return atan2(y, x);
}

float Vector::distanceTo(const Vector& b) const {
	return sqrt(pow(x - b.x, 2) + pow(y - b.y, 2));
}

float Vector::dotProduct(const Vector& b) const {
    return x * b.x + y * b.y;
}

float Vector::getAngleBetween(const Vector& b) const {
    return acosf(dotProduct(b) / (getLength() * b.getLength()));
}

Vector Vector::getRotated(float angle) const {
    return Vector(
        x * Math::cos(angle) - y * Math::sin(angle),
        x * Math::sin(angle) + y * Math::cos(angle)
    );
}

Vector Vector::getScaledTo(float target_length) const {
    float length = getLength();
    if (length == 0.0f) { return Math::Vector(0.0f, 0.0f); } //Can't scale 0-vector
    return (*this) * (target_length/length);
}

Vector& Vector::operator+=(const Vector& other) {
	x += other.x;
	y += other.y;
	return *this;
}
Vector& Vector::operator-=(const Vector& other) {
	x -= other.x;
	y -= other.y;
	return *this;
}
Vector& Vector::operator*=(float magnitude) {
	x *= magnitude;
	y *= magnitude;
	return *this;
}
Vector& Vector::operator/=(float magnitude) {
	x /= magnitude;
	y /= magnitude;
	return *this;
}

Rectangle::Rectangle() {}

Rectangle::Rectangle(Vector upperLeft, Vector lowerRight) {
	this->upperLeft = upperLeft;
	this->lowerRight = lowerRight;
}

bool Rectangle::containsPoint(Vector point)
{
	return ((point.x >= upperLeft.x) && (point.x <= lowerRight.x) && (point.y >= lowerRight.y) && (point.y <= upperLeft.y));
}

// 2D Polygon
Polygon::Polygon() {}

Polygon::Polygon(const PointList& points) {
    this->points = points;
}

void Polygon::addPoint(float x, float y) {
    points.push_back(Vector(x, y));
}

void Polygon::addPoint(Vector point) {
    points.push_back(point);
}

bool Polygon::containsPoint(float x, float y) const {
    bool c = false;
    int i = -1;

    for(int l = points.size(), j = l - 1; ++i < l; j = i) {
		((points[i].y <= y && y < points[j].y) || (points[j].y <= y && y < points[i].y))
		&& (x < (points[j].x - points[i].x) * (y - points[i].y) / (points[j].y - points[i].y) + points[i].x)
		&& (c = !c);
	}

	return c;
}

Polygon Polygon::getTranslated(float dx, float dy) const {
    Polygon translated;

    for(unsigned int i = 0; i < points.size(); i++) {
        translated.addPoint(points[i].x + dx, points[i].y + dy);
	}

    return translated;
}

Polygon Polygon::getScaled(float sx, float sy) const {
    Polygon scaled;

    for(unsigned int i = 0; i < points.size(); i++) {
        scaled.addPoint(points[i].x * sx, points[i].y * sy);
	}

    return scaled;
}

Polygon Polygon::getRotated(float angle) const {
	Polygon rotated;

	for(unsigned int i = 0; i < points.size(); i++) {
		rotated.addPoint(points[i].getRotated(angle));
	}

	return rotated;
}

std::string Polygon::toJSON() {
    std::stringstream stream;

	stream << "[";

	Vector point;
	bool first = true;

	for (PointListIt it = points.begin(); it != points.end(); it++) {
		point = *it;

		if (!first) {
            stream << ",";
        } else {
            first = false;
        }

		stream << "{";
		stream << "\"x\":" << point.x << ",";
		stream << "\"y\":" << point.y;
		stream << "}";
	}

	stream << "]";

	return stream.str();
}

Circle::Intersections Circle::getIntersections(const Circle& other) {
	Intersections intersections = Intersections();

	float distanceSquared = Math::pow(x - other.x, 2.0f) + Math::pow(y - other.y, 2.0f);
	float distance = Math::sqrt(distanceSquared);
	
	if (
		distance > radius + other.radius
		|| distance < Math::abs(radius - other.radius)
		|| distance == 0.0f
	) {
		intersections.exist = false;

		return intersections;
	}

	intersections.exist = true;
	
	float a = (Math::pow(radius, 2.0f) - Math::pow(other.radius, 2.0f) + distanceSquared) / (2.0f * distance);
	float d = Math::pow(radius, 2.0f) - Math::pow(a, 2.0f);

	if (d >= 0) {
		float h = Math::sqrt(d);
		float centerX = x + a * (other.x - x) / distance;
		float centerY = y + a * (other.y - y) / distance;
	
		intersections.x1 = centerX + h * (other.y - y) / distance,
		intersections.y1 = centerY + h * (other.x - x) / distance,
		intersections.x2 = centerX - h * (other.y - y) / distance,
		intersections.y2 = centerY - h * (other.x - x) / distance;
	} else {
		intersections.exist = false;

		return intersections;
	}

	//std::cout << "! INT " << intersections.x1 << "x" << intersections.y1 << ", " << intersections.x2 << "x" << intersections.y2 << std::endl;
	//std::cout << "  > distance: " << distance << ", a: " << a << ", h: " << h << ", centerX: " << centerX << ", centerY: " << centerY << std::endl;
		
	return intersections;
};

void Avg::add(float sample) {
	samples.push_back(sample);

	if ((int)samples.size() > sampleCount) {
		samples.erase(samples.begin(), samples.begin() + 1);
	}
}

float Avg::value() {
	float total = 0;

	for  (std::vector<float>::const_iterator it = samples.begin(); it != samples.end(); it++) {
		total += *it;
	}

	return total / samples.size();
}

} // namespace Math
