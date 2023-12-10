



//Wad.cpp
/*
* Author: Steven Miller
* Date created: November 19, 2023
* Purpose: Functions for wad file operations
* Used by: Wad.cpp
* Uses: Wad.h
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cctype>
#include <string>
#include <stack>
#include <map>
#include "Wad.h"

//the actual constructor
//used courses.cs.vt.edu/~cs2604/fall02/binio.html for help
Wad::Wad(const std::string& path)
{
	//declare ifstream object
	this->wad_fstream = std::fstream(path,std::ios_base::binary | std::ios_base::in |std::ios_base::out);
	//this->wad_fstream.open(path, std::ios_base::binary | std::ios_base::in | std::ios_base::out);
	this->wad_path = path;
	//get magic
	char wad_magic_char[5];
	for (int i = 0; i < 4; i++)
	{
		wad_fstream.read(&wad_magic_char[i], sizeof(uint8_t));
	}
	wad_magic_char[4] = '\0';
	this->wad_magic = wad_magic_char;

	//get number of descriptors
	wad_fstream.read((char*)&wad_numberofdescriptors, sizeof(uint32_t));

	//get descriptor offset
	wad_fstream.read((char*)&wad_descriptoroffset, sizeof(uint32_t));



	//seek forward
	wad_fstream.seekg(wad_descriptoroffset);


	//get descriptors and build the tree
	std::stack<std::string> file_stack;
	this->file_tree = new nary_tree;

	//create root for tree
	std::string parent_directory = "/";
	file_stack.push(parent_directory);
	wad_struct* root_start = new wad_struct;
	root_start->directory = parent_directory;
	root_start ->isnamespacemarker = true;
	root_start->isstart = true;
	root_start->fullname = "/_START";
	this->file_tree->insert(root_start);
	this->file_map.emplace(parent_directory, root_start);
	uint32_t descriptor_offset_var = this->wad_descriptoroffset;

	for(int k =0; k < wad_numberofdescriptors; k++)
	{
		//get descriptors
		wad_struct* wad_file = new wad_struct;
		//read element offset
		wad_fstream.read((char*)&wad_file->element_offset, sizeof(uint32_t));
		//read element length
		wad_fstream.read((char*)&wad_file->element_length, sizeof(uint32_t));
		//read descriptor name
		char file_name[9];
		for (int i = 0; i < 8; i++)
		{
			wad_fstream.read(&file_name[i], sizeof(uint8_t));
		}
		this->wad_fstream.flush();
		file_name[8] = '\0';
		std::string name = file_name;
		
		//build the tree
		std::string marker = name.substr(2,(name.size()-1));
		std::string markershort = name.substr(1, (name.size() - 1));

		//check if its a namespace marker start
		if(marker == "_START" || markershort == "_START")
		{
			wad_file->name = name.substr(0,name.find("_"));
			wad_file->fullname = name;
			wad_file->iscontentfile = false;
			wad_file->ismapmarker = false;
			wad_file->isnamespacemarker = true;
			wad_file->isstart = true;
			wad_file->parent_directory = parent_directory;
			file_stack.push(wad_file->name+"/");
			parent_directory = create_stringfromstack(file_stack);
			wad_file->directory = parent_directory;
			wad_file->descriptor_location = descriptor_offset_var;
			this->file_map.emplace(parent_directory, wad_file);
			file_tree->insert(wad_file);
			descriptor_offset_var = descriptor_offset_var + 16;
		}
		//check if its a namespace marker end
		else if(marker == "_END" || markershort == "_END")
		{
			wad_file->name = name.substr(0, name.find("_"));
			wad_file->fullname = name;
			wad_file->iscontentfile = false;
			wad_file->ismapmarker = false;
			wad_file->isnamespacemarker = true;
			wad_file->isend = true;
			wad_file->directory = parent_directory;
			wad_file->descriptor_location = descriptor_offset_var;
			if (file_stack.size() != 0)
			{
				file_stack.pop();
				parent_directory = create_stringfromstack(file_stack);
			}
			wad_file->parent_directory = parent_directory;
			file_tree->insert(wad_file);
			descriptor_offset_var = descriptor_offset_var + 16;
		}
		//check if its a map marker
		else if (file_name[0] == 'E' && file_name[2] == 'M' && std::isdigit(file_name[1]) && std::isdigit(file_name[3]))
		//else if (std::regex_match(name, std::regex("^E\dM\d$")))
		{
			wad_file->name = name;
			wad_file->fullname = name;
			wad_file->iscontentfile = false;
			wad_file->ismapmarker = true;
			wad_file->isnamespacemarker = false;
			wad_file->parent_directory = parent_directory;
			file_stack.push(wad_file->name+"/");
			parent_directory = create_stringfromstack(file_stack);
			wad_file->directory = parent_directory;
			wad_file->descriptor_location = descriptor_offset_var;
			this->file_map.emplace(parent_directory, wad_file);
			file_tree->insert(wad_file);
			descriptor_offset_var = descriptor_offset_var + 16;

			//get the next ten files
			for (int i =1; i <= 10; i++)
			{
				wad_struct* mapmarker_child = new wad_struct;
				//read element offset
				wad_fstream.read((char*)&mapmarker_child->element_offset, sizeof(uint32_t));
				//read element length
				wad_fstream.read((char*)&mapmarker_child->element_length, sizeof(uint32_t));
				//read descriptor name
				char child_file_name[9];
				for (int j = 0; j < 8; j++)
				{
					wad_fstream.read(&child_file_name[j], sizeof(uint8_t));
				}
				this->wad_fstream.flush();
				child_file_name[8] = '\0';
				std::string child_name = child_file_name;

				mapmarker_child->name = child_name;
				mapmarker_child->fullname = child_name;
				mapmarker_child->iscontentfile = true;
				mapmarker_child->ismapmarker = false;
				mapmarker_child->isnamespacemarker = false;
				mapmarker_child->parent_directory = parent_directory;
				mapmarker_child->directory = parent_directory + child_name;
				mapmarker_child->descriptor_location = descriptor_offset_var;
				this->file_map.emplace(parent_directory + child_name, mapmarker_child);
				file_tree->insert(mapmarker_child);
				k++;
				descriptor_offset_var = descriptor_offset_var + 16;
			}
			file_stack.pop();
			parent_directory = create_stringfromstack(file_stack);
		}
		//its a content file
		else
		{
			wad_file->name = name;
			wad_file->fullname = name;
			wad_file->iscontentfile = true;
			wad_file->ismapmarker = false;
			wad_file->isnamespacemarker = false;
			wad_file->parent_directory = parent_directory;
			wad_file->directory = parent_directory + file_name;
			wad_file->descriptor_location = descriptor_offset_var;
			this->file_map.emplace(parent_directory + file_name, wad_file);
			file_tree->insert(wad_file);
			descriptor_offset_var = descriptor_offset_var + 16;

		}
	
	}

	this->wad_fstream.flush();
	this->wad_fstream.close();
	this->file_size = descriptor_offset_var;

}
//call private constructor in this function
Wad* Wad::loadWad(const std::string& path)
{
	Wad* wad_file = new Wad(path);

	return wad_file;
}

//returns magic number of file
std::string Wad::getMagic()
{
	return this->wad_magic;
}

//returns if file is content or not
bool Wad::isContent(const std::string& path)
{
	auto it = this->file_map.find(path);
	if (it == this->file_map.end())
	{
		return false;
	}
	else if (it->second->iscontentfile)
	{
		return true;
	}
	return false;
}

bool Wad::isDirectory(const std::string& path)
{
	std::string temppath = path;
	auto it = this->file_map.begin();
	if (temppath != "/" && temppath != "" && temppath.at(temppath.size() - 1) != '/')
	{
		temppath = path + "/";
	}
	it = this->file_map.find(temppath);
	if (it == this->file_map.end())
	{
		return false;
	}
	else if (it->second->isnamespacemarker || it->second->ismapmarker)
	{
		return true;
	}
	return false;
}

int Wad::getSize(const std::string& path)
{
	wad_struct* wad = this->file_tree->get_file_struct_directory(path);
	if (wad == nullptr || wad->ismapmarker || wad->isnamespacemarker)
	{
		return -1;
	}
	return wad->element_length;
}

int Wad::getContents(const std::string& path, char* buffer, int length, int offset)
{
	wad_struct* wad = this->file_tree->get_file_struct_directory(path);
	if (wad == nullptr || (wad->iscontentfile == false))
	{
		return -1;
	}
	this->wad_fstream.open(this->wad_path, std::ios_base::binary | std::ios_base::in);
	this->wad_fstream.seekg(offset + wad->element_offset);
	uint32_t lengthtoread = 0;
	//compare requested length with file structs length

	//if requested length is larger or if offset is incompatible with length
	if ((length > wad->element_length))
	{
		lengthtoread = wad->element_length - offset;
	}
	else if (offset > wad->element_length)
	{
		lengthtoread = 0;
	}
	else
	{
		lengthtoread = length;
	}
	this->wad_fstream.read(buffer, lengthtoread * sizeof(char));
	this->wad_fstream.close();
	return lengthtoread;
}

int Wad::getDirectory(const std::string& path, std::vector<std::string>* directory)
{
	std::string temppath = path;
	auto it = this->file_map.begin();
	if (temppath != "/" && temppath != "" && temppath.at(temppath.size()-1) != '/')
	{
		temppath = path + "/";
	}
	it = this->file_map.find(temppath);
	if (it == this->file_map.end())
	{
		return -1;
	}
	std::vector<std::string> kids = this->file_tree->get_node_kids_names(temppath);
	for (int i = 0; i < kids.size(); i++)
	{
		directory->push_back(kids.at(i));
	}
	return directory->size();
}

void Wad::createDirectory(const std::string& path)
{

	this->wad_fstream.open(this->wad_path, std::ios_base::binary|std::ios_base::out |std::ios_base::in);
	//get directory name
	std::stack<std::string>* stack = this->create_stackfromstring(path);
	std::string directory_name = stack->top();
	stack->pop();
	directory_name.erase(directory_name.end()-1);
	//check if the directory to be created is less than or equal to 2 characters, and its parent directory exists, and its not a map directory

	std::string parent_directory = create_stringfromstack(*stack);

	//check if parent directory exists
	auto it = this->file_map.find(parent_directory);
	if (it == this->file_map.end())
	{
		//do nothing
	}
	else
	{
		//check if directory name is less than 2
		if (directory_name.size() >= 3) //all directory names will have a size of 3 because of the "/" character at the end
		{
			//do nothing
		}
		else
		{
			//check if namespace
			if (!(it -> second->isnamespacemarker))
			{
				//do nothing
			}
			else //were good to go
			{

				//create two new file structs
				wad_struct* wad_file_start = new wad_struct;
				wad_file_start->name = directory_name;
				wad_file_start->fullname = directory_name + "_START";
				wad_file_start->iscontentfile = false;
				wad_file_start->ismapmarker = false;
				wad_file_start->isnamespacemarker = true;
				wad_file_start->isstart = true;
				wad_file_start->parent_directory = parent_directory;
				wad_file_start->element_offset = 0;
				wad_file_start->element_length = 0;
				wad_file_start->directory = parent_directory + directory_name+ "/";

				wad_struct* wad_file_end = new wad_struct;
				wad_file_end->name = directory_name;
				wad_file_end->fullname = directory_name + "_END";
				wad_file_end->iscontentfile = false;
				wad_file_end->ismapmarker = false;
				wad_file_end->isnamespacemarker = true;
				wad_file_end->isend = true;
				wad_file_end->parent_directory = parent_directory;
				wad_file_end->element_offset = 0;
				wad_file_end->element_length = 0;
				wad_file_end->directory = parent_directory + directory_name + "/";

				//add our new descriptors
				this->file_tree->insert_atback(wad_file_start);
				this->file_tree->insert_atback(wad_file_end);

				this->file_map.emplace(parent_directory + directory_name + "/", wad_file_start);

				//get a vector of all of our descriptors in a depth traversal order
				//i.e.: the order that the vector appears is the same order the wad file descriptors are

				//get the entire descriptor list for the tree
				std::vector<file_struct*> vec = this->file_tree->get_descriptor_depthtraverse(this->file_tree->get_root_file()->fullname);

				//set our seek pointer to the beginning of our descriptor list
				uint32_t seek_original = this->wad_descriptoroffset;
				
				//just write the whole list
				for (int i = 1; i < vec.size(); i++)
				{
					//update descriptor locations just in case
					vec.at(i)->descriptor_location = seek_original;
					write_wadfile_descriptor(vec.at(i), seek_original);
					seek_original = seek_original + 16;
				}
				
				//clean up after ourselves
				delete stack;
				
				// update our number of descriptors
				this->wad_numberofdescriptors = this->wad_numberofdescriptors + 2;
				this->wad_fstream.seekp(4, std::ios_base::beg);
				this->wad_fstream.write((char*)&this->wad_numberofdescriptors, sizeof(uint32_t));
				this->wad_fstream.flush();
			}
			
		}
	}
	this->wad_fstream.close();
}

void Wad::createFile(const std::string& path)
{
	this->wad_fstream.open(this->wad_path, std::ios_base::binary | std::ios_base::out | std::ios_base::in);
	//get file name
	std::stack<std::string>* stack = this->create_stackfromstring(path);
	std::string file_name = stack->top();
	stack->pop();
	file_name.erase(file_name.end() - 1);
	//check if the directory to be created is less than or equal to 2 characters, and its parent directory exists, and its not a map directory

	std::string parent_directory = create_stringfromstack(*stack);

	//check if parent directory exists
	auto it = this->file_map.find(parent_directory);

	if (it == this->file_map.end())
	{
		//do nothing
	}
	else
	{
		//check if namespace
		if (!(it->second->isnamespacemarker) || (file_name[0] == 'E' && file_name[2] == 'M' && std::isdigit(file_name[1]) && std::isdigit(file_name[3])))
		{
			//do nothing
		}
		else
		{
			//check if name too long
			if (file_name.size() > 8)
			{
				//do nothing
			}
			else //were good to go
			{
				//add the file descriptor to the end of the parent nodes descriptor, and the map, and shift
				//* everything from the parents descriptor end up to the end of the file forward by
				//* 16 bytes.
				//set the offset to 0, set the length to 0, its a blank file.
				wad_struct* content_file = new wad_struct;
				content_file->name = file_name;
				content_file->fullname = file_name;
				content_file->iscontentfile = true;
				content_file->ismapmarker = false;
				content_file->isnamespacemarker = false;
				content_file->parent_directory = parent_directory;
				content_file->element_offset = 0;
				content_file->element_length = 0;
				content_file->directory = path;

				//insert into tree
				this->file_tree->insert_atback(content_file);

				//insert into map
				this->file_map.emplace(path, content_file);

				//get the entire descriptor list for the tree
				std::vector<file_struct*> vec = this->file_tree->get_descriptor_depthtraverse(this->file_tree->get_root_file()->fullname);

				//set our seek pointer to the beginning of our descriptor list
				uint32_t seek_original = this->wad_descriptoroffset;

				//just write the whole list
				for (int i = 1; i < vec.size(); i++)
				{
					//update descriptor locations just in case
					vec.at(i)->descriptor_location = seek_original;
					write_wadfile_descriptor(vec.at(i), seek_original);
					seek_original = seek_original + 16;
				}


				//clean up after ourselves
				delete stack;

				// update our number of descriptors
				this->wad_numberofdescriptors = this->wad_numberofdescriptors + 1;
				this->wad_fstream.seekp(4, std::ios_base::beg);
				this->wad_fstream.write((char*)&this->wad_numberofdescriptors, sizeof(uint32_t));
				this->wad_fstream.flush();
			}
		}
		

	}

	this->wad_fstream.close();
}

int Wad::writeToFile(const std::string& path, const char* buffer, int length, int offset)
{

	this->wad_fstream.open(this->wad_path, std::ios_base::binary | std::ios_base::out | std::ios_base::in);
	//get directory name
	std::stack<std::string>* stack = this->create_stackfromstring(path);
	std::string file_name = stack->top();
	stack->pop();
	file_name.erase(file_name.end() - 1);
	//get the file were editing
	auto it = this->file_map.find(path);

	if (it == this->file_map.end())
	{
		this->wad_fstream.close();
		return -1;
	}
	else
	{
		wad_struct* file = it->second;
		//a little safety check
		if (file == nullptr)
		{
			this->wad_fstream.close();
			return -1;
		}
		//check if content file
		if (file->iscontentfile == false)
		{
			this->wad_fstream.close();
			return -1;
		}
		else
		{
			//check if its empty
			if (file->element_length != 0)
			{
				this->wad_fstream.close();
				return 0;
			}
			else
			{
				//set files offset
				file->element_offset = this->wad_descriptoroffset;
				//adjust descriptor offset
				this->wad_descriptoroffset = this->wad_descriptoroffset + length;
				this->wad_fstream.seekp(8, std::ios_base::beg);
				this->wad_fstream.write((char*)&this->wad_descriptoroffset, sizeof(uint32_t));
				this->wad_fstream.flush();
				//set its length to the length passed in.
				file->element_length = length;
				//now actually write the data
				uint32_t data_beginning = this->wad_descriptoroffset - length;
				uint32_t seek = 0;
				this->wad_fstream.seekp(data_beginning, std::ios_base::beg);
				this->wad_fstream.write(buffer, sizeof(char) * length);
				this->wad_fstream.flush();
				//since weve overwritten the first descriptor, rewrite them all.
				std::vector<file_struct*> vec = this->file_tree->get_descriptor_depthtraverse(this->file_tree->get_root_file()->fullname);
				//point to beginning of descriptor list
				seek = this->wad_descriptoroffset;
				for (int i = 1; i < vec.size(); i++)
				{
					//update descriptor locations just in case
					vec.at(i)->descriptor_location = seek;
					write_wadfile_descriptor(vec.at(i), seek);
					seek = seek + 16;
				}

				this->wad_fstream.close();

				return length;
			}
		}
	}

	this->wad_fstream.close();

	return length;
}

std::string Wad::create_stringfromstack(std::stack<std::string> stack)
{
	//copy stack using its copy constructor
	std::stack<std::string> cpy(stack);
	std::string temp;
	for (int i = 0; i < stack.size(); i++)
	{
		temp.insert(0, cpy.top());
		cpy.pop();
	}
	return temp;
}

std::stack<std::string >* Wad::create_stackfromstring(std::string string)
{
	std::stack<std::string>* stack = new std::stack<std::string>;
	char* str = new char[string.size()+1];
	char* context = nullptr;
	std::memcpy(str,string.c_str(),string.size()+1);
	int x = 0;
	char* token = strtok(str, "/");
	stack->push("/");
	while (token != nullptr)
	{
		std::string temp = token;
		stack->push(temp + "/");
		token = strtok(nullptr, "/");
	}
	delete[] str;
	return stack;
}

void Wad::write_wadfile_descriptor(wad_struct* wad_file, uint32_t seekpointer)
{
	//write element offset
	this->wad_fstream.seekp(seekpointer, std::ios_base::beg);

	this->wad_fstream.write((char*)&wad_file->element_offset, sizeof(uint32_t));

	//write element length
	this->wad_fstream.write((char*)&wad_file->element_length, sizeof(uint32_t));

	char fullname_char[9] = { NULL };

	for (int j = 0; j < wad_file->fullname.size(); j++)
	{
		fullname_char[j] = wad_file->fullname[j];
	}

	fullname_char[8] = '\0';

	for (int k = 0; k < 8; k++)
	{
		wad_fstream.write((char*)&fullname_char[k], sizeof(char));
	}

	//actually write our changes to the file
	this->wad_fstream.flush();
}

