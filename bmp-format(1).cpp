#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>

struct FXPT2DOT30
{
	uint32_t data;
	FXPT2DOT30(uint32_t &&d) : data(d) {}
	constexpr operator long double()
	{
		long double res = data >> 30;
		res += static_cast<long double>(data & 0x7fffffff) / 0x80000000;
		return res;
	}
	constexpr operator double()
	{
		return static_cast<double>(this->operator long double());
	}
};

struct RGBAquad
{
	uint8_t r, g, b, a;
};
class RGBAfactory
{
	const uint32_t &maskRed, &maskGreen, &maskBlue, maskAlpha;
	uint8_t shiftRed, shiftGreen, shiftBlue, shiftAlpha;
public:
	RGBAfactory(const uint32_t &r, const uint32_t &g, const uint32_t &b, const uint32_t &a)
	: maskRed(r), maskGreen(g), maskBlue(b), maskAlpha(a)
	{
		shiftRed=0;
		auto tempRed = ~maskRed;
		while(tempRed & 1)
		{
			++shiftRed;
			tempRed >>= 1;
		}

		shiftGreen=0;
		auto tempGreen = ~maskGreen;
		while(tempGreen & 1)
		{
			++shiftGreen;
			tempGreen >>= 1;
		}

		shiftBlue=0;
		auto tempBlue = ~maskBlue;
		while(tempBlue & 1)
		{
			++shiftBlue;
			tempBlue >>= 1;
		}

		shiftAlpha=0;
		auto tempAlpha = ~maskAlpha;
		while(tempAlpha & 1)
		{
			++shiftAlpha;
			tempAlpha >>= 1;
		}
	}
	void makeRGBAquad(RGBAquad &res, uint32_t data)
	{
		res.r = (data & maskRed  ) >> shiftRed;
		res.g = (data & maskGreen) >> shiftGreen;
		res.b = (data & maskBlue ) >> shiftBlue;
		res.a = (data & maskAlpha) >> shiftAlpha;
	}
	uint32_t saveRGBAquad(const RGBAquad &pixel)
	{
		return
			( static_cast<uint32_t>(pixel.r) << shiftRed   ) |
			( static_cast<uint32_t>(pixel.g) << shiftGreen ) |
			( static_cast<uint32_t>(pixel.b) << shiftBlue  ) |
			( static_cast<uint32_t>(pixel.a) << shiftAlpha );
	}
};

using RGBAdata = std::vector<std::vector<RGBAquad>>;

bool finish(const char* message)
{
	std::cerr << message << std::endl;
	exit(0);
}

constexpr uint16_t get16bit(char buffer[])
{
	return 
		static_cast<uint16_t>(static_cast<uint8_t>(buffer[0]))      |
		static_cast<uint16_t>(static_cast<uint8_t>(buffer[1])) <<  8;
}
constexpr uint32_t get32bit(char buffer[])
{
	return 
		static_cast<uint32_t>(static_cast<uint8_t>(buffer[0]))        |
		static_cast<uint32_t>(static_cast<uint8_t>(buffer[1])) <<  8  |
		static_cast<uint32_t>(static_cast<uint8_t>(buffer[2])) <<  16 |
		static_cast<uint32_t>(static_cast<uint8_t>(buffer[3])) <<  24;
}
void set16bit(uint16_t data, char buffer[])
{
	buffer[0] = static_cast<char>(data);
	buffer[1] = static_cast<char>(data >> 8);
}
void set32bit(uint32_t data, char buffer[])
{
	buffer[0] = static_cast<char>(data);
	buffer[1] = static_cast<char>(data >> 8);
	buffer[2] = static_cast<char>(data >> 16);
	buffer[3] = static_cast<char>(data >> 24);
}

