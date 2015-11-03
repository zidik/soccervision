#include "LookupTable.h"
#include "Maths.h"

#include <cstdio>
#include <iostream>

float LookupTable::getValue(float search) {
    float key;
    float lastKey = -1;
    float lastValue = -1;

    LookupMapIt finalIt = (map.end())--;

    for (LookupMapIt it = map.begin(); it != map.end(); it++) {
        key = it->first;

        if (key == search) {
            return it->second;
        }

        if (key >= search) {
            if (it == map.begin()) {
                if (it != finalIt) {
                    float v1 = it->second;
                    it++;
                    float v2 = it->second;
                    float diff = key - lastKey;
                    float share = (search - lastKey) / diff;
                    float result = share * v1 + v2 * (1.0f - share);

					/*std::cout << "------ 1 ---------" << std::endl;
					std::cout << "search: " << search << std::endl;
					std::cout << "v1: " << v1 << std::endl;
					std::cout << "v2: " << v2 << std::endl;
					std::cout << "key: " << key << std::endl;
					std::cout << "lastKey: " << lastKey << std::endl;
					std::cout << "diff: " << diff << std::endl;
					std::cout << "share: " << share << std::endl;
					std::cout << "result: " << result << std::endl;*/

					return result;
                } else {
                    return it->second;
                }
            }

            float v1 = it->second;
            float v2 = lastValue;
            float diff = key - lastKey;
            float share = (search - lastKey) / diff;
            float result = share * v1 + v2 * (1.0f - share);

            /*std::cout << "------ 2 ---------" << std::endl;
            std::cout << "search: " << search << std::endl;
            std::cout << "v1: " << v1 << std::endl;
            std::cout << "v2: " << v2 << std::endl;
            std::cout << "key: " << key << std::endl;
            std::cout << "lastKey: " << lastKey << std::endl;
            std::cout << "diff: " << diff << std::endl;
            std::cout << "share: " << share << std::endl;
            std::cout << "result: " << result << std::endl;*/

            return result;
        }

        lastKey = it->first;
        lastValue = it->second;
    }

    return lastValue;
}

float LookupTable::getInverseValue(float search) {
    float key, value;
    float lastKey = -1.0f;
    float lastValue = -1.0f;

	//std::cout << "@ Search inverse for " << search << std::endl;

    for (LookupMapIt it = map.begin(); it != map.end(); it++) {
        key = it->first;
        value = it->second;

        if (value == search) {
			//std::cout << "  > Found exact match: " << key << std::endl;

            return key;
        }

		if (value < search) {
			if (lastValue == -1.0f) {
				return key;
			}

			float value1 = value;
			float value2 = lastValue;
			float valueDiff = Math::abs(value1 - value2);
			float key1 = key;
			float key2 = lastKey;
			float keyDiff = Math::abs(key1 - key2);
			float share = (lastValue - search) / valueDiff;
			float result = lastKey * share + key * (1.0f - share);

			/*std::cout << "  > value1: " << value1 << std::endl;
			std::cout << "  > value2: " << value2 << std::endl;
			std::cout << "  > valueDiff: " << valueDiff << std::endl;
			std::cout << "  > key1: " << key1 << std::endl;
			std::cout << "  > key2: " << key2 << std::endl;
			std::cout << "  > keyDiff: " << keyDiff << std::endl;
			std::cout << "  > share: " << share << std::endl;
			std::cout << "  > result: " << result << std::endl << std::endl;*/

			return result;
			
		}

        /*if (key >= search) {
            if (it == map.begin()) {
                if (it != finalIt) {
                    float v1 = it->second;
                    it++;
                    float v2 = it->second;
                    float diff = key - lastKey;
                    float share = (search - lastKey) / diff;
                    float result = share * v1 + v2 * (1.0f - share);

					/std::cout << "------ 1 ---------" << std::endl;
					std::cout << "search: " << search << std::endl;
					std::cout << "v1: " << v1 << std::endl;
					std::cout << "v2: " << v2 << std::endl;
					std::cout << "key: " << key << std::endl;
					std::cout << "lastKey: " << lastKey << std::endl;
					std::cout << "diff: " << diff << std::endl;
					std::cout << "share: " << share << std::endl;
					std::cout << "result: " << result << std::endl;/

					return result;
                } else {
                    return it->second;
                }
            }

            float v1 = it->second;
            float v2 = lastValue;
            float diff = key - lastKey;
            float share = (search - lastKey) / diff;
            float result = share * v1 + v2 * (1.0f - share);

            /std::cout << "------ 2 ---------" << std::endl;
            std::cout << "search: " << search << std::endl;
            std::cout << "v1: " << v1 << std::endl;
            std::cout << "v2: " << v2 << std::endl;
            std::cout << "key: " << key << std::endl;
            std::cout << "lastKey: " << lastKey << std::endl;
            std::cout << "diff: " << diff << std::endl;
            std::cout << "share: " << share << std::endl;
            std::cout << "result: " << result << std::endl;/

            return result;
        }*/

        lastKey = it->first;
        lastValue = it->second;
    }

	//std::cout << "  > No match, returning " << lastKey << std::endl << std::endl;

    return lastKey;
}

bool LookupTable::load(std::string filename, float valueDiff) {
    FILE* in;

    in = fopen(filename.c_str(), "rt");

    if (!in) {
        return(false);
    }

    const int lineBufferSize = 256;
    char buf[lineBufferSize];
    float key;
    float value;

    while (fgets(buf, lineBufferSize, in)) {
        if (sscanf(buf, "%f %f", &key, &value)) {
            addValue(key, value + valueDiff);
        }
    };

	fclose(in);

    return true;
}
