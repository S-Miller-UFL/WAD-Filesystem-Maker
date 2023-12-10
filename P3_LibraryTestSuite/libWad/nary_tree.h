





//nary_tree.h
/*
* Author: Steven Miller
* Date created: November 16, 2023
* Purpose: Functions for our n_ary tree data structure
* Used by: wad.cpp
* Uses: nary_tree.cpp
*/

//need to figure out what search functions i want
//will probably find most use out of "search_by_file()"
#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <cctype>
#include <string>
#include <stack>
#include <map>
#include "filestruct.h"

typedef wad_struct file_struct;

class nary_tree
{
private:
	struct Node
	{
		std::vector<Node*> children;
		//pointer to actual file struct
		file_struct* file_struct_pntr;
	};

	//functions that help with finding nodes
	Node* search_by_file(file_struct*, nary_tree::Node*);
	//functions that help with finding nodes
	Node* search_by_directory(std::string, nary_tree::Node*);
	Node* create_node(file_struct* file_struct_pntr)
	{
		Node* n = new Node;
		n->file_struct_pntr = file_struct_pntr;
		return n;
	}
	//kill yourself and your children
	Node* kill(Node*);
	//variables
	uint32_t node_count;
	Node* tree_root;

	std::vector<file_struct*> get_node_descriptor_list(std::vector<file_struct*>*, Node*);
	//do not include in final submission
	Node* print_node_level_traverse(Node*);
	Node* print_node_depth_traverse(Node*);
public:

	//constructor
	nary_tree();
	//functions that help with inserting nodes
	void insert(file_struct*);
	//functions that help with removing nodes
	void remove(file_struct*);
	//destructor
	~nary_tree();
	
	//getters
	file_struct* get_root_file();
	uint32_t getnodecount();
	//mutators
	void change_node_file(file_struct*, file_struct*);

	//get file struct pointer
	file_struct* get_file_struct(std::string);

	//get number of kids
	uint32_t get_number_of_children(std::string);

	std::vector<file_struct*> get_descriptor_list();

	//get kids
	std::vector<std::string> get_node_kids_names(std::string);
	//for debugging, do not include in submission
	void print_tree_level_traverse();
	void print_tree_depth_traverse();

};

