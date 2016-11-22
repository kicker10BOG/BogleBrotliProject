#include "DeflateHuffman.h"
#include<iostream>
#include<iomanip>


DeflateHuffman::DeflateHuffman() {
	;
}

DeflateHuffman::DeflateHuffman(char* inMsg, uint32_t size) {
	SetInput(inMsg, size);
}

void DeflateHuffman::SetInput(char* inMsg, uint32_t size) {
	free(inputMsg);
	inputSize = size;
	inputMsg = (char*)malloc(size + 1);
	memset(inputMsg, '\0', size + 1);
	memcpy(inputMsg, inMsg, size);
	charFrequencies.clear();
	charLengths.clear();
	lz1Frequencies.clear();
	lz1Lengths.clear();
	lz2Frequencies.clear();
	lz2Lengths.clear();
	charToCode.clear();
	lz1ToCode.clear();
	lz2ToCode.clear();
	charTreeSize = 0;
	lz1TreeSize = 0;
	lz2TreeSize = 0;
	//CountChars();
}

char* DeflateHuffman::GetInput() {
	return inputMsg;
}

char* DeflateHuffman::GetOutput() {
	return outputMsg;
}

uint32_t DeflateHuffman::GetOutputLength() {
	return outputSize;
}

void DeflateHuffman::SetCharFrequencies(std::map<char, int> freqs) {
	charFrequencies = freqs;
}

void DeflateHuffman::SetLZ1Frequencies(std::map<char, int> freqs) {
	lz1Frequencies = freqs;
}

void DeflateHuffman::SetLZ2Frequencies(std::map<char, int> freqs) {
	lz2Frequencies = freqs;
}

