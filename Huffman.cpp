/*
	Author: Jason Bogle
	Class: Analysis of Algorithms
	Assignment: The big project
	File: Huffman.cpp
	Description: This file implements the Huffman class
*/

#include "Huffman.h"
#include<iostream>
#include<iomanip>
#include<fstream>
#include<string>
#include<sstream>
#include<queue>
#include<bitset>
#include<algorithm>


Huffman::Huffman() {
	root = NULL;
	totalBytes = 0;
	treeNodes = 0;
}

Huffman::Huffman(std::string infilename) {
	root = NULL;
	totalBytes = 0;
	treeNodes = 0;
	infile.open(infilename, std::ios::in | std::ios::binary);
	outfile.open(infilename + "_out", std::ios::out | std::ios::binary);
	CountChars();
}

Huffman::Huffman(std::string infilename, std::string outfilename) {
	root = NULL;
	totalBytes = 0;
	treeNodes = 0;
	infile.open(infilename, std::ios::in | std::ios::binary);
	outfile.open(outfilename, std::ios::out | std::ios::binary);
	CountChars();
}

void Huffman::SetInputFile(std::string inputfilename) {
	free(inputMsg);
	infile.open(inputfilename, std::ios::in | std::ios::binary);
	infile.seekg(0, infile.end);
	inputSize = infile.tellg();
	infile.seekg(infile.beg);
	inputMsg = (char*)malloc(inputSize + 1);
	memset(inputMsg, '\0', inputSize + 1);
	infile.read(inputMsg, inputSize);
	infile.seekg(infile.beg);
	frequencies.clear();
	lengths.clear();
	chartocode.clear();
	codetochar.clear();
	treesize = 0;
	CountChars();
}

void Huffman::SetOutputFile(std::string outputfilename) {
	outfile.open(outputfilename, std::ios::out | std::ios::binary);
}

void Huffman::CloseInputFile() {
	infile.close();
}

void Huffman::CloseOutputFile() {
	outfile.close();
}


void Huffman::Compress() {
	outputPos = 0;
	memset(outputMsg, '\0', outputReserved);
	outputSize = 0;
	//CountChars();
	BuildTreeForCompression();
	//PrintCodes(root, "");
	StoreCodesAndLengths(root, "");
	CalcCompressedSize();
	WriteCompressedFile();
}


void Huffman::Decompress() {
	outputPos = 0;
	memset(outputMsg, '\0', outputSize);
	outputSize = 0;
	InflateTree();
	//PrintCodes(root, "");
	//StoreChars(root, "");
	DecryptMsg();
}