RGBAdata applyBlurMatrix(const RGBAdata &data, int8_t mat[3][3])
{
	RGBAdata res = data;
	
	int8_t total = 0;
	for(size_t i=0; i<3; ++i) for(size_t j=0; j<3; ++j) total+=mat[i][j];
	
	for(size_t row=1, endRow=data.size()-1; row<endRow; ++row)
	{
		for(size_t col=1, endCol=data[row].size()-1; col<endCol; ++col)
		{
			res[row][col].r = 
				(
					static_cast<int16_t>(data[row+1][col-1].r) * mat[0][0] +
					static_cast<int16_t>(data[row+1][col  ].r) * mat[0][1] +
					static_cast<int16_t>(data[row+1][col+1].r) * mat[0][2] +
					static_cast<int16_t>(data[row  ][col-1].r) * mat[0][0] +
					static_cast<int16_t>(data[row  ][col  ].r) * mat[0][1] +
					static_cast<int16_t>(data[row  ][col+1].r) * mat[0][2] +
					static_cast<int16_t>(data[row-1][col-1].r) * mat[0][0] +
					static_cast<int16_t>(data[row-1][col  ].r) * mat[0][1] +
					static_cast<int16_t>(data[row-1][col+1].r) * mat[0][2]
				) / total;
			res[row][col].g = 
				(
					static_cast<int16_t>(data[row+1][col-1].g) * mat[0][0] +
					static_cast<int16_t>(data[row+1][col  ].g) * mat[0][1] +
					static_cast<int16_t>(data[row+1][col+1].g) * mat[0][2] +
					static_cast<int16_t>(data[row  ][col-1].g) * mat[0][0] +
					static_cast<int16_t>(data[row  ][col  ].g) * mat[0][1] +
					static_cast<int16_t>(data[row  ][col+1].g) * mat[0][2] +
					static_cast<int16_t>(data[row-1][col-1].g) * mat[0][0] +
					static_cast<int16_t>(data[row-1][col  ].g) * mat[0][1] +
					static_cast<int16_t>(data[row-1][col+1].g) * mat[0][2]
				) / total;
			res[row][col].b = 
				(
					static_cast<int16_t>(data[row+1][col-1].b) * mat[0][0] +
					static_cast<int16_t>(data[row+1][col  ].b) * mat[0][1] +
					static_cast<int16_t>(data[row+1][col+1].b) * mat[0][2] +
					static_cast<int16_t>(data[row  ][col-1].b) * mat[0][0] +
					static_cast<int16_t>(data[row  ][col  ].b) * mat[0][1] +
					static_cast<int16_t>(data[row  ][col+1].b) * mat[0][2] +
					static_cast<int16_t>(data[row-1][col-1].b) * mat[0][0] +
					static_cast<int16_t>(data[row-1][col  ].b) * mat[0][1] +
					static_cast<int16_t>(data[row-1][col+1].b) * mat[0][2]
				) / total;
			res[row][col].a = 
				(
					static_cast<int16_t>(data[row+1][col-1].a) * mat[0][0] +
					static_cast<int16_t>(data[row+1][col  ].a) * mat[0][1] +
					static_cast<int16_t>(data[row+1][col+1].a) * mat[0][2] +
					static_cast<int16_t>(data[row  ][col-1].a) * mat[0][0] +
					static_cast<int16_t>(data[row  ][col  ].a) * mat[0][1] +
					static_cast<int16_t>(data[row  ][col+1].a) * mat[0][2] +
					static_cast<int16_t>(data[row-1][col-1].a) * mat[0][0] +
					static_cast<int16_t>(data[row-1][col  ].a) * mat[0][1] +
					static_cast<int16_t>(data[row-1][col+1].a) * mat[0][2]
				) / total;
		}
	}
	return res;
}

