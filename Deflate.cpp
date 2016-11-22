/*
	Author: Jason Bogle
	Class: Analysis of Algorithms
	Assignment: The big project
	File: Deflate.cpp
	Description: This file implements the Deflate class. 
*/


#include "Deflate.h"
#include<iostream>
#include<iomanip>
#include<algorithm>


Deflate::Deflate() {
	totalBytes = 0;
}

Deflate::Deflate(std::string infilename) {
	totalBytes = 0;
	infile.open(infilename, std::ios::in | std::ios::binary);
	outfile.open(infilename + "_out", std::ios::out | std::ios::binary);
}


Deflate::Deflate(std::string infilename, std::string outfilename) {
	totalBytes = 0;
	infile.open(infilename, std::ios::in | std::ios::binary);
	outfile.open(outfilename, std::ios::out | std::ios::binary);
}

void Deflate::SetInputFile(std::string inputfilename) {
	free(inputMsg);
	infile.open(inputfilename, std::ios::in | std::ios::binary);
	infile.seekg(0, infile.end);
	inputSize = infile.tellg();
	infile.seekg(infile.beg);
	inputMsg = (char*)malloc(inputSize + 1);
	memset(inputMsg, '\0', inputSize + 1);
	infile.read(inputMsg, inputSize);
	infile.seekg(infile.beg);
}

void Deflate::SetOutputFile(std::string outputfilename) {
	outfile.open(outputfilename, std::ios::out | std::ios::binary);
}

void Deflate::CloseInputFile() {
	infile.close();
}

void Deflate::CloseOutputFile() {
	outfile.close();
}

void Deflate::WriteToOutputFile() {
	outfile.write(outputMsg, outputSize);
	//for (int w = 0; w < outputMsg.size()/**/; w += 4095) {
	//	int len = std::min(4095, (int)(outputMsg.size() - w));
	//	outfile.write(outputMsg.substr(w, len).c_str(), len);
	//}
}

void Deflate::SetInput(char* inMsg, uint32_t size) {
	free(inputMsg);
	inputSize = size;
	inputMsg = (char*)malloc(size + 1);
	memset(inputMsg, '\0', size + 1);
	memcpy(inputMsg, inMsg, size);
}

char* Deflate::GetInput() {
	return outputMsg;
}

char* Deflate::GetOutput() {
	return outputMsg;
}

uint32_t Deflate::GetOutputLength() {
	return outputSize;
}

void Deflate::Compress() {
	lz.SetMode(LZ77_DFLAT);
	int blocksize = 16000;
	char* currBlock = (char*)malloc(blocksize + 1);
	char* outBlock1 = (char*)malloc(blocksize + 1);
	char* outBlock2 = (char*)malloc(blocksize + 1);
	int start, end;
	end = -1;
	do {
		start = end + 1;
		end = std::min((uint32_t)(start + blocksize), inputSize);
		memset(currBlock, '\0', blocksize + 1);
		//memset(outBlock1, '\0', blocksize + 1);
		//memset(outBlock2, '\0', blocksize + 1);
		memcpy(currBlock, &(inputMsg[start]), end - start); 
		lz.SetInput(currBlock, end - start);
		lz.Compress();
		huff.SetInput(lz.GetOutput(), lz.GetOutputLength());
		huff.SetCharFrequencies(lz.GetCharFrequencies());
		huff.SetLZ1Frequencies(lz.GetLZ1Frequencies());
		huff.SetLZ2Frequencies(lz.GetLZ2Frequencies());
		huff.Compress();
		//if (outputReserved - outputSize < lz.GetOutputLength() + 1) {
		if (outputReserved - outputSize < huff.GetOutputLength() + 1) {
			char* temp = (char*)malloc(outputSize + 1);
			memset(temp, '\0', outputSize + 1);
			outputReserved += blocksize;
			free(outputMsg);
			outputMsg = (char*)malloc(outputReserved + 1);
			memset(outputMsg, '\0', outputReserved + 1);
			memcpy(outputMsg, temp, outputSize + 1);
			free(temp);
			//delete temp;
		}
		/*
		memcpy(&(outputMsg[outputPos]), lz.GetOutput(), lz.GetOutputLength() + 1);
		outputPos += lz.GetOutputLength();
		outputSize += lz.GetOutputLength();
		// */
		//*
		memcpy(&(outputMsg[outputPos]), huff.GetOutput(), huff.GetOutputLength() + 1);
		outputPos += huff.GetOutputLength();
		outputSize += huff.GetOutputLength();
		// */
	} while (end < inputSize);
	compressedsize = outputSize;
	totalBytes = inputSize;
	DisplayStats();
}

void Deflate::Decompress() {
	uint32_t start = 0;
	uint32_t end = 0;
	uint32_t len;
	int thebit;
	while (end < inputSize) {
		len = 0;
		for (int i = 0; i < 32; i++) {
			thebit = ((1 << (i % 8)) & inputMsg[(start + (i / 8))]) >> (i % 8);
			//std::cout << thebit;
			if (thebit == 1) {
				len |= (thebit << ((3 - (i / 8)) * 8 + (i % 8)));
			}
		}
		start = end + len + 1;
		end = start;
	}
}

void Deflate::DisplayStats() {
	double percent = (double)compressedsize / totalBytes * 100;
	double ratio = (double)totalBytes / compressedsize;
	//*
	std::cout << "\nOriginal data size:\t" << totalBytes * 8 << " bits;\t";
	std::cout << totalBytes << " bytes\n";
	std::cout << "Total compressed size:\t" << compressedsize * 8 << " bits; \t";
	std::cout << compressedsize << " bytes\n";
	std::cout << "Compression ratio:\t"
		<< std::fixed << std::setprecision(3) << ratio << '\t'
		<< std::fixed << std::setprecision(2) << percent << "%\n";
	// */
}

Deflate::~Deflate()
{
}
