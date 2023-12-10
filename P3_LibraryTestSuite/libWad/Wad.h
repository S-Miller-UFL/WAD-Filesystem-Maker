



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
#include <map>
#include "filestruct.h"
#include "nary_tree.h"
class Wad
{
private:

	char wad_magic[4];
	uint32_t wad_numberofdescriptors;
	uint32_t wad_descriptoroffset;
	std::fstream wad_fstream;
	std::string wad_path = "";

	Wad(const std::string&);
	std::string create_stringfromstack(std::stack<std::string>);
	std::stack<std::string>* create_stackfromstring(std::string);
	std::vector<uint8_t> read_whole_file(std::string);
	std::map < std::string, wad_file_struct*> file_map;
	nary_tree* file_tree;

	//test function, dont include in submission
	void print_map();
	//std::stack<std::string> file_stack;
public:
	//your constructor
	/*
	* youll be given a path string
	* that path string represents a wad file on your real file system on your VM
	* youll need to load this wad file
	* 
	* will probably be your longest function
	* 
	* you need to do load the descriptors from the wad file and organize it so it can be represented in memory
	* 
	* specifically, you need to represent your file system as a data structure
	* you need to traverse, look through its descriptors
	* and return its information
	* 
	* the best way to think about this, is that youre taking a flattened tree, and organizing it into an actual tree

	//going back to our example in project3.cpp
	
	* your directory will look like the following:
	* 
	*   X
	*   |
	*   -->Y
	*     |
	*     |-->E1M1
	*     |  |
	*     |  |-->math.obj
	*     |  |
	*     |  |-->polygon.c
	*     |
	*     |-->Z
	* 
	* notice that X, our root, has a child.
	* X's child has two children of its own
	* and one of Y's children has 2 children of its own!
	*
	* you wanna organize it so:
	*
	* you have 1 primary element which can have any amount of children
	* those n children can have any amount of children of its own
	*
	*
	* this is an N-ary tree
	*
	I.E.: use an N-ary tree to represent your file directory!
	*

	* now, notice that you need to keep track of directory nesting.
	* 
	* say you finish putting things in E1M1, how will you put things into Z?
	* 
	* youll need a temporary data structure inside your loadwad to traverse your directories
	* 
	* once youre done with directory E1M1, youll switch to Z, and load your data structure with Z's contents.
	
	* your temporary data structure can be approached in two ways:
	* use a stack:
	*	start at begininning of WAD file
		top of the stack is your current directory, create a new node and add it to your data structure
	*	look at the file descriptor, if you see a namespace, create a new node and add it to your data structure,add it to the top of the stack
	*	once you see E#M#, realize that the next 10 file descriptors will belong to E#M#, so dont add them to the stack
	*	instead, just create a new E#M# node that is a child of the previous file descriptor.
	*	add the next 10 elements as children to the E#M# node.
	*	once counter hits 10 or you hit a start or an end, pop E#M# off the stack
	*	go to E#M#'s parent, go to next file descriptor
	*	add the file descriptor you find to the parents children
	*	pop the file descriptor off the stack if you encounter its end
	* use recursion
	* 
	* either is valid, use whichever is easier
	*
	* another thing to think about:
	* with the stack implementation, you are accidentally creating a path
	* it would be a good idea to create a map alongside your tree data structure
	* 
	* when you add your file to your directory, store its directory as a key to the map.
	* and store the file struct as its value.
	*/
	static Wad* loadWad(const std::string& path);
	//this is very very simple
	/*
	* its just a getter, just get the magic descriptor
	*/
	std::string getMagic();

	//given a path, if it represents a content file, return true.
	//if its a directory or if it just doesnt exist, return false.
	//the map you, hopefully, created in your "loadwad" function would be really convenient
	//here, dont you think?
	bool isContent(const std::string& path);

	//if its a valid file or an invalid path, return false
	bool isDirectory(const std::string& path);
	//return size of content file
	//otherwise, return -1
	int getSize(const std::string& path);
	
	//if path represents content, copy as many bytes as are available
	/*
	* given a path, check if its a path, if it isnt, return -1
	* 
	* else, given a pointer to a buffer and a integer representing length, and an optional parameter
	* 
	* look at the lump data of the file, and copy length amount of bytes into the buffer.
	* 
	* youre going to go to position <offset>, read <length> bytes from it, then copy
	* 
	* those <length> bytes to the buffer pointer, then return it.
	* 
	* lets say the length provided is longer than the content files actual length
	* 
	* compare the requested length with the file structs length
	* 
	* if <length> > struct<length>, return the number of bytes you can read.
	* 
	* else, return the number of bytes you read. in the normal case, <length
	* 
	* but say the offset is incompatible with the length, for example, length of file is 5,
	* offset is passed in as 1, <length> is passed in as 5.
	* 
	* read from the <offset> + filestruct<offset> as you normally would, but you can only copy
	* a portion of the file. in this case, 5-1 = 4.
	* 
	* return number of bytes you were able to copy.
	*/
	int getContents(const std::string& path, char* buffer, int length, int offset = 0);
	/*
	* if the path you pass in exists, go to its node in your n - ary tree and look at its children
	* add the names of its children into the directory* vector passed in, and return the number
	* of children you added
	* 
	* since its passed in by pointer, you dont need to worry about preserving data
	*/
	int getDirectory(const std::string& path, std::vector<std::string>* directory);


	//this is one of those tricky ones...hang on...
	/*
	* take a path to a directory that does not yet exist
	* lets say you wanna add directory F2 to /F/F1
	* 
	* F2 will be added to the end of the list of descriptors of the directory
	* youre currently inside
	* 
	* F2 will be a namespace directory youre adding to F1
	* 
	* to do this:
	* youre going to go into your n-ary tree, add F2 to the F1 node, then you add F2 to
	* your descriptor map
	* 
	* then you go to your descriptor list, and you need to now move everything forward.
	* 
	* so what you need to do is you need to add your F2 descriptor to the very end
	* of F1
	* 
	* so right before F1_END, youre going to make two new descriptors:
	* F2_START
	* F2_END
	* 
	* now, to be able to write to the file, you need to create space in the file.
	* 
	* so if you just write F2_START and F2_END to the end of F1_END right off the bat,
	* youre going to fuck up the WAD by overwriting another descriptor.
	* 
	* instead, you need to copy everything from F2_END to the end of the file, and move it
	* 32 bytes forward.
	* 
	*/
	void createDirectory(const std::string& path);
	 
	/*
	* using the path passed in,
	* add the file to the directorys node if it exists
	* if it doesnt, just dont do anything.
	* DONT MAKE A NEW DIRECTORY, just dont do anything.
	* 
	* add the file descriptor to the end of the parent nodes descriptor,and the map, and shift
	* everything from the parents descriptor end up to the end of the file forward by 
	* 16 bytes.
	* 
	* set the offset to 0, set the length to 0, its a blank file.
	* 
	*/
	void createFile(const std::string& path);
	 
	/*
	* if given a path to an empty file, 
	* call iscontent, if false, return -1.
	* 
	* if true, make sure its empty by checking its length and offset.
	* if its not empty, just return -1.
	* 
	* if empty, write to it.
	* 
	* take the buffer, write the contents of the buffer to the wad file.
	* set its offset as the offset passed in (if it isnt, set it to 0),
	* set its length to the length passed in.
	* 
	* but how do you add lump data?
	* youre going to add the lump data at the end of the existing lump data
	* 
	* 
	* 
	* NEVER EVER EVER EDIT FILE THAT ARENT EMPTY!!!!
	*/
	int writeToFile(const std::string& path, const char* buffer, int length, int offset = 0);

};