void DeflateHuffman::Compress() {
	BuildTreesForCompression();
	StoreCodesAndLengths(charRoot, "");
	StoreCodesAndLengths(lz1Root, "", 1);
	StoreCodesAndLengths(lz2Root, "", 2);
	totalBytes = inputSize;
	int thebit;
	for (int i = 0; i < 32; i++) {
		thebit = ((1 << (7 - (i % 8))) & (inputMsg[i / 8])) >> (7 - (i % 8));
		//std::cout << thebit;
		totalBits |= thebit << (31 - i);
		//totalBits |= thebit << (((3 - i) * 8) + 7 - (i % 8));
		//totalBits |= (inputMsg[i] << (8 * (3 - i)));
	}
	CalcCompressedSize();
	unsigned char byte = 0;
	int bit = 0;
	// Save total bytes of compressed data - for Deflate to know how much to 
	// send for decompression
	for (int i = 0; i < 32; i++) {
		thebit = ((1 << (31 - i)) & compressedBytes) >> (31 - i);
		//std::cout << thebit;
		if (thebit == 1) {
			byte |= 1 << (7 - bit);
		}
		bit++;
		TryToWriteByte(&byte, &bit);
	}
	outputBits += 32;
	//*
	// Save tree for normal chars
	for (int i = 0; i < 32; i++) {
		thebit = ((1 << (31 - i)) & charTreeSize) >> (31 - i);
		//std::cout << thebit;
		if (thebit == 1) {
			byte |= 1 << (7 - bit);
		}
		bit++;
		TryToWriteByte(&byte, &bit);
	}
	outputBits += 32;
	SaveNode(charRoot, &byte, &bit);
	if (bit > 0) {
		//outfile.write((char*)&byte, 1);
		outputMsg[outputPos] = byte;
		outputPos++;
		outputSize++;
		outputBits += (8 - bit);
		bit = 0;
	}
	// Save tree for first byte of lz pairs
	byte = 0;
	for (int i = 0; i < 32; i++) {
		thebit = ((1 << (31 - i)) & lz1TreeSize) >> (31 - i);
		//std::cout << thebit;
		if (thebit == 1) {
			byte |= 1 << (7 - bit);
		}
		bit++;
		TryToWriteByte(&byte, &bit);
	}
	SaveNode(lz1Root, &byte, &bit);
	if (bit > 0) {
		//outfile.write((char*)&byte, 1);
		outputMsg[outputPos] = byte;
		outputPos++;
		outputSize++;
		outputBits += (8 - bit);
		bit = 0;
	}
	// Save tree for second byte of lz pairs
	byte = 0;
	for (int i = 0; i < 32; i++) {
		thebit = ((1 << (31 - i)) & lz2TreeSize) >> (31 - i);
		//std::cout << thebit;
		if (thebit == 1) {
			byte |= 1 << (7 - bit);
		}
		bit++;
		TryToWriteByte(&byte, &bit);
	}
	SaveNode(lz2Root, &byte, &bit);
	if (bit > 0) {
		//outfile.write((char*)&byte, 1);
		outputMsg[outputPos] = byte;
		outputPos++;
		outputSize++;
		outputBits += (8 - bit);
		bit = 0;
	}
	outputSize = outputPos = allTreeBytes + 4;
	outputBits = allTreeBytes * 8 + 32;
	//std::cout << "Char Tree:\n";
	//PrintCodes(charRoot, "");
	//std::cout << "LZ1 Tree:\n";
	//PrintCodes(lz1Root, "");
	//std::cout << "LZ2 Tree:\n";
	//PrintCodes(lz2Root, "");
	//std::cout << '\n';
	// Save the bitcount of the expected data
	byte = 0;
	for (int i = 0; i < 32; i++) {
		thebit = ((1 << (31 - i)) & compressedSize) >> (31 - i);
		//std::cout << thebit;
		if (thebit == 1) {
			byte |= 1 << (7 - bit);
		}
		bit++;
		outputBits++;
		TryToWriteByte(&byte, &bit);
	}

	char cBuffer = 0;
	std::string code;
	int pairs = 0;
	int chars = 0;
	uint32_t lastBit = totalBits + 32;
	//std::cout << '\n' << totalBits << ' ' << lastBit << ' ' << totalBytes << '\n';
	uint32_t bp;
	for (bp = 32; bp < lastBit; ) {
		//for (bp = 32; bp < 600; ) {
		thebit = ((1 << (7 - (bp % 8))) & (inputMsg[bp / 8])) >> (7 - (bp % 8));
		//std::cout << thebit << ", ";
		if (outputReserved - (outputBits / 8) < 100) {
			char* temp = (char*)malloc(outputSize + 1);
			memcpy(temp, outputMsg, outputSize + 1);
			free(outputMsg);
			outputReserved += 1024;
			outputMsg = (char*)malloc(outputReserved);
			memset(outputMsg, '\0', outputReserved);
			memcpy(outputMsg, temp, outputSize + 1);
		}
		outputMsg[outputBits / 8] |= (thebit << (7 - (outputBits % 8)));
		outputBits++;
		bp++;
		//std::cout << bp << ": \n";
		if (thebit == 0) {
			// already encoded as <offset:length> pair
			pairs++;
			// byte 1
			byte = 0;
			for (int i = 0; i < 8; i++) {
				thebit = ((1 << (7 - (bp % 8)) & (inputMsg[bp / 8])) >> (7 - (bp % 8)));
				//std::cout << thebit;
				byte |= (thebit << (7 - i));
				bp++;
			}
			code = lz1ToCode[byte];
			for (int i = 0; i < code.size(); i++) {
				thebit = atoi(code.substr(i, 1).c_str());
				outputMsg[outputBits / 8] |= (thebit << (7 - (bp % 8)));
				outputBits++;
			}
			// byte 2
			byte = 0;
			for (int i = 0; i < 8; i++) {
				thebit = ((1 << (7 - (bp % 8)) & (inputMsg[bp / 8])) >> (7 - (bp % 8)));
				//std::cout << thebit;
				byte |= (thebit << (7 - i));
				bp++;
			}
			code = lz2ToCode[byte];
			for (int i = 0; i < code.size(); i++) {
				thebit = atoi(code.substr(i, 1).c_str());
				outputMsg[outputBits / 8] |= (thebit << (7 - (bp % 8)));
				outputBits++;
			}
			/*
			for (int i = 0; i < 16; i++) {
				thebit = ((1 << (7 - (bp % 8)) & (inputMsg[bp / 8])) >> (7 - (bp % 8)));
				//std::cout << thebit;
				outputMsg[outputBits / 8] |= (thebit << (7 - (bp % 8)));
				bp++;
				outputBits++;
			}
			// */
			//bp--;
			//bp += 16;
		}
		else {
			// use huffman code on next 8 bits
			chars++;
			//std::cout << "\nbp: " << bp << ":" << thebit << ": ";
			char c = 0;
			for (int i = 0; i < 8; i++) {
				thebit = ((1 << (7 - (bp % 8)) & (inputMsg[bp / 8])) >> (7 - (bp % 8)));
				//std::cout << thebit;
				c |= (thebit << (7 - i));
				//outputMsg[outBits / 8] |= (thebit << (7 - (bp % 8)));
				bp++;
			}
			//bp--;
			code = charToCode[c];
			std::cout << c;
			//*
			if (code.size() == 0) {
				std::cout << "\nerror finding char code\n";
			}
			for (int i = 0; i < code.size(); i++) {
				thebit = atoi(code.substr(i, 1).c_str());
				//std::cout << thebit;
				//outputMsg[outBits / 8] |= (thebit << (7 - (outBits % 8)));
				//cBuffer |= (thebit << (7 - (outputBits % 8)));
				//TryToWriteByte((unsigned char*)&cBuffer, &bit);
				outputMsg[outputBits / 8] |= (thebit << (7 - (outputBits % 8)));
				outputBits++;
			}
			// */
			//std::cout << '\n' << c << ": " << code;
		}
	}
	outputSize = (outputBits / 8);
	outputPos = (outputBits / 8);
	if (outputBits % 8 > 0) {
		outputSize++;
		outputPos++;
	}
}

