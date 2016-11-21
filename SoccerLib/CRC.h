#ifndef CRC_H
#define CRC_H
#include <cstdint>
#include <string>

#define WIDTH  (8 * sizeof(crc))
#define TOPBIT (1 << (WIDTH - 1))

class CRC
{	
public:
	//Change this to change crc length
	typedef uint8_t crc;

	void init();
	crc calculateCRC(uint8_t message[], int nBytes);
	unsigned int calclulateCRC(std::string message);
	CRC() = default;
	CRC(const CRC&) = default;
	CRC& operator=(CRC&) = default;


private:
	crc crcTable[256];
	const unsigned int POLYNOMIAL = 0xD8;

};
#endif

