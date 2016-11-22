/*
	Author: Jason Bogle
	Class: Analysis of Algorithms
	Assignment: The big project
	File: Deflate.h
	Description: This file defines the Deflat class. 
*/
#pragma once

#include "DeflateHuffman.h"
#include "LZ77.h"
#include<fstream>

class Deflate {
	int totalBytes;
	std::ifstream infile;
	std::ofstream outfile;
	uint32_t compressedsize;
	uint32_t compressedbits;
	uint32_t inputSize;
	char* inputMsg = (char*)malloc(1024);
	uint32_t outputPos = 0;
	uint32_t outputSize = 0;
	uint32_t outputReserved = 16050;
	char* outputMsg = (char*)malloc(16050);
	DeflateHuffman huff;
	LZ77 lz;

public:
	Deflate();
	Deflate(std::string infilename);
	Deflate(std::string infilename, std::string outfilename);

	void SetInputFile(std::string inputfilename);
	void SetOutputFile(std::string outputfilename);
	void CloseInputFile();
	void CloseOutputFile();
	void DisplayStats();
	void WriteToOutputFile();
	void SetInput(char* inMsg, uint32_t size);
	char* GetInput();
	char* GetOutput();
	uint32_t GetOutputLength();

	void Compress();
	void Decompress();

	~Deflate();
};