void DeflateHuffman::Decompress() {
	// inflate each tree
	// char tree
	int thebit;
	charTreeSize = 0;
	int inputBit = 0;
	for (int i = 0; i < 32; i++) {
		thebit = ((1 << (i % 8)) & inputMsg[inputBit / 8]) >> (i % 8);
			charTreeSize |= (thebit << ((3 - (i / 8) * 8 + (i % 8))));
			inputBit++;
	}
	std::cout << "char tree: " << charTreeSize << '\n';
}

void DeflateHuffman::BuildTreesForCompression() {
	std::priority_queue<DefHuffNode, std::vector<DefHuffNode*>, DefHuffNodeComp> forest;
	for (auto const& ent : charFrequencies) {
		forest.push(new DefHuffNode(ent.first, ent.second));
	}
	charTreeNodes = forest.size();
	while (!forest.empty()) {
		if (forest.size() > 1) {
			DefHuffNode* t1 = forest.top();
			forest.pop();
			DefHuffNode* t2 = forest.top();
			forest.pop();
			forest.push(new DefHuffNode(t1, t2));
		}
		else {
			charRoot = forest.top();
			forest.pop();
		}
		charTreeNodes++;
	}
	//*
	for (auto const& ent : lz1Frequencies) {
		forest.push(new DefHuffNode(ent.first, ent.second));
	}
	lz1TreeNodes = forest.size();
	while (!forest.empty()) {
		if (forest.size() > 1) {
			DefHuffNode* t1 = forest.top();
			forest.pop();
			DefHuffNode* t2 = forest.top();
			forest.pop();
			forest.push(new DefHuffNode(t1, t2));
		}
		else {
			lz1Root = forest.top();
			forest.pop();
		}
		lz1TreeNodes++;
	}
	for (auto const& ent : lz2Frequencies) {
		forest.push(new DefHuffNode(ent.first, ent.second));
	}
	lz2TreeNodes = forest.size();
	while (!forest.empty()) {
		if (forest.size() > 1) {
			DefHuffNode* t1 = forest.top();
			forest.pop();
			DefHuffNode* t2 = forest.top();
			forest.pop();
			forest.push(new DefHuffNode(t1, t2));
		}
		else {
			lz2Root = forest.top();
			forest.pop();
		}
		lz2TreeNodes++;
	}
	// */
}

void DeflateHuffman::SaveNode(DefHuffNode* node, unsigned char *byte, int* bit) {
	//if (node->data == '\0') {
	if (node->left != NULL) {
		(*bit)++;
		//outputBits++;
		TryToWriteByte(byte, bit);
		SaveNode(node->left, byte, bit);
		SaveNode(node->right, byte, bit);
	}
	else {
		*byte |= (1 << (*bit));
		(*bit)++;
		outputBits++;
		TryToWriteByte(byte, bit);
		AddChar(node->data, byte, bit);
	}
}

void DeflateHuffman::AddChar(char c, unsigned char *byte, int* bit) {
	outputBits += 8;
	if (*bit == 0) {
		*byte = c;
		*bit = 8;
		TryToWriteByte(byte, bit);
		return;
	}
	int thebit;
	for (int i = 0; i < 8; i++) {
		thebit = (c & (1 << i)) >> i;
		if (thebit == 1) {
			*byte |= (1 << *bit);
		}
		(*bit)++;
		TryToWriteByte(byte, bit);
	}
}

