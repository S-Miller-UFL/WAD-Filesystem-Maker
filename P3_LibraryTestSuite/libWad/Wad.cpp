



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
	this->wad_path = path;
	//get magic
	for (int i = 0; i < 4; i++)
	{
		wad_fstream.read(&wad_magic[i], sizeof(uint8_t));
	}

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
	wad_struct* root = new wad_struct;
	root->directory = parent_directory;
	root->isnamespacemarker = true;
	this->file_tree->insert(root);
	for(int k =0; k < wad_numberofdescriptors; k++)
	{
		//get descriptors
		wad_struct* wad_file = new wad_struct;
		//read offset
		wad_fstream.read((char*)&wad_file->offset, sizeof(uint32_t));
		//read length
		wad_fstream.read((char*)&wad_file->length, sizeof(uint32_t));
		//read name
		char file_name[8];
		for (int i = 0; i < 8; i++)
		{
			wad_fstream.read(&file_name[i], sizeof(uint8_t));
		}

		std::string name = file_name;
		
		//build the tree
		
		//check if its a namespace marker start
		if (name.find("START") != std::string::npos)
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
			this->file_map.emplace(parent_directory, wad_file);
			file_tree->insert(wad_file);
			int x= 0;
		}
		//check if its a namespace marker end
		else if (name.find("END") != std::string::npos)
		{
			wad_file->name = name.substr(0, name.find("_"));
			wad_file->fullname = name;
			wad_file->iscontentfile = false;
			wad_file->ismapmarker = false;
			wad_file->isnamespacemarker = true;
			wad_file->isend = true;
			wad_file->directory = parent_directory;
			file_stack.pop();
			parent_directory = create_stringfromstack(file_stack);
			wad_file->parent_directory = parent_directory;
			file_tree->insert(wad_file);

		}
		//check if its a map marker
		else if (file_name[0] == 'E' and file_name[2] == 'M' and std::isdigit(file_name[1]) and std::isdigit(file_name[3]))
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
			this->file_map.emplace(parent_directory, wad_file);
			file_tree->insert(wad_file);

			//get the next ten files
			for (int i =0; i < 10; i++)
			{
				wad_struct* mapmarker_child = new wad_struct;
				wad_fstream.read((char*)&mapmarker_child->offset, sizeof(uint32_t));
				//read length
				wad_fstream.read((char*)&mapmarker_child->length, sizeof(uint32_t));
				//read name
				char child_file_name[8];
				for (int j = 0; j < 8; j++)
				{
					wad_fstream.read(&child_file_name[j], sizeof(uint8_t));
				}

				std::string child_name = child_file_name;

				mapmarker_child->name = child_name;
				mapmarker_child->fullname = child_name;
				mapmarker_child->iscontentfile = true;
				mapmarker_child->ismapmarker = false;
				mapmarker_child->isnamespacemarker = false;
				mapmarker_child->parent_directory = parent_directory;
				mapmarker_child->directory = parent_directory + child_name;
				this->file_map.emplace(parent_directory + child_name, mapmarker_child);
				file_tree->insert(mapmarker_child);
				k++;
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
			this->file_map.emplace(parent_directory + file_name, wad_file);
			file_tree->insert(wad_file);

		}
	
	}

	this->wad_fstream.close();
}
//call private constructor in this function
Wad* Wad::loadWad(const std::string& path)
{
	Wad* wad_file = new Wad(path);

	return wad_file;
}

std::string Wad::getMagic()
{
	return this->wad_magic;
}

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
	auto it = this->file_map.find(path + "/");
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
	wad_struct* wad = this->file_tree->get_file_struct(path);
	return wad->length;
}

int Wad::getContents(const std::string& path, char* buffer, int length, int offset)
{
	wad_struct* wad = this->file_tree->get_file_struct(path);
	buffer = wad->content;
	//temporary
	return wad->length;
}

int Wad::getDirectory(const std::string& path, std::vector<std::string>* directory)
{
	std::vector<std::string> kids = this->file_tree->get_node_kids_names(path);
	for (int i = 0; i < kids.size(); i++)
	{
		directory->push_back(kids.at(i));
	}
	return directory->size();
}