void Huffman::PrintCodes(HuffNode* node, std::string code) {
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

void Huffman::StoreCodesAndLengths(HuffNode* node, std::string code) {
	if (node->data == '\0') {
		if (node->left != NULL) {
			StoreCodesAndLengths(node->left, code + "0");
		}
		if (node->right != NULL) {
			StoreCodesAndLengths(node->right, code + "1");
		}
	}
	else {
		chartocode[node->data] = code;
		lengths[node->data] = code.length();
	}
}

void Huffman::StoreChars(HuffNode* node, std::string code) {
	if (node->data == '\0') {
		if (node->left != NULL) {
			StoreChars(node->left, code + "0");
		}
		if (node->right != NULL) {
			StoreChars(node->right, code + "1");
		}
	}
	else {
		codetochar[code] = node->data;
	}
}

void Huffman::CountChars() {
	// get total chars
	//infile.seekg(0, infile.end);
	//totalBytes = infile.tellg();
	totalBytes = inputSize;
	// now get the frequency of each char
	/*
	infile.seekg(0, infile.beg);
	char byte_buffer;
	for (int i = 0; i < totalBytes; i++) {
		infile.read(&byte_buffer, 1);
		if (frequencies.find(byte_buffer) != frequencies.end()) {
			frequencies[byte_buffer]++;
		}
		else {
			frequencies[byte_buffer] = 1;
		}
	} 
	infile.seekg(0, infile.beg);
	// */
	for (int i = 0; i < totalBytes; i++) {
		if (frequencies.find(inputMsg[i]) == frequencies.end()) {
			frequencies[inputMsg[i]] = 1;
		}
		else {
			frequencies[inputMsg[i]]++;
		}
	}
}

void Huffman::BuildTreeForCompression() {
	// create forest with priority queue
	std::priority_queue<HuffNode, std::vector<HuffNode*>, HuffNodeComp> forest;
	for (auto const& ent : frequencies) {
		forest.push(new HuffNode(ent.first, ent.second));
	}
	treeNodes = forest.size();
	while (!forest.empty()) {
		if (forest.size() > 1) {
			HuffNode* t1 = forest.top();
			forest.pop();
			HuffNode* t2 = forest.top();
			forest.pop();
			forest.push(new HuffNode(t1, t2));
		}
		else {
			root = forest.top();
			forest.pop();
		}
		treeNodes++;
	}
}

void Huffman::WriteCompressedFile() {
	unsigned char byte = 0;
	int bit = 0;
	int thebit;
	//*
	for (int i = 0; i < 32; i++) {
		thebit = ((1 << (31 - i)) & treesize) >> (31 - i);
		//std::cout << thebit;
		if (thebit == 1) {
			byte |= 1 << (7 - bit);
		}
		bit++;
		TryToWriteByte(&byte, &bit);
	}
	SaveNode(root, &byte, &bit);
	if (bit > 0) {
		//outfile.write((char*)&byte, 1);
		outputMsg[outputPos] = byte;
		outputPos++;
		outputSize++;
		bit = 0;
		//std::cout << "\nwrote last byte\n";
		for (int i = 0; i < 8; i++) {
			thebit = ((1 << (7 - i)) & byte) >> (7 - i);
			//std::cout << thebit;
		}
	}
	int s = outputSize;//outfile.tellp();
	//std::cout << "\nsize: " << s << '\n';
	byte = 0;
	// */
	//std::cout << '\n' << compressedsize << '\n';
	for (int i = 0; i < 32; i++) {
		thebit = ((1 << (31 - i)) & compressedsize) >> (31 - i);
		//std::cout << thebit;
		if (thebit == 1) {
			byte |= 1 << (7 - bit);
		}
		bit++;
		TryToWriteByte(&byte, &bit);
	}
	WriteCompressedData(&byte, &bit);
	if (bit > 0) {
		//outfile.write((char*)&byte, 1);
		outputMsg[outputPos] = byte;
		outputPos++;
		outputSize++;
	}
}

void Huffman::SaveNode(HuffNode* node, unsigned char *byte, int* bit) {
	if (node->data == '\0') {
		(*bit)++;
		TryToWriteByte(byte, bit);
		SaveNode(node->left, byte, bit);
		SaveNode(node->right, byte, bit);
	}
	else {
		*byte |= (1 << (*bit));
		(*bit)++;
		TryToWriteByte(byte, bit);
		AddChar(node->data, byte, bit);
	}
}

void  Huffman::WriteCompressedData(unsigned char *byte, int* bit) {
	//infile.seekg(infile.beg);
	//char* data = (char*)malloc(totalBytes + 1);
	//infile.read(data, totalBytes);
	std::string code;
	//bool found;
	int thebit;
	for (uint32_t i = 0; i < totalBytes; i++) {
		//found = false;
		//code = GetCode(data[i], root, "", &found);
		//code = chartocode[data[i]];
		code = chartocode[inputMsg[i]];
		for (int n = 0; n < code.length(); n++) {
			thebit = atoi(code.substr(n,1).c_str());
			if (thebit == 1) {
				*byte |= (1 << (7 - *bit));
			}
			(*bit)++;
			TryToWriteByte(byte, bit);
		}
	}
}

void  Huffman::TryToWriteByte(unsigned char *byte, int* bit) {
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

void Huffman::AddChar(char c, unsigned char *byte, int* bit) {
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

std::string Huffman::GetCode(char c, HuffNode* node, 
	std::string code, bool* found)
{
	if (node->data == c) {
		*found = true;
		return code;
	}
	std::string codetemp;
	if (node->left != NULL && node->left->freq >= frequencies[c]) {
		codetemp = GetCode(c, node->left, code + "0", found);
		if (*found) {
			return codetemp;
		}
	}
	if (node->right != NULL && node->right->freq >= frequencies[c]) {
		codetemp = GetCode(c, node->right, code + "1", found);
		if (*found) {
			return codetemp;
		}
	}
	return "";
}

void Huffman::InflateTree()
{
	unsigned char *buffer = (unsigned char*)malloc(4 * sizeof(unsigned char));
	// get the tree size (stored as a uint32_t)
	//infile.read((char*)buffer, 4);
	memcpy(buffer, inputMsg, 4);
	//infile.read((char*)&treesize, 4);
	int bit = 0;
	int thebit = 0;
	int buffbyte = 0;
	//*
	//std::cout << "\ntreesize: ";
	treesize = 0;
	for (int i = 0; i < 32; i++) {
		if (bit == 8) {
			bit = 0;
			buffbyte++;
		}
		thebit = ((1 << bit) & buffer[buffbyte]) >> bit;
		//std::cout << thebit;
		if (thebit == 1) {
			treesize |= (thebit << ((3-buffbyte) * 8 + (i % 8)));
		}
		bit++;
	}
	// */

	// read the tree
	//treesize += treesize % 8;
	//free(buffer);
	buffer = (unsigned char*)malloc(treesize / sizeof(char) + 1);
	memset(buffer, 0, treesize / sizeof(char) + 1);
	//infile.read((char*)buffer, treesize / sizeof(char));
	memcpy(buffer, &(inputMsg[4]), treesize / sizeof(char));

	// create nodes
	bit = 0;
	buffbyte = 0;
	unsigned char byte = 0;
	root = GenNode(buffer, &buffbyte, &byte, &bit, "");
}

HuffNode* Huffman::GenNode(unsigned char* buffer, int* buffbyte, 
	unsigned char* byte, int* bit, std::string code)
{
	int thebit = ((1 << (*bit)) & (buffer[(*buffbyte)])) >> (*bit);
	(*byte) |= (thebit << (*bit));
	(*bit)++;
	if (*bit == 8) {
		(*byte) = 0;
		(*buffbyte)++;
		(*bit) = 0;
	}
	if (thebit == 1) {
		char c = ReadByte(buffer, buffbyte, byte, bit);
		return new HuffNode(c);
	}
	else {
		HuffNode* left = GenNode(buffer, buffbyte, byte, bit, code + "0");
		HuffNode* right = GenNode(buffer, buffbyte, byte, bit, code + "1");
		return new HuffNode(left, right);
	}
	return NULL;
}

char Huffman::ReadByte(unsigned char* buffer, int* buffbyte,
	unsigned char* byte, int* bit)
{
	unsigned char c = 0;
	int thebit;
	if ((*bit) == 0) {
		c = buffer[(*buffbyte)];
		(*buffbyte)++;
	}
	else {
		for (int i = 0; i < sizeof(unsigned char)*8; i++) {
			thebit = ((1 << (*bit)) & (buffer[(*buffbyte)])) >> (*bit);
			c |= thebit << i;
			(*byte) |= thebit << (*bit);
			(*bit)++;
			if ((*bit) == 8) {
				(*buffbyte)++;
				(*byte) = 0;
				(*bit) = 0;
			}
		}
	}
	return c;
}

void Huffman::DecryptMsg() {
	//std::cout << '\n';
	unsigned char *buffer = (unsigned char*)malloc(5 * sizeof(unsigned char));
	// get the tree size (stored as a uint32_t)
	int treebytes = (treesize / 8) + 4;
	if (treesize % 8 > 0) {
		treebytes++;
	}
	//std::cout << "tree: " << treesize << ' ' << treebytes << '\n';
	//infile.seekg(infile.beg);
	//infile.seekg(treebytes, infile.beg);
	//infile.read((char*)buffer, 4);
	memcpy((char*)buffer, &(inputMsg[treebytes]), 4);
	//infile.read((char*)&compressedsize, 4);
	int bit = 0;
	int thebit = 0;
	int buffbyte = 0;
	//*
	compressedsize = 0;
	for (int i = 0; i < 32; i++) {
		if (bit == 8) {
			bit = 0;
			buffbyte++;
		}
		thebit = ((1 << bit) & buffer[buffbyte]) >> bit;
		//std::cout << thebit;
		if (thebit == 1) {
			compressedsize |= (thebit << ((3 - buffbyte) * 8 + (i % 8)));
		}
		bit++;
	}
	int compressedbytes = (compressedsize / 8);
	if (compressedsize % 8 > 0) {
		compressedbytes++;
	}
	int lastbit = ((treebytes + 4) * 8) + compressedsize;
	//std::cout << '\n' << compressedsize << ' ' << compressedbytes << ' ' << lastbit << '\n';
	// */
	//unsigned char* msg = (unsigned char*)malloc(compressedbytes + 1);
	//memset(msg, '\0', compressedbytes);
	//infile.read((char*)msg, compressedbytes);
	buffbyte = 0;
	bit = 0;
	HuffNode *node = root;
	for (uint32_t i = (treebytes + 4) * 8; i < lastbit; i++) {
		thebit = ((1 << (7 - (bit))) & (inputMsg[i / 8])) >> (7 - (bit));
		//std::cout << thebit;
		if (thebit == 1) {
			node = node->right;
		}
		else {
			node = node->left;
		}
		if (node->data != '\0') {
			//TryToWriteByte(&byte, &bit);
			if (outputReserved - outputSize < 100) {
				char* temp = (char*)malloc(outputSize + 1);
				memcpy(temp, outputMsg, outputSize + 1);
				free(outputMsg);
				outputReserved += 1024;
				outputMsg = (char*)malloc(outputReserved);
				memset(outputMsg, '\0', outputReserved);
				memcpy(outputMsg, temp, outputSize + 1);
			}
			outputMsg[outputPos] = node->data;
			outputPos++;
			outputSize++;
			node = root;
		}
		bit++;
		if (bit == 8) {
			buffbyte++;
			bit = 0;
		}
	}
	//std::cout << outputMsg << '\n';
	//outfile.write(decodedmsg.c_str(), decodedmsg.size());
}

char Huffman::SearchNode(HuffNode* node, std::string code, bool* found) {
	//std::string copycode = code.substr(0, code.size());
	(*found) = false;
	if (code.size() == 0) {
		if (node->data != '\0') {
			(*found) = true;
		}
		return node->data;
	}
	if (node->data != '\0') {
		return 0;
	}
	char child = code[0];
	code.erase(0, 1);
	if (child == '0') {
		return SearchNode(node->left, code, found);
	}
	return SearchNode(node->right, code, found);
}

int Huffman::CalcCompressedSize() {
	// calc compressed data size
	uint32_t databits = 0;
	for (auto const& ent : frequencies) {
		databits += ent.second * lengths[ent.first];
	}
	compressedsize = databits;
	databits += 32; // to store the size of the data
	int databytes = databits / 8;
	if (databits % 8 > 0) {
		databytes++;
	}

	// calc space needed for tree
	int treebits = treeNodes + frequencies.size() * 8;
	treesize = treebits;
	treebits += 32; // to store the size of the tree
	int treebytes = treebits / 8;
	if (treebits % 8 > 0) {
		treebytes++;
	}
	// total size
	int totalbits = compressedsize + treebits;
	int totalbytes = databytes + treebytes;
	double percent = (double)totalbytes / totalBytes * 100;
	double ratio = (double)totalBytes / totalbytes;
	/*
	std::cout << "\nOriginal data size:\t" << inputSize << '\t' << totalBytes * 8 << " bits;\t";
	std::cout << totalBytes << " bytes\n";
	std::cout << "Compressed data size:\t" << compressedsize << " bits;\t";
	std::cout << databytes << " bytes\n";
	std::cout << "Space needed for tree:\t" << treebits << " bits;\t";
	std::cout << treebytes << " bytes\n";
	std::cout << "Total compressed size:\t" << totalbits << " bits; \t";
	std::cout << totalbytes << " bytes\n";
	std::cout << "Compression ratio:\t" 
		<< std::fixed << std::setprecision(3) << ratio << '\t'
		<< std::fixed << std::setprecision(2) << percent << "%\n";
	// */
	return totalbits;
}

void Huffman::WriteToOutputFile() {
	outfile.write(outputMsg, outputSize);
	//for (int w = 0; w < outputMsg.size()/**/; w += 4095) {
	//	int len = std::min(4095, (int)(outputMsg.size() - w));
	//	outfile.write(outputMsg.substr(w, len).c_str(), len);
	//}
}

void Huffman::SetInput(char* inMsg, uint32_t size) {
	free(inputMsg);
	inputSize = size;
	inputMsg = (char*)malloc(size + 1);
	memset(inputMsg, '\0', size + 1);
	memcpy(inputMsg, inMsg, size);
	frequencies.clear();
	lengths.clear();
	chartocode.clear();
	codetochar.clear();
	treesize = 0;
	CountChars();
}

char* Huffman::GetInput() {
	return inputMsg;
}

uint32_t Huffman::GetInputLength() {
	return inputSize;
}

char* Huffman::GetOutput() {
	return outputMsg;
}

uint32_t Huffman::GetOutputLength() {
	return outputSize;
}

Huffman::~Huffman() {
	delete root;
	infile.close();
	outfile.close();
}