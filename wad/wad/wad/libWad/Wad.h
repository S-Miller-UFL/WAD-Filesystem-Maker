



//Wad.h
/*
* Author: Steven Miller
* Date created: November 16, 2023
* Purpose: Functions for wad file operations
* Used by: wad.cpp
* Uses: filestruct.h, nary_tree.h
*/


#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <cctype>
#include <string>
#include <string.h>
#include <cstring>
#include <stack>
#include <unordered_map>
#include "filestruct.h"
#include "nary_tree.h"
class Wad
{
private:

	//private functions
	Wad(const std::string&);
	std::string create_stringfromstack(std::stack<std::string>);
	std::stack<std::string>* create_stackfromstring(std::string);
	std::unordered_map <std::string, wad_file_struct*> file_map;
	void write_wadfile_descriptor(wad_struct*, uint32_t);

	//private variables
	std::string wad_magic;
	uint32_t wad_numberofdescriptors;
	uint32_t wad_descriptoroffset;
	std::fstream wad_fstream;
	std::string wad_path = "";
	uint32_t file_size = 0;
	nary_tree* file_tree;
public:
	//your constructor
	static Wad* loadWad(const std::string& );

	//get magic descriptor
	std::string getMagic();

	//given a path, if it represents a content file, return true.
	//if its a directory or if it just doesnt exist, return false.
	//the map you, hopefully, created in your "loadwad" function would be really convenient
	//here, dont you think?
	bool isContent(const std::string& );

	//if its a valid file or an invalid path, return false
	bool isDirectory(const std::string& );
	//return size of content file
	//otherwise, return -1
	int getSize(const std::string& );
	
	//if path represents content, copy as many bytes as are available
	int getContents(const std::string& , char* , int , int offset = 0);
	
	//if path represents directory, get its children
	int getDirectory(const std::string& , std::vector<std::string>* );


	//create a dirctory and write it to our wad file
	void createDirectory(const std::string& );
	 
	//create a BLANK file and write it to our wad
	void createFile(const std::string& );
	 
	//write contents in buffer to EMPTY file
	int writeToFile(const std::string&, const char*, int , int offset = 0);

};