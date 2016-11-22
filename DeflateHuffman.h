/*
Author: Jason Bogle
Class: Analysis of Algorithms
Assignment: The big project
File: DeflateHuffman.h
Description: This file defines the DeflateHuffman class.
*/

#pragma once

#include<cstddef>
#include<map>
#include<fstream>
#include<queue>
#include<bitset>

struct DefHuffNode
{
	char data;
	char data2;
	int freq;
	DefHuffNode* left;
	DefHuffNode* right;

	DefHuffNode(char dataChar, int frequency)
	{
		left = right = NULL;
		data = dataChar;
		freq = frequency;
	}

	DefHuffNode(char dataChar, char dataChar2)
	{
		left = right = NULL;
		data = dataChar;
		data2 = dataChar2;
	}

	DefHuffNode(char dataChar, char dataChar2, int frequency)
	{
		left = right = NULL;
		data = dataChar;
		data2 = dataChar2;
		freq = frequency;
	}

	DefHuffNode(char dataChar)
	{
		left = right = NULL;
		data = dataChar;
	}

	DefHuffNode(DefHuffNode* l, DefHuffNode* r)
	{
		data = '\0';
		left = l;
		right = r;
		freq = left->freq + right->freq;
	}
};

struct  DefHuffNodeComp
{
	bool operator()(DefHuffNode* l, DefHuffNode* r)
	{
		return l->freq > r->freq;
	}
};

class DeflateHuffman
{
private:
	DefHuffNode* charRoot;
	DefHuffNode* lz1Root;
	DefHuffNode* lz2Root;
	int totalBytes = 0;
	uint32_t totalBits = 0;
	int charTreeNodes = 0;
	int lz1TreeNodes = 0;
	int lz2TreeNodes = 0;
	uint32_t charTreeSize = 0;
	uint16_t lz1TreeSize = 0;
	uint16_t lz2TreeSize = 0;
	uint32_t allTreeBytes;
	uint32_t compressedSize = 0;
	uint32_t compressedBytes = 0;
	std::map<char, int> charFrequencies;
	std::map<char, int> charLengths;
	std::map<char, int> lz1Frequencies;
	std::map<char, int> lz1Lengths;
	std::map<char, int> lz2Frequencies;
	std::map<char, int> lz2Lengths;
	std::map<char, std::string> charToCode;
	std::map<char, std::string> lz1ToCode;;
	std::map<char, std::string> lz2ToCode;
	uint32_t inputSize = 0;
	char* inputMsg = (char*)malloc(1024);
	uint32_t outputPos = 0;
	uint32_t outputBits = 0;
	uint32_t outputSize = 0;
	uint32_t outputReserved = 1024;
	char* outputMsg = (char*)malloc(1024);

public:
	DeflateHuffman();
	DeflateHuffman(char* inMsg, uint32_t size);

	void SetInput(char* inMsg, uint32_t size);
	char* GetInput();
	char* GetOutput();
	uint32_t GetOutputLength();
	void SetCharFrequencies(std::map<char, int> freqs);
	void SetLZ1Frequencies(std::map<char, int> freqs);
	void SetLZ2Frequencies(std::map<char, int> freqs);

	void Compress();
	void Decompress();

	void PrintCodes(DefHuffNode* node, std::string code, bool lz = false);
	int CalcCompressedSize();

	~DeflateHuffman();

private:
	// compression and decompression methods
	//std::string GetCode(char c, DefHuffNode* node, std::string code, bool* found);
	// compression methods
	//void WriteCompressedFile();
	//void CountChars();
	void BuildTreesForCompression();
	void StoreCodesAndLengths(DefHuffNode* node, std::string code, int mode = 0);
	void SaveNode(DefHuffNode* node, unsigned char *byte, int* bit);
	//void WriteCompressedData(unsigned char *byte, int* bit);
	void TryToWriteByte(unsigned char *byte, int* bit);
	void AddChar(char c, unsigned char *byte, int* bit);
	// decompression methods
	void InflateTree();
	DefHuffNode* GenNode(unsigned char* buffer, int* buffbyte, unsigned char* byte, int* bit, std::string code);
	char ReadByte(unsigned char* buffer, int* buffbyte, unsigned char* byte, int* bit);
	void StoreChars(DefHuffNode* node, std::string code);
	void DecryptMsg();
	char SearchNode(DefHuffNode* node, std::string code, bool* found);
};

