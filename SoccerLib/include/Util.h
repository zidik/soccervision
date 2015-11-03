#ifndef UTIL_H
#define UTIL_H

#include <Windows.h>
#include <string>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <iterator>

class Util {

public:
    static std::string base64Encode(const unsigned char* data, unsigned int len);
    static double millitime();
    static double duration(double start);
    static float signum(float value);
    static float limit(float num, float min, float max);
    static int strpos(const std::string& haystack, const std::string &needle);
    static bool replace(std::string& str, const std::string& from, const std::string& to);
	static void sleep(int milliseconds) { Sleep(milliseconds); }
	static __int64 timerStart();
	static double timerEnd(__int64 startTime = -1);
	//static void correctCameraPoint(int& x, int& y);
	static void confineField(float& x, float& y);
	static std::string json(std::string id, std::string payload);
	static std::vector<std::string> Util::getFilesInDir(std::string path);

    static inline int rgbToInt(int red, int green, int blue) {
        int rgb = red;
        rgb = (rgb << 8) + green;
        rgb = (rgb << 8) + blue;

        return rgb;
    }

    static inline void intToRgb(int rgb, int& red, int& green, int& blue) {
        red = (rgb >> 16) & 0xFF;
        green = (rgb >> 8) & 0xFF;
        blue = rgb & 0xFF;
    }

    template <class T>
    static inline std::string toString(const T& t) {
        std::stringstream ss;
        ss << t;

        return ss.str();
    }

    template <class T>
    static inline std::string toString(std::vector<T> vec) {
		if (vec.size() == 0) {
			return "[]";
		}

        std::stringstream ss;

        std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(ss, "\", \""));
        std::string result = ss.str();

        return "[\"" + result.substr(0, result.length() - 4) + "\"]";
    }

    static inline int toInt(const std::string str) {
        return atoi(str.c_str());
    }

    static inline float toFloat(const std::string str) {
        return (float)atof(str.c_str());
    }

	static inline double toDouble(const std::string str) {
        return (double)atof(str.c_str());
    }

	// these can be modified on the fly
	//static float cameraCorrectionK;
	//static float cameraCorrectionZoom;

private:
    static const std::string base64Chars;
	static double queryPerformanceFrequency;
	static __int64 timerStartCount;
	
};

#endif // UTIL_H