void  DeflateHuffman::TryToWriteByte(unsigned char *byte, int* bit) {
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

void DeflateHuffman::StoreCodesAndLengths(DefHuffNode* node, std::string code, int mode) {
	if (mode > 0) {
		if (node->data == '\0') {
			if (node->left != NULL) {
				StoreCodesAndLengths(node->left, code + "0", mode);
			}
			if (node->right != NULL) {
				StoreCodesAndLengths(node->right, code + "1", mode);
			}
		}
		else {
			if (mode == 1) {
				lz1ToCode[node->data] = code;
				lz1Lengths[node->data] = code.length();
			}
			else {
				lz2ToCode[node->data] = code;
				lz2Lengths[node->data] = code.length();
			}
		}
	}
	else {
		if (node->data == '\0') {
			if (node->left != NULL) {
				StoreCodesAndLengths(node->left, code + "0");
			}
			if (node->right != NULL) {
				StoreCodesAndLengths(node->right, code + "1");
			}
		}
		else {
			charToCode[node->data] = code;
			charLengths[node->data] = code.length();
		}
	}
}

void DeflateHuffman::PrintCodes(DefHuffNode* node, std::string code, bool lz) {
	if (node->data == '\0') {
		if (node->left != NULL) {
			PrintCodes(node->left, code + "0");
		}
		if (node->right != NULL) {
			PrintCodes(node->right, code + "1");
		}
	}
	else {
		std::cout << node->data << ":\t" << node->freq;
		std::cout << '\t' << code.length() << '\t' << code << '\n';
	}
}

int DeflateHuffman::CalcCompressedSize() {
	// data
	//totalBits = inputSize * 8;
	uint32_t databits = totalBits;
	//uint32_t databits = 0;
	for (auto const& ent : charFrequencies) {
		databits -= (ent.second * (8 - charLengths[ent.first]));
		//databits += (ent.second * (charLengths[ent.first]) + 1);
	}
	for (auto const& ent : lz1Frequencies) {
		databits -= (ent.second * (8 - lz1Lengths[ent.first]));
		//databits += (ent.second * (lz1Lengths[ent.first]) + 1);
	}
	for (auto const& ent : lz2Frequencies) {
		databits -= (ent.second * (8 - lz2Lengths[ent.first]));
		//databits += (ent.second * (lz2Lengths[ent.first]) + 1);
	}
	compressedSize = databits;
	databits += 32; // to store the size of the data
	int databytes = databits / 8;
	if (databits % 8 > 0) {
		databytes++;
	}

	// treesize
	uint32_t chartreebits = charTreeNodes + charFrequencies.size() * 8 + 32;
	charTreeSize = chartreebits - 32;
	int chartreebytes = chartreebits / 8;
	if (chartreebits % 8 > 0) {
		chartreebytes++;
	}
	uint32_t lz1treebits = lz1TreeNodes + lz1Frequencies.size() * 8 + 32;
	lz1TreeSize = lz1treebits - 32;
	int lz1treebytes = lz1treebits / 8;
	if (lz1treebits % 8 > 0) {
		lz1treebytes++;
	}
	uint32_t lz2treebits = lz2TreeNodes + lz2Frequencies.size() * 8 + 32;
	lz2TreeSize = lz2treebits - 32;
	int lz2treebytes = lz2treebits / 8;
	if (lz2treebits % 8 > 0) {
		lz2treebytes++;
	}
	allTreeBytes = chartreebytes + lz1treebytes + lz2treebytes;
	// total size
	int totalbits = databits + chartreebits + lz1treebits + lz2treebits;
	int totalbytes = compressedBytes = databytes + allTreeBytes;
	double percent = (double)totalbytes / inputSize * 100;
	double ratio = (double)inputSize / totalbytes;
	//*
	std::cout << "\nOriginal data size:\t" << totalBits << '\t' 
		<< inputSize * 8 << " bits;\t" << inputSize << " bytes\n";
	std::cout << "Compressed data size:\t" << databits << " bits;\t"
		<< databytes << " bytes\n";
	std::cout << "Space needed for char tree:\t" << chartreebits << " bits;\t"
		<< chartreebytes << " bytes\n";
	std::cout << "Space needed for lz1 tree:\t" << lz1treebits << " bits;\t"
		<< lz1treebytes << " bytes\n";
	std::cout << "Space needed for lz2 tree:\t" << lz2treebits << " bits;\t"
		<< lz2treebytes << " bytes\n";
	std::cout << "Total compressed size:\t" << totalbits << " bits; \t"
		<< totalbytes << " bytes\n";
	std::cout << "Compression ratio:\t"
		<< std::fixed << std::setprecision(3) << ratio << '\t'
		<< std::fixed << std::setprecision(2) << percent << "%\n";
	// */
	return totalbits;
}

DeflateHuffman::~DeflateHuffman() {
	;
}
