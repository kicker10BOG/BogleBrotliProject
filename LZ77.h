/*
	Author: Jason Bogle
	Class: Analysis of Algorithms
	Assignment: The big project
	File: LZ77
	Description: This file defines the LZ77 class. 
*/

#pragma once

#include<fstream>
#include<map>

#define LZ77_2BYTE 0
#define LZ77_PROTO 1
#define LZ77_DFLAT 2

class LZ77
{
private:
	int totalBytes;
	std::ifstream infile;
	std::ofstream outfile;
	uint32_t compressedsize;
	uint32_t compressedbits;
	int mode = 0;
	//std::string encodedMsgStrings[1024];
	//std::string deodedMsgStrings[1024
	uint32_t inputSize;
	char* inputMsg = (char*)malloc(1024);
	uint32_t outputPos = 0;
	uint32_t outputSize = 0;
	uint32_t outputReserved = 1024;
	char* outputMsg = (char*)malloc(1024);
	std::map<char, int> charFrequencies;
	std::map<char, int> lz1Frequencies;
	std::map<char, int> lz2Frequencies;

public:
	LZ77();
	LZ77(int option);
	LZ77(std::string infilename);
	LZ77(std::string infilename, std::string outfilename);

	void SetInputFile(std::string inputfilename);
	void SetOutputFile(std::string outputfilename);
	void CloseInputFile();
	void CloseOutputFile();
	void SetMode(int option);
	void DisplayStats();
	void WriteToOutputFile();
	void SetInput(char* inMsg, uint32_t size);
	char* GetInput();
	uint32_t GetInputLength();
	char* GetOutput();
	uint32_t GetOutputLength();
	std::map<char, int> GetCharFrequencies();
	std::map<char, int> GetLZ1Frequencies();
	std::map<char, int> GetLZ2Frequencies();

	void Compress();
	void Decompress();
	~LZ77();

private:
	// Compression Methods
	void CompressProto();
	void Compress2Bytes();
	void CompressForDeflate();
	void TryToWriteByte(unsigned char *byte, int* bit);
	void AddChar(char c, unsigned char *byte, int* bit);
	void AddPointer(uint32_t offset, uint32_t length, 
		int dictBits, int lenBits, unsigned char *byte, int* bit);
	// Decompression Methods
	void DecompressProto();
	void Decompress2Bytes();
};

