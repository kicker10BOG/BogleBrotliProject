/*
	Author: Jason Bogle
	Class: Analysis of Algorithms
	Assignment: The big project
	File: Main.cpp
	Description: This is the main file. It brings all of the algorithms 
		together and executes certain ones based on command line argument.
*/

// constants
#define B_TEST 0
#define B_HUFF 1
#define B_LZ77 2
#define B_DFLT 3
#define B_BROT 4

#include "Huffman.h"
#include "LZ77.h"
#include "Deflate.h"
#include "Algs.h"

#include<windows.h>
#include<vector>
#include<string>
#include<iostream>
#include<fstream>
#include<ctime>
#include <chrono>

std::vector<std::string> GetFileNamesIn(std::string folder);

int main(int argc, char** argv) {
	// run test on Canterbury Corpus , my drawing app, and my groupme bot
	Algs alg;
	std::ofstream res("results.csv");
	int origSize, compressedSize, uncompressedSize;
	res << "File,Alogrithm,Original Size,Compressed Size,Uncompressed Size"
		<< ",Compression Time,Decompression Time\n";
	std::ifstream tfile;
	system("rm -rf huff_out/*");
	system("rm -rf lz77_out/*");
	system("rm -rf brotli_out/*");
	system("rm -rf deflate_out/*");
	std::vector<std::string> dirs;
	std::string cmd;
	uint32_t compressTime, decompressTime;
	dirs.push_back("cantrbry");
	dirs.push_back("paint");
	dirs.push_back("bot");
	for (std::string dir : dirs) {
		cmd = "mkdir huff_out\\" + dir;
		system(cmd.c_str());
		cmd = "mkdir huff_out\\" + dir + "\\compressed";
		system(cmd.c_str());
		cmd = "mkdir huff_out\\" + dir + "\\decompressed";
		system(cmd.c_str());
		cmd = "mkdir lz77_out\\" + dir;
		system(cmd.c_str());
		cmd = "mkdir lz77_out\\" + dir + "\\compressed";
		system(cmd.c_str());
		cmd = "mkdir lz77_out\\" + dir + "\\decompressed";
		system(cmd.c_str());
		cmd = "mkdir brotli_out\\" + dir;
		system(cmd.c_str());
		cmd = "mkdir brotli_out\\" + dir + "\\compressed";
		system(cmd.c_str());
		cmd = "mkdir brotli_out\\" + dir + "\\decompressed";
		system(cmd.c_str());
		cmd = "mkdir deflate_out\\" + dir;
		system(cmd.c_str());
		cmd = "mkdir deflate_out\\" + dir + "\\compressed";
		system(cmd.c_str());
		cmd = "mkdir deflate_out\\" + dir + "\\decompressed";
		system(cmd.c_str());
		std::vector<std::string> files = GetFileNamesIn(dir);
		for (std::string f : files) {
			//std::cout << f << '\n';
			std::string file = dir + "/" + f;
			std::cout << file << '\n';
			tfile.open(file.c_str());
			tfile.seekg(0, tfile.end);
			origSize = tfile.tellg();
			tfile.close();
			// Huffman
			auto start = std::chrono::steady_clock::now();
			alg.huff.SetInputFile(file);
			alg.huff.SetOutputFile("./huff_out/" + dir + "/compressed/" + f + ".compressed");
			alg.huff.Compress();
			alg.huff.WriteToOutputFile();
			alg.huff.CloseInputFile();
			alg.huff.CloseOutputFile();
			auto end = std::chrono::steady_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
			compressTime = elapsed.count();
			compressedSize = alg.huff.GetOutputLength();

			start = std::chrono::steady_clock::now();
			alg.huff.SetInputFile("./huff_out/" + dir + "/compressed/" + f + ".compressed");
			alg.huff.SetOutputFile("./huff_out/" + dir + "/decompressed/" + f);
			alg.huff.Decompress();
			alg.huff.WriteToOutputFile();
			alg.huff.CloseInputFile();
			alg.huff.CloseOutputFile();
			end = std::chrono::steady_clock::now();
			elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
			decompressTime = elapsed.count();
			uncompressedSize = alg.huff.GetOutputLength();
			res << file << ",Huffman," << origSize << ',' << compressedSize
				<< ',' << uncompressedSize << ',' << compressTime << ','
				<< decompressTime << '\n';

			// LZ77
			start = std::chrono::steady_clock::now();
			alg.lz77.SetInputFile(file);
			alg.lz77.SetOutputFile("./lz77_out/" + dir + "/compressed/" + f + ".compressed");
			alg.lz77.Compress();
			alg.lz77.WriteToOutputFile();
			alg.lz77.CloseInputFile();
			alg.lz77.CloseOutputFile();
			end = std::chrono::steady_clock::now();
			elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
			compressTime = elapsed.count();
			compressedSize = alg.lz77.GetOutputLength();

			start = std::chrono::steady_clock::now();
			alg.lz77.SetInputFile("./lz77_out/" + dir + "/compressed/" + f + ".compressed");
			alg.lz77.SetOutputFile("./lz77_out/" + dir + "/decompressed/" + f);
			alg.lz77.Decompress();
			alg.lz77.WriteToOutputFile();
			alg.lz77.CloseInputFile();
			alg.lz77.CloseOutputFile();
			end = std::chrono::steady_clock::now();
			elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
			decompressTime = elapsed.count();
			uncompressedSize = alg.lz77.GetOutputLength();
			res << file << ",LZ77," << origSize << ',' << compressedSize
				<< ',' << uncompressedSize << ',' << compressTime << ','
				<< decompressTime << '\n';

			// Brotli
			std::string bCmd = "Brotli.exe --in " + file + " --out ./brotli_out/" + dir + "/compressed/" + f + ".compressed";
			//std::cout << bCmd << '\n';
			start = std::chrono::steady_clock::now();
			system(bCmd.c_str());
			end = std::chrono::steady_clock::now();
			elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
			compressTime = elapsed.count();
			std::string name = "./brotli_out/" + dir + "/compressed/" + f + ".compressed";
			tfile.open(name.c_str());
			tfile.seekg(0, tfile.end);
			compressedSize = tfile.tellg();
			tfile.close();

			bCmd = "Brotli.exe --decompress --in ./brotli_out/" + dir + "/compressed/" + f + ".compressed --out ./brotli_out/" + dir + "/decompressed/" + f;
			start = std::chrono::steady_clock::now();
			system(bCmd.c_str());
			end = std::chrono::steady_clock::now();
			elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
			decompressTime = elapsed.count();
			name = "./brotli_out/" + dir + "/decompressed/" + f;
			tfile.open(name);
			tfile.seekg(0, tfile.end);
			uncompressedSize = tfile.tellg();
			tfile.close();
			res << file << ",Brotli," << origSize << ',' << compressedSize
				<< ',' << uncompressedSize << ',' << compressTime << ','
				<< decompressTime << '\n';

			// Deflate - using 7zip
			std::string zCmd = "7z.exe a -bso0 -bsp0 deflate_out/" + dir + "/compressed/" + f + ".zip " + file;
			//std::cout << zCmd << '\n';
			start = std::chrono::steady_clock::now();
			system(zCmd.c_str());
			end = std::chrono::steady_clock::now();
			elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
			compressTime = elapsed.count();
			name = "./deflate_out/" + dir + "/compressed/" + f + ".zip";
			tfile.open(name.c_str());
			tfile.seekg(0, tfile.end);
			compressedSize = tfile.tellg();
			tfile.close();

			zCmd = "7z.exe e -bso0 -bsp0 -odeflate_out/" + dir + "/decompressed deflate_out/" + dir + "/compressed/" + f + ".zip";
			//std::cout << zCmd << '\n';
			start = std::chrono::steady_clock::now();
			system(zCmd.c_str());
			end = std::chrono::steady_clock::now();
			elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
			decompressTime = elapsed.count();
			name = "./deflate_out/" + dir + "/decompressed/" + f;
			tfile.open(name);
			tfile.seekg(0, tfile.end);
			uncompressedSize = tfile.tellg();
			tfile.close();
			res << file << ",Deflate," << origSize << ',' << compressedSize
				<< ',' << uncompressedSize << ',' << compressTime << ','
				<< decompressTime << '\n';
		}
	}
	return 0;
}

std::vector<std::string> GetFileNamesIn(std::string folder)
{
	std::vector<std::string> names;
	std::string search_path = folder + "/*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}