int main(int argc, char* argv[])
{
	std::ifstream fin("in.bmp", std::ios::binary);
	
	char buffer[4];
	
	// ***** BITMAP HEADER
	
	// offset 0, size 2 - BM
	fin.read(buffer, 2) || finish("cannot read");
	(buffer[0]=='B' && buffer[1]=='M') || finish("not a BMP");
	
	// offset 2, size 4 - size of file
	fin.read(buffer, 4) || finish("cannot read");
	const size_t fileSize = get32bit(buffer);
	std::cout << "The size of the file is " << fileSize << " bytes" << std::endl;
	
	// offset 6, size 4 - reserved (ignoring)
	fin.read(buffer, 4) || finish("cannot read");

	// offset 10, size 4 - location of data
	fin.read(buffer, 4) || finish("cannot read");
	const auto dataOffset = get32bit(buffer);
	std::cout << "Data is fout at offset " << dataOffset << std::endl;
	
	// ***** DIB HEADER
	// http://www.vsokovikov.narod.ru/New_MSDN_API/Bitmaps/str_bitmapv5header.htm
	// https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv5header
	// https://stackoverflow.com/questions/20864752/how-is-defined-the-data-type-fxpt2dot30-in-the-bmp-file-structure
	
	// offset 14, size 4 - size of DIB header
	fin.read(buffer, 4) || finish("cannot read");
	const auto dibHeaderSize = get32bit(buffer);
	std::cout << "DIB header size is " << dibHeaderSize << std::endl;
	
	// offset 18, size 4 - pixel width
	fin.read(buffer, 4) || finish("cannot read");
	const auto pixelWidth = get32bit(buffer);
	std::cout << "Width in pixels is " << pixelWidth << std::endl;
	
	// offset 22, size 4 - pixel height
	fin.read(buffer, 4) || finish("cannot read");
	auto const pixelHeight = get32bit(buffer);
	std::cout << "Height in pixels is " << pixelHeight << std::endl;
	
	// offset 26, size 2 - colour planes ?
	fin.read(buffer, 2) || finish("cannot read");
	auto const colourPlanes = get16bit(buffer); // non-const
	(colourPlanes == 1) || finish("colour planes must be 1");
	
	// offset 28, size 2 - bits per pixel
	fin.read(buffer, 2) || finish("cannot read");
	auto const bitsPerPixel = get16bit(buffer); // non-const
	std::cout << "Bits per pixel is " << bitsPerPixel << std::endl;
	
	// offset 30, size 4 - compression type
	fin.read(buffer, 4) || finish("cannot read");
	auto const compressionType = get32bit(buffer);
	(compressionType == 0) || finish("only compression 0 is supported");
	std::cout << "The type of compression is " << compressionType << std::endl;
	
	// offset 34, size 4 - image size
	fin.read(buffer, 4) || finish("cannot read");
	auto const imageSize = get32bit(buffer);
	std::cout << "Image size is " << imageSize << std::endl;
	
	// offset 38, size 4 - horizontal resolution
	fin.read(buffer, 4) || finish("cannot read");
	auto const horizontalRes = get32bit(buffer);
	std::cout << "Horizontal resolution is " << horizontalRes << std::endl;
	
	// offset 42, size 4 - horizontal resolution
	fin.read(buffer, 4) || finish("cannot read");
	auto const verticalRes = get32bit(buffer);
	std::cout << "Vertical resolution is " << verticalRes << std::endl;
	
	// offset 46, size 4 - number of colours
	fin.read(buffer, 4) || finish("cannot read");
	auto const numOfColours = std::max<uint32_t>(get32bit(buffer), static_cast<uint32_t>(1)<<bitsPerPixel);
	std::cout << "Number of colours is " << numOfColours << std::endl;
	
	// offset 50, size 4 - number of important colours
	fin.read(buffer, 4) || finish("cannot read");
	auto const numOfImportantColours = get32bit(buffer);
	std::cout << "Number of important colours is " << numOfImportantColours << std::endl;
	
	// DIB: COLOUR PROFILE
	
	// offset 54, size 4 - red bitmask
	fin.read(buffer, 4) || finish("cannot read");
	auto const maskRed = get32bit(buffer);
	std::cout << "Red mask is   " << std::hex << std::setw(8) << std::setfill('0') << maskRed << std::endl;
	
	// offset 58, size 4 - green bitmask
	fin.read(buffer, 4) || finish("cannot read");
	auto const maskGreen = get32bit(buffer);
	std::cout << "Green mask is " << std::hex << std::setw(8) << std::setfill('0') << maskGreen << std::endl;
	
	// offset 62, size 4 - blue bitmask
	fin.read(buffer, 4) || finish("cannot read");
	auto const maskBlue = get32bit(buffer);
	std::cout << "Blue mask is  " << std::hex << std::setw(8) << std::setfill('0') << maskBlue << std::endl;
	
	// offset 66, size 4 - alpha bitmask
	fin.read(buffer, 4) || finish("cannot read");
	auto const maskAlpha = get32bit(buffer);
	std::cout << "Alpha mask is  " << std::hex << std::setw(8) << std::setfill('0') << maskAlpha << std::endl;
	
	// offset 70, size 4 - colour space
	fin.read(buffer, 4) || finish("cannot read");
	auto const colourSpace = get32bit(buffer);
	std::cout << "Colour space is " << colourSpace << ' ' << (char)buffer[3] << (char)buffer[2] << (char)buffer[1] << (char)buffer[0] << std::endl;
	(colourSpace == 0x73524742) || finish("only sRBG colour space is supported");
	std::cout << std::dec;
	
	// offset 74, size 4 - X coordinate of red endpoint
	fin.read(buffer, 4) || finish("cannot read");
	auto const endpRedX = FXPT2DOT30( get32bit(buffer) );
	std::cout << "X coordinate of red endpoint is   " << (double)endpRedX << std::endl;
	
	// offset 78, size 4 - Y coordinate of red endpoint
	fin.read(buffer, 4) || finish("cannot read");
	auto const endpRedY = FXPT2DOT30( get32bit(buffer) );
	std::cout << "Y coordinate of red endpoint is   " << (double)endpRedY << std::endl;
	
	// offset 82, size 4 - Z coordinate of red endpoint
	fin.read(buffer, 4) || finish("cannot read");
	auto const endpRedZ = FXPT2DOT30( get32bit(buffer) );
	std::cout << "Z coordinate of red endpoint is   " << (double)endpRedZ << std::endl;
	
	// offset 86, size 4 - X coordinate of green endpoint
	fin.read(buffer, 4) || finish("cannot read");
	auto const endpGreenX = FXPT2DOT30( get32bit(buffer) );
	std::cout << "X coordinate of green endpoint is " << (double)endpGreenX << std::endl;
	
	// offset 90, size 4 - Y coordinate of green endpoint
	fin.read(buffer, 4) || finish("cannot read");
	auto const endpGreenY = FXPT2DOT30( get32bit(buffer) );
	std::cout << "Y coordinate of green endpoint is " << (double)endpGreenY << std::endl;
	
	// offset 94, size 4 - Z coordinate of green endpoint
	fin.read(buffer, 4) || finish("cannot read");
	auto const endpGreenZ = FXPT2DOT30( get32bit(buffer) );
	std::cout << "Z coordinate of green endpoint is " << (double)endpGreenZ << std::endl;
	
	// offset 98, size 4 - X coordinate of blue endpoint
	fin.read(buffer, 4) || finish("cannot read");
	auto const endpBlueX = FXPT2DOT30( get32bit(buffer) );
	std::cout << "X coordinate of blue endpoint is  " << (double)endpBlueX << std::endl;
	
	// offset 102, size 4 - Y coordinate of blue endpoint
	fin.read(buffer, 4) || finish("cannot read");
	auto const endpBlueY = FXPT2DOT30( get32bit(buffer) );
	std::cout << "Y coordinate of blue endpoint is  " << (double)endpBlueY << std::endl;
	
	// offset 106, size 4 - Z coordinate of blue endpoint
	fin.read(buffer, 4) || finish("cannot read");
	auto const endpBlueZ = FXPT2DOT30( get32bit(buffer) );
	std::cout << "Z coordinate of blue endpoint is  " << (double)endpBlueZ << std::endl;

	// offset 110, size 4 - Gamma red coordinate scale value
	fin.read(buffer, 4) || finish("cannot read");
	auto const gammaRed = get32bit(buffer);
	std::cout << "Gamma red coordinate scale value  " << gammaRed << std::endl;

	// offset 114, size 4 - Gamma green coordinate scale value
	fin.read(buffer, 4) || finish("cannot read");
	auto const gammaGreen = get32bit(buffer);
	std::cout << "Gamma green coordinate scale value " << gammaGreen << std::endl;

	// offset 118, size 4 - Gamma blue coordinate scale value
	fin.read(buffer, 4) || finish("cannot read");
	auto const gammaBlue = get32bit(buffer);
	std::cout << "Gamma blue coordinate scale value  " << gammaBlue << std::endl;

	// offset 122, size 4 - Intent!
	fin.read(buffer, 4) || finish("cannot read");
	auto const intent = get32bit(buffer);
	std::cout << "The intent is " << intent << std::endl;

	// offset 126, size 4 - Profile data
	fin.read(buffer, 4) || finish("cannot read");
	auto const profileDataOffset = get32bit(buffer);
	std::cout << "Profile data offset " << profileDataOffset << std::endl;

	// offset 130, size 4 - Profile size
	fin.read(buffer, 4) || finish("cannot read");
	auto const profileDataSize = get32bit(buffer);
	std::cout << "Profile data size   " << profileDataSize << std::endl;

	// offset 134, size 4 - nothing
	fin.read(buffer, 4) || finish("cannot read");
	
	// ****************** DATA ***********************
	buffer[0] = buffer[1] = buffer[2] = buffer[3] = 0;
	
	RGBAfactory factory(maskRed, maskGreen, maskBlue, maskAlpha);
	RGBAdata data(pixelHeight);
	
	auto currentOffset = dataOffset;
	for(auto &row : data)
	{
		row.resize(pixelWidth);
		{
			auto nextOffset = (currentOffset*4 + 3)/4;
			while(nextOffset > currentOffset && currentOffset < fileSize)
			{
				fin.read(buffer, 1) || finish("cannot read");
				++currentOffset;
			}
		}
		for(auto &pixel : row)
		{
			auto bytesToRead=(bitsPerPixel+7)/8;
			fin.read(buffer, bytesToRead) || finish("cannot read");
			factory.makeRGBAquad(pixel, get32bit(buffer));
			currentOffset+=bytesToRead;
		}
	}
	std::cout << "The offset at the end is " << currentOffset << std::endl;
	
	fin.close();
	
	
	// APPLYING BLUR
	
	int8_t blurMatrix[3][3] =
		{	{ 1,  1,  1},
			{ 1,  1,  1},
			{ 1,  1,  1}
		};
	
	auto blurredData = applyBlurMatrix(data, blurMatrix);
	
	// writing it all out
	std::ofstream fout("out.bmp", std::ios::binary);
	
	// ***** HEADER
	buffer[0]='B'; buffer[1]='M';
	fout.write(buffer, 2) || finish("cannot write");
	
	// offset 2, size 4 - size of file
	set32bit(fileSize, buffer);
	fout.write(buffer, 4) || finish("cannot write");
	
	// offset 6, size 4 - reserved (ignoring)
	set32bit(0, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 10, size 4 - location of data
	set32bit(dataOffset, buffer);
	fout.write(buffer, 4) || finish("cannot write");
	
	// ***** DIB HEADER
	// http://www.vsokovikov.narod.ru/New_MSDN_API/Bitmaps/str_bitmapv5header.htm
	// https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv5header
	// https://stackoverflow.com/questions/20864752/how-is-defined-the-data-type-fxpt2dot30-in-the-bmp-file-structure
	
	// offset 14, size 4 - size of DIB header
	set32bit(dibHeaderSize, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 18, size 4 - pixel width
	set32bit(pixelWidth, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 22, size 4 - pixel height
	set32bit(pixelHeight, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 26, size 2 - colour planes ?
	set16bit(colourPlanes, buffer);
	fout.write(buffer, 2) || finish("cannot write");

	// offset 28, size 2 - bits per pixel
	set16bit(bitsPerPixel, buffer);
	fout.write(buffer, 2) || finish("cannot write");

	// offset 30, size 4 - compression type
	set32bit(compressionType, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 34, size 4 - image size
	set32bit(imageSize, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 38, size 4 - horizontal resolution
	set32bit(horizontalRes, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 42, size 4 - horizontal resolution
	set32bit(verticalRes, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 46, size 4 - number of colours
	set32bit(numOfColours, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 50, size 4 - number of important colours
	set32bit(numOfImportantColours, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// DIB: COLOUR PROFILE
	
	// offset 54, size 4 - red bitmask
	set32bit(maskRed, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 58, size 4 - green bitmask
	set32bit(maskGreen, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 62, size 4 - blue bitmask
	set32bit(maskGreen, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 66, size 4 - alpha bitmask
	set32bit(maskAlpha, buffer);
	fout.write(buffer, 4) || finish("cannot write");
	
	// offset 70, size 4 - colour space
	set32bit(colourSpace, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 74, size 4 - X coordinate of red endpoint
	set32bit(endpRedX.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");
	
	// offset 78, size 4 - Y coordinate of red endpoint
	set32bit(endpRedY.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");
	
	// offset 82, size 4 - Z coordinate of red endpoint
	set32bit(endpRedZ.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");
	
	// offset 86, size 4 - X coordinate of green endpoint
	set32bit(endpGreenX.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");
	
	// offset 90, size 4 - Y coordinate of green endpoint
	set32bit(endpGreenY.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");
	
	// offset 94, size 4 - Z coordinate of green endpoint
	set32bit(endpGreenZ.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");
	
	// offset 98, size 4 - X coordinate of blue endpoint
	set32bit(endpBlueX.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");
	
	// offset 102, size 4 - Y coordinate of blue endpoint
	set32bit(endpBlueY.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");
	
	// offset 106, size 4 - Z coordinate of blue endpoint
	set32bit(endpBlueZ.data, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 110, size 4 - Gamma red coordinate scale value
	set32bit(gammaRed, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 114, size 4 - Gamma green coordinate scale value
	set32bit(gammaGreen, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 118, size 4 - Gamma blue coordinate scale value
	set32bit(gammaBlue, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 122, size 4 - Intent!
	set32bit(intent, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 126, size 4 - Profile data
	set32bit(profileDataOffset, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 130, size 4 - Profile size
	set32bit(profileDataSize, buffer);
	fout.write(buffer, 4) || finish("cannot write");

	// offset 134, size 4 - nothing
	set32bit(0, buffer);
	fout.write(buffer, 4) || finish("cannot write");
	
	// ****************** DATA ***********************
	buffer[0] = buffer[1] = buffer[2] = buffer[3] = 0;
	
	auto currentWriteOffset = dataOffset;
	for(auto &row : blurredData)
	{
		{
			auto nextOffset = (currentWriteOffset*4 + 3)/4;
			while(nextOffset > currentWriteOffset && currentWriteOffset < fileSize)
			{
				fout.write(0, 1) || finish("cannot write");
				++currentWriteOffset;
			}
		}
		for(auto &pixel : row)
		{
			auto bytesToWrite=(bitsPerPixel+7)/8;
			uint32_t bytes = factory.saveRGBAquad(pixel);
			set32bit(bytes, buffer);
			fout.write(buffer, bytesToWrite) || finish("cannot write");
			currentWriteOffset+=bytesToWrite;
		}
	}
	std::c
	endl;
	
	fout.close();

	return 0;
}