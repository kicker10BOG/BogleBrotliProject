/*
Author: Jason Bogle
Class: Analysis of Algorithms
Assignment: The big project
File: LZ77.cpp
Description: This file implements the LZ77 class
*/

#include "LZ77.h"
#include<cstddef>
#include<iostream>
#include<sstream>
#include<iomanip>
#include<algorithm>

LZ77::LZ77() {
	totalBytes = 0;
}

LZ77::LZ77(int option) {
	totalBytes = 0;
	mode = option;
}

LZ77::LZ77(std::string infilename) {
	totalBytes = 0;
	infile.open(infilename, std::ios::in | std::ios::binary);
	outfile.open(infilename + "_out", std::ios::out | std::ios::binary);
}


LZ77::LZ77(std::string infilename, std::string outfilename) {
	totalBytes = 0;
	infile.open(infilename, std::ios::in | std::ios::binary);
	outfile.open(outfilename, std::ios::out | std::ios::binary);
}

void LZ77::SetInputFile(std::string inputfilename) {
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

void LZ77::SetOutputFile(std::string outputfilename) {
	outfile.open(outputfilename, std::ios::out | std::ios::binary);
}

void LZ77::CloseInputFile() {
	infile.close();
}

void LZ77::CloseOutputFile() {
	outfile.close();
}

void LZ77::SetMode(int option) {
	mode = option;
}

void LZ77::Compress() {
	free(outputMsg);
	outputReserved = totalBytes = inputSize;
	outputMsg = (char*)malloc(outputReserved);
	outputPos = outputSize = 0;
	memset(outputMsg, '\0', outputReserved);
	switch (mode)
	{
	case LZ77_2BYTE:
		Compress2Bytes();
		break;
	case LZ77_PROTO:
		CompressProto();
		break;
	case LZ77_DFLAT:
		CompressForDeflate();
		break;
	default:
		break;
	}
	compressedsize = outputSize;
	//DisplayStats();
}

void LZ77::CompressProto() {
	// get total chars
	infile.seekg(0, infile.end);
	totalBytes = infile.tellg();
	infile.seekg(0, infile.beg);
	char* msg = (char*)calloc(sizeof(char), totalBytes + 1);
	infile.read(msg, totalBytes);
	//std::cout << msg << '\n';
	char* encodedMsg = (char*)calloc(sizeof(char), totalBytes + 1);
	int dictSize = 4095;// how far back to look
	int minMatch = 7;// minimum best match
	int maxMatch = INT16_MAX;
	int ep, mp;		// indexes for encoded and original message
	int dp, cp;		// indexes for dictionary start and current
	int currCount;	// the current match length
	int bp, bestLen;// index to start of best match and its length

	for (int i = 0; i < minMatch; i++) {
		outfile << msg[i];
	}

	// loop through every letter
	for (mp = minMatch; mp < totalBytes; ) {
		dp = std::max(0, mp - dictSize);
		bp = 0;
		bestLen = 0;
		for (cp = mp - 3; cp >= dp && bestLen < maxMatch; cp--) {
			for (currCount = 0;
				msg[cp + currCount] == msg[mp + currCount] && bestLen < maxMatch;
				currCount++)
			{
				if (currCount + 1 > bestLen) {
					bp = cp;
					bestLen = currCount + 1;
				}
			}
		}
		if (bestLen > minMatch) {
			outfile << '<' << mp - bp << ':' << bestLen << '>';
			mp += bestLen;
		}
		else {
			outfile << msg[mp];
			mp++;
		}
	}
}

void LZ77::Compress2Bytes() {
	/*
	// get total chars
	infile.seekg(0, infile.end);
	totalBytes = infile.tellg();
	infile.seekg(0, infile.beg);
	char* msg = (char*)calloc(sizeof(char), totalBytes + 1);
	infile.read(msg, totalBytes);
	//std::cout << msg << '\n';
	char* encodedMsg = (char*)calloc(sizeof(char), totalBytes + 1);
	// */
	int dictSize = 4096;// how far back to look
	int minMatch = 3;// minimum best match
	int maxMatch = 15 + minMatch; // maximum best match
	int ep, mp;		// indexes for encoded and original message
	int dp, cp;		// indexes for dictionary start and current
	int currCount;	// the current match length
	int bp, bestLen;// index to start of best match and its length
	unsigned char cBuffer = 0; // buffer for writing to file
	int bitCount = 0;// the bitcount of the buffer

	//for (int i = 0; i < minMatch; i++) {
	//	outputMsg[outputPos] = inputMsg[i];
	//	outputPos++;
	//	outputSize++;
	//}

	// loop through every letter
	for (mp = 0; mp < totalBytes; ) {
		dp = std::max(0, mp - dictSize);
		bp = 0;
		bestLen = 0;
		for (cp = mp - 1; cp >= dp && bestLen < maxMatch; cp--) {
			for (currCount = 0;
			inputMsg[cp + currCount] == inputMsg[mp + currCount] && bestLen < maxMatch;
				currCount++)
			{
				if (currCount + 1 > bestLen) {
					bp = cp;
					bestLen = currCount + 1;
				}
			}
		}
		cp++;
		// */
		if (bestLen > minMatch) {
			// mark as encoded
			//cBuffer |= (0 << (7 - bitCount));
			bitCount++;
			TryToWriteByte(&cBuffer, &bitCount);
			AddPointer(mp - bp, bestLen - minMatch, 12, 4, &cBuffer, &bitCount);
			mp += bestLen;
		}
		else {
			// mark as uncoded
			cBuffer |= (1 << (7 - bitCount));
			bitCount++;
			TryToWriteByte(&cBuffer, &bitCount);
			AddChar(inputMsg[mp], &cBuffer, &bitCount);
			mp++;
		}
		if (mp > 25225) {
			std::cout << "";
		}
	}
	if (outputReserved - outputSize < 10) {
		char* temp = (char*)malloc(outputSize + 1);
		memcpy(temp, outputMsg, outputSize + 1);
		free(outputMsg);
		outputReserved += 10;
		outputMsg = (char*)malloc(outputReserved);
		memset(outputMsg, '\0', outputReserved);
		memcpy(outputMsg, temp, outputSize + 1);
	}
	compressedbits = outputSize * 8;
	if (bitCount % 8 > 0) {
		outputMsg[outputPos] = (char)(cBuffer);
		outputPos++;
		outputSize++;
		compressedbits -= (8 - bitCount);
	}
	char* temp = (char*)malloc(outputSize + 1);
	memcpy(temp, outputMsg, outputSize + 1);
	free(outputMsg);
	outputReserved = outputSize + 5;
	outputMsg = (char*)malloc(outputReserved);
	memset(outputMsg, '\0', outputReserved);
	outputPos = 0;
	int thebit;
	cBuffer = 0;
	bitCount = 0;
	for (int i = 0; i < 32; i++) {
		thebit = ((1 << (31 - i)) & compressedbits) >> (31 - i);
		//std::cout << thebit;
		if (thebit == 1) {
			cBuffer |= 1 << (7 - bitCount);
		}
		bitCount++;
		TryToWriteByte(&cBuffer, &bitCount);
	}
	memcpy(&(outputMsg[4]), temp, outputSize - 4);
	free(temp);
}

void LZ77::CompressForDeflate() {
	/*
	// get total chars
	infile.seekg(0, infile.end);
	totalBytes = infile.tellg();
	infile.seekg(0, infile.beg);
	char* msg = (char*)calloc(sizeof(char), totalBytes + 1);
	infile.read(msg, totalBytes);
	//std::cout << msg << '\n';
	char* encodedMsg = (char*)calloc(sizeof(char), totalBytes + 1);
	// */
	int dictSize = 4096;// how far back to look
	int minMatch = 3;// minimum best match
	int maxMatch = 15 + minMatch; // maximum best match
	int ep, mp;		// indexes for encoded and original message
	int dp, cp;		// indexes for dictionary start and current
	int currCount;	// the current match length
	int bp, bestLen;// index to start of best match and its length
	unsigned char cBuffer = 0; // buffer for writing to file
	int bitCount = 0;// the bitcount of the buffer

	//for (int i = 0; i < minMatch; i++) {
	//	outputMsg[outputPos] = inputMsg[i];
	//	outputPos++;
	//	outputSize++;
	//}

	// loop through every letter
	for (mp = 0; mp < totalBytes; ) {
		dp = std::max(0, mp - dictSize);
		bp = 0;
		bestLen = 0;
		for (cp = mp - 1; cp >= dp && bestLen < maxMatch; cp--) {
			for (currCount = 0;
			inputMsg[cp + currCount] == inputMsg[mp + currCount] && bestLen < maxMatch;
				currCount++)
			{
				if (currCount + 1 > bestLen) {
					bp = cp;
					bestLen = currCount + 1;
				}
			}
		}
		cp++;
		// */
		if (bestLen > minMatch) {
			// mark as encoded
			//cBuffer |= (0 << (7 - bitCount));
			bitCount++;
			TryToWriteByte(&cBuffer, &bitCount);
			AddPointer(mp - bp, bestLen - minMatch, 12, 4, &cBuffer, &bitCount);
			char byte1 = ((mp - bp) >> 4);
			char byte2 = (((mp - bp) << 4) | bestLen);
			if (lz1Frequencies.find(byte1) == lz1Frequencies.end()) {
				lz1Frequencies[byte1] = 0;
			}
			lz1Frequencies[byte1]++;
			if (lz2Frequencies.find(byte2) == lz2Frequencies.end()) {
				lz2Frequencies[byte2] = 0;
			}
			lz2Frequencies[byte2]++;
			mp += bestLen;
		}
		else {
			// mark as uncoded
			cBuffer |= (1 << (7 - bitCount));
			bitCount++;
			TryToWriteByte(&cBuffer, &bitCount);
			AddChar(inputMsg[mp], &cBuffer, &bitCount);
			if (charFrequencies.find(inputMsg[mp]) == charFrequencies.end()) {
				charFrequencies[inputMsg[mp]] = 0;
			}
			charFrequencies[inputMsg[mp]]++;
			mp++;
		}
	}
	if (outputReserved - outputSize < 10) {
		char* temp = (char*)malloc(outputSize + 1);
		memcpy(temp, outputMsg, outputSize + 1);
		free(outputMsg);
		outputReserved += 10;
		outputMsg = (char*)malloc(outputReserved);
		memset(outputMsg, '\0', outputReserved);
		memcpy(outputMsg, temp, outputSize + 1);
	}
	compressedbits = outputSize * 8;
	if (bitCount % 8 > 0) {
		outputMsg[outputPos] = (char)(cBuffer);
		outputPos++;
		outputSize++;
		compressedbits -= (8 - bitCount);
	}
	char* temp = (char*)malloc(outputSize + 1);
	memcpy(temp, outputMsg, outputSize + 1);
	free(outputMsg);
	outputReserved = outputSize + 5;
	outputMsg = (char*)malloc(outputReserved);
	memset(outputMsg, '\0', outputReserved);
	outputPos = 0;
	int thebit;
	cBuffer = 0;
	bitCount = 0;
	for (int i = 0; i < 32; i++) {
		thebit = ((1 << (31 - i)) & compressedbits) >> (31 - i);
		std::cout << thebit;
		if (thebit == 1) {
			cBuffer |= 1 << (7 - bitCount);
		}
		bitCount++;
		TryToWriteByte(&cBuffer, &bitCount);
	}
	memcpy(&(outputMsg[4]), temp, outputSize - 4);
	free(temp);
}

void  LZ77::TryToWriteByte(unsigned char *byte, int* bit) {
	if (*bit == 8) {
		//outfile.write((char*)byte, 1);
		//outputMsg += (*byte);
		if (outputReserved - outputSize < 100) {
			char* temp = (char*)malloc(outputSize + 1);
			memcpy(temp, outputMsg, outputSize + 1);
			free(outputMsg);
			outputReserved += 1024;
			outputMsg = (char*)malloc(outputReserved);
			memset(outputMsg, '\0', outputReserved);
			memcpy(outputMsg, temp, outputSize + 1);
		}
		outputMsg[outputPos] = (char)(*byte);
		outputPos++;
		outputSize++;
		*bit = 0;
		*byte = 0;
	}
}

void LZ77::AddChar(char c, unsigned char *byte, int* bit) {
	if (*bit == 0) {
		*byte = c;
		*bit = 8;
		TryToWriteByte(byte, bit);
		return;
	}
	int thebit;
	for (int i = 0; i < 8; i++) {
		thebit = (c & (1 << (7 - i))) >> (7 - i);
		if (thebit == 1) {
			*byte |= (1 << (7 - *bit));
		}
		(*bit)++;
		TryToWriteByte(byte, bit);
	}
}

void LZ77::AddPointer(uint32_t offset, uint32_t length,
	int dictBits, int lenBits, unsigned char *byte, int* bit)
{
	int thebit;
	for (int i = 0; i < dictBits; i++) {
		thebit = (offset & (1 << i)) >> i;
		if (thebit == 1) {
			*byte |= (1 << (7 - *bit));
		}
		(*bit)++;
		TryToWriteByte(byte, bit);
	}
	for (int i = 0; i < lenBits; i++) {
		thebit = (length & (1 << i)) >> i;
		if (thebit == 1) {
			*byte |= (1 << (7 - *bit));
		}
		(*bit)++;
		TryToWriteByte(byte, bit);
	}
}

void LZ77::DisplayStats() {
	std::string smode;

	switch (mode)
	{
	case LZ77_2BYTE:
		smode = "LZ77_2BYTE";
		break;
	case LZ77_PROTO:
		smode = "LZ77_2BYTE";
		break;
	case LZ77_DFLAT:
		smode = "LZ77_DFLAT";
		break;
	default:
		break;
	}
	//std::cout << "Mode used: " << smode.c_str() << '\n';
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

void LZ77::Decompress() {
	free(outputMsg);
	outputReserved  = compressedsize = inputSize;
	outputMsg = (char*)malloc(outputReserved);
	outputPos = outputSize = 0;
	memset(outputMsg, '\0', outputReserved);
	switch (mode)
	{
	case LZ77_2BYTE:
	case LZ77_DFLAT:
		Decompress2Bytes();
		break;
	case LZ77_PROTO:
		DecompressProto();
		break;
	default:
		break;
	}
	totalBytes = outputSize;
}

void LZ77::DecompressProto() {
	// get total chars
	infile.seekg(0, infile.end);
	totalBytes = infile.tellg();
	infile.seekg(0, infile.beg);
	char* encodedMsg = (char*)calloc(sizeof(char), totalBytes + 1);
	infile.read(encodedMsg, totalBytes);
	//std::cout << encodedMsg << '\n';
	//char* msg = (char*)calloc(sizeof(char), totalBytes + 1);
	int dictSize = 4096;// how far back to look
	int minMatch = 7;// minimum best match
	int ep, mp;		// indexes for encoded and original message
	int dp, cp;		// indexes for dictionary start and current
	int currCount;	// the current match length
	int bp, bestLen;// index to start of best match and its length

	//for (int i = 0; i < minMatch; i++) {
	//	outfile << encodedMsg[i];
	//}
}

void LZ77::Decompress2Bytes() {
	// get total chars
	/*
	infile.seekg(0, infile.end);
	totalBytes = infile.tellg();
	infile.seekg(0, infile.beg);
	char* encodedMsg = (char*)calloc(sizeof(char), totalBytes + 1);
	infile.read(encodedMsg, totalBytes);
	// */
	//std::cout << encodedMsg << '\n';
	//char* msg = (char*)calloc(sizeof(char), totalBytes + 1);
	int dictSize = 4095;// how far back to look
	int minMatch = 3;// minimum best match
	int ep, mp;		// indexes for encoded and original message
	int dp, cp;		// indexes for dictionary start and current
	int currCount;	// the current match length
	int bp;			// bit index for encoded message
	unsigned char cBuffer = 0; // buffer for writing to file
	int bitCount = 0;// the bitcount of the buffer
	int thebit;		// used to determine if the next portion is encoded
	int offset, length;
	std::string decodedMsg = "";
	//char* decodedMsg = (char*)malloc(sizeof(char) * (totalBytes + 1) ); // the decoded message - assume you need n more than 2x the size of the compressed message
	//long int allocated = (totalBytes + 1) ;
	//memset(decodedMsg, '\0', sizeof(char) * allocated);
	//*
	compressedbits = 0;
	for (int i = 0; i < 32; i++) {
		//thebit = ((1 << (7 - (i % 8)) & (inputMsg[i / 8])) >> (7 - (i % 8)));
		thebit = ((1 << (i % 8)) & inputMsg[i / 8]) >> (i % 8);
		//std::cout << thebit;
		compressedbits |= (thebit << ((3 - (i / 8)) * 8 + (i % 8)));
	}
	//std::cout << '\n' << compressedbits << '\n';
	// */
	/*
	for (int i = 0; i < minMatch; i++) {
		//outfile << encodedMsg[i];
		//decodedMsg[i] = encodedMsg[i];
		//decodedMsg += encodedMsg[i];
		if (outputReserved - outputSize < 100) {
			char* temp = (char*)malloc(outputSize + 1);
			memcpy(temp, outputMsg, outputSize + 1);
			free(outputMsg);
			outputReserved += 1024;
			outputMsg = (char*)malloc(outputReserved);
			memset(outputMsg, '\0', outputReserved);
			memcpy(outputMsg, temp, outputSize + 1);
		}
		outputMsg[outputPos] = inputMsg[i + 4];
		outputPos++;
		outputSize++;
		std::cout << inputMsg[i + 4];
	}
	// */
	//std::cout << '\n';
	// loop through the encoded message
	mp = 0;
	uint32_t lastbit = compressedbits + 4 * 8;
	for (bp = 4 * 8; bp < lastbit; ) {
		//std::cout << '\n' << bp << ": ";
		ep = bp / 8;
		bitCount = bp % 8;
		thebit = ((1 << (7 - bitCount)) & (inputMsg[ep])) >> (7 - bitCount);
		//std::cout << thebit << ", ";
		bp++;
		//std::cout << bp << ": ";
		ep = bp / 8;
		bitCount = ep % 8;
		if (thebit == 0) {
			// is encoded
			//std::cout << '\n' << decodedMsg.c_str();
			//std::cout << '\n' << bp << ":\n\t";
			offset = 0;
			length = 0;
			for (int i = 0; i < 12; i++) {
				thebit = ((1 << (7 - (bp % 8)) & (inputMsg[bp / 8])) >> (7 - (bp % 8)));
				//std::cout << thebit;
				offset |= (thebit << (i));
				bp++;
			}
			//std::cout << "\n\t";
			for (int i = 0; i < 4; i++) {
				thebit = ((1 << (7 - (bp % 8)) & (inputMsg[bp / 8])) >> (7 - (bp % 8)));
				//std::cout << thebit;
				length |= (thebit << (i));
				bp++;
			}
			length += minMatch;
			//std::cout << "\n\t";
			//std::cout << "offset: " << offset << " length: " << length;
			//std::cout << " size: " << decodedMsg.size() << " decodedMsg.size() - offset = " << (long long)decodedMsg.size() - (long long)offset;
			for (int i = 0; i < length; i++) {
				//decodedMsg += decodedMsg[mp - offset];
				if (outputReserved - outputSize < 100) {
					char* temp = (char*)malloc(outputSize + 1);
					memcpy(temp, outputMsg, outputSize + 1);
					free(outputMsg);
					outputReserved += 1024;
					outputMsg = (char*)malloc(outputReserved);
					memset(outputMsg, '\0', outputReserved);
					memcpy(outputMsg, temp, outputSize + 1);
				}
				outputMsg[outputPos] = outputMsg[mp - offset];
				outputPos++;
				outputSize++;
				mp++;
			}
			//bp += 32;
		}
		else {
			// is uncoded
			char c = 0;
			for (int i = 0; i < 8; i++) {
				thebit = ((1 << (7 - (bp % 8)) & (inputMsg[bp / 8])) >> (7 - (bp % 8)));
				//std::cout << thebit;
				c |= (thebit << (7 - i));
				bp++;
			}
			//outfile.write(&c, 1);
			//decodedMsg += c;
			if (outputReserved - outputSize < 100) {
				char* temp = (char*)malloc(outputSize + 1);
				memcpy(temp, outputMsg, outputSize + 1);
				free(outputMsg);
				outputReserved += 1024;
				outputMsg = (char*)malloc(outputReserved);
				memset(outputMsg, '\0', outputReserved);
				memcpy(outputMsg, temp, outputSize + 1);
			}
			outputMsg[outputPos] = c;
			outputPos++;
			outputSize++;
			mp++;
			//bp += 8;
		}
	}
}

void LZ77::WriteToOutputFile() {
	outfile.write(outputMsg, outputSize);
	//for (int w = 0; w < outputMsg.size()/**/; w += 4095) {
	//	int len = std::min(4095, (int)(outputMsg.size() - w));
	//	outfile.write(outputMsg.substr(w, len).c_str(), len);
	//}
}

void LZ77::SetInput(char* inMsg, uint32_t size) {
	free(inputMsg);
	inputSize = size;
	inputMsg = (char*)malloc(size + 1);
	memset(inputMsg, '\0', size + 1);
	memcpy(inputMsg, inMsg, size);
}

char* LZ77::GetInput() {
	return outputMsg;
}

uint32_t LZ77::GetInputLength() {
	return inputSize;
}

char* LZ77::GetOutput() {
	return outputMsg;
}

uint32_t LZ77::GetOutputLength() {
	return outputSize;
}

std::map<char, int> LZ77::GetCharFrequencies() {
	return charFrequencies;
}

std::map<char, int> LZ77::GetLZ1Frequencies() {
	return lz1Frequencies;
}

std::map<char, int> LZ77::GetLZ2Frequencies() {
	return lz2Frequencies;
}

LZ77::~LZ77() {
}
