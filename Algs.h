
#pragma once

#include "Huffman.h"
#include "LZ77.h"
//#include "Deflate.h"
//#include <zlib1>

class Algs {
public:
	Huffman huff;
	LZ77 lz77;
	//Deflate dflt;
	//zlib zlibDflt;

	Algs();
	~Algs();
};