void Wad::createDirectory(const std::string& path)
{
	
	//get directory name
	std::stack<std::string>* stack = this->create_stackfromstring(path);
	std::string directory_name = stack->top();
	stack->pop();
	if (this->file_tree->get_file_struct(path) == nullptr)
	{


		//create two new file structs
		wad_struct* wad_file = new wad_struct;
		wad_file->name = directory_name;
		wad_file->fullname = directory_name + "_START";
		wad_file->iscontentfile = false;
		wad_file->ismapmarker = false;
		wad_file->isnamespacemarker = true;
		std::stack<std::string>* stack_two = new std::stack<std::string>;


		//create parent directory
		for (int i = 0; i <= stack->size(); i++)
		{
			stack_two->push(stack->top());
			stack->pop();
		}
		std::string foo = "/";
		for (int i = 0; i <= stack_two->size(); i++)
		{
			foo = foo + stack_two->top();
			stack_two->pop();
			foo = foo + "/";
		}
		wad_file->parent_directory = foo;
		wad_file->directory = path;
		this->file_map.emplace(wad_file->parent_directory, wad_file);
		file_tree->insert(wad_file);
		stack_two->empty();

		//this is taking so long dude jesus christ
		stack = this->create_stackfromstring(path);
		directory_name = stack->top();
		stack->pop();
		wad_struct* wad_file_two = new wad_struct;
		wad_file_two->name = directory_name;
		wad_file_two->fullname = directory_name + "_END";
		wad_file_two->iscontentfile = false;
		wad_file_two->ismapmarker = false;
		wad_file_two->isnamespacemarker = true;

		for (int i = 0; i <= stack->size(); i++)
		{
			stack_two->push(stack->top());
			stack->pop();
		}
		foo = "/";

		for (int i = 0; i <= stack_two->size(); i++)
		{
			foo = foo + stack_two->top();
			stack_two->pop();
			foo = foo + "/";
		}

		wad_file_two->parent_directory = foo;
		wad_file_two->directory = path;
		this->file_map.emplace(wad_file_two->parent_directory, wad_file_two);
		file_tree->insert(wad_file_two);
		delete stack_two;

		//get descriptor list from tree
		std::vector<file_struct*> vec = this->file_tree->get_descriptor_list();

		//change number of descriptors
		this->wad_fstream.open(this->wad_path, std::ios::in | std::ios::out | std::ios::binary);
		this->wad_fstream.seekp(4, std::ios_base::beg);
		this->wad_numberofdescriptors = this->wad_numberofdescriptors + 2;
		this->wad_fstream.write((char*)&this->wad_numberofdescriptors, sizeof(uint32_t));

		//write to file

		this->wad_fstream.seekp(this->wad_descriptoroffset, std::ios_base::beg);

		//for every descriptor:
		for (int i = 1; i < vec.size(); i++)
		{

			//write offset
			this->wad_fstream.write((char*)&vec.at(i)->offset, sizeof(uint32_t));

			//write length
			this->wad_fstream.write((char*)&vec.at(i)->length, sizeof(uint32_t));
			char foo[8] = { NULL};
			//write name
			for (int j = 0; j < vec.at(i)->fullname.size(); j++)
			{
				foo[j] = vec.at(i)->fullname[j];
			}
			for (int k = 0; k < 8; k++)
			{
				this->wad_fstream.write(&foo[k], sizeof(uint8_t));
			}


		}

		this->wad_fstream.close();

		delete stack;
	}
}

void Wad::createFile(const std::string& path)
{
}

int Wad::writeToFile(const std::string& path, const char* buffer, int length, int offset)
{
	return 0;
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
	while (token != nullptr)
	{
		stack->push(token);
		token = strtok(nullptr, "/");
	}
	delete[] str;
	return stack;
}

std::vector<uint8_t> Wad::read_whole_file(std::string path)
{
	std::vector<uint8_t> byte_vector;
	wad_fstream.open(this->wad_path);
	while (!wad_fstream.eof())
	{
		uint8_t byte = 0;
		wad_fstream.read((char*)&byte, sizeof(uint8_t));
		byte_vector.push_back(byte);

	}
	return byte_vector;
}

void Wad::print_map()
{
	for (auto it = this->file_map.begin(); it != this->file_map.end(); it++)
	{
		std::cout << it->first << " " << it->second->name << std::endl;
	}
}
