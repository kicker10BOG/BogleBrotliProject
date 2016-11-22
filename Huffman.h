/*
	Author: Jason Bogle
	Class: Analysis of Algorithms
	Assignment: The big project
	File: Huffman.h
	Description: This file defines the Huffman class. 
*/

#pragma once

#include<cstddef>
#include<map>
#include<fstream>
#include<queue>
#include<bitset>

struct HuffNode 
{
	char data; 
	int freq;
	HuffNode* left;
	HuffNode* right;

	HuffNode(char dataChar, int frequency)
{
		left = right = NULL;
		data = dataChar;
		freq = frequency;
	}

	HuffNode(char dataChar)
{
		left = right = NULL;
		data = dataChar;
	}

	HuffNode(HuffNode* l, HuffNode* r) 
{
		data = '\0';
		left = l;
		right = r;
		freq = left->freq + right->freq;
	}
};

struct  HuffNodeComp
{
	bool operator()(HuffNode* l, HuffNode* r)
{
		return l->freq > r->freq;
	}
};

class Huffman 
{
private:
	HuffNode* root;
	int totalBytes;
	int treeNodes;
	uint32_t treesize;
	uint32_t compressedsize;
	std::map<char, int> frequencies;
	std::map<char, int> lengths;
	std::map<char, std::string> chartocode;
	std::map<std::string, char> codetochar;
	std::ifstream infile;
	std::ofstream outfile;
	uint32_t inputSize;
	char* inputMsg = (char*)malloc(1024);
	uint32_t outputPos = 0;
	uint32_t outputSize = 0;
	uint32_t outputReserved = 1024;
	char* outputMsg = (char*)malloc(1024);

public:
	Huffman();
	Huffman(std::string infilename);
	Huffman(std::string infilename, std::string outfilename);

	void SetInputFile(std::string inputfilename);
	void SetOutputFile(std::string outputfilename);
	void CloseInputFile();
	void CloseOutputFile();
	void WriteToOutputFile();
	void SetInput(char* inMsg, uint32_t size);
	char* GetInput();
	uint32_t GetInputLength();
	char* GetOutput();
	uint32_t GetOutputLength();

	void Compress();
	void Decompress();

	void PrintCodes(HuffNode* node, std::string code);
	int CalcCompressedSize();

	~Huffman();

private:
	// compression and decompression methods
	std::string GetCode(char c, HuffNode* node, std::string code, bool* found);
	// compression methods
	void WriteCompressedFile();
	void CountChars();
	void BuildTreeForCompression();
	void StoreCodesAndLengths(HuffNode* node, std::string code);
	void SaveNode(HuffNode* node, unsigned char *byte, int* bit);
	void WriteCompressedData(unsigned char *byte, int* bit);
	void TryToWriteByte(unsigned char *byte, int* bit);
	void AddChar(char c, unsigned char *byte, int* bit);
	// decompression methods
	void InflateTree();
	HuffNode* GenNode(unsigned char* buffer, int* buffbyte, unsigned char* byte, int* bit, std::string code);
	char ReadByte(unsigned char* buffer, int* buffbyte, unsigned char* byte, int* bit);
	void StoreChars(HuffNode* node, std::string code);
	void DecryptMsg();
	char SearchNode(HuffNode* node, std::string code, bool* found);
};

