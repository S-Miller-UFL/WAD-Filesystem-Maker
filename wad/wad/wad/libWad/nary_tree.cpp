


//nary_tree.cpp
/*
* Author: Steven Miller
* Date created: November 19, 2023
* Purpose: Functions for our n_ary tree data structure
* Used by: nary_tree.h
* Uses: N/A
*/

#include "nary_tree.h"

//constructor
nary_tree::nary_tree()
{
	this->tree_root = nullptr;
	this->node_count = 0;
}

//insert into tree
void nary_tree::insert(file_struct* file_struct_pntr)
{
	if (this->tree_root == nullptr)
	{
		this->tree_root = new Node;
		this->tree_root->file_struct_pntr = file_struct_pntr;
		this->node_count++;
	}
	else
	{

		//get nodes parent
		nary_tree::Node* temp = this->search_by_directory(file_struct_pntr->parent_directory, this->tree_root);
		//if directory doesnt exist
		if (temp == nullptr)
		{
			//node doesnt exist
		}
		else
		{
			//add to node list
			temp->children.push_back(this->create_node(file_struct_pntr));
			this->node_count++;

		}
	}
}

void nary_tree::insert_atback(file_struct* file_struct_pntr)
{
	if (this->tree_root == nullptr)
	{
		this->tree_root = new Node;
		this->tree_root->file_struct_pntr = file_struct_pntr;
		this->node_count++;
	}
	else
	{

		//get nodes parent
		nary_tree::Node* temp = this->search_by_directory(file_struct_pntr->parent_directory, this->tree_root);
		//if directory doesnt exist
		if (temp == nullptr)
		{
			//node doesnt exist
		}
		else
		{
			//add to node list
			temp->children.emplace_back(this->create_node(file_struct_pntr));
			this->node_count++;

		}
	}
}

//remove from tree
void nary_tree::remove(file_struct* file_struct_pntr)
{
	//get node representing directory
	nary_tree::Node* temp = this->search_by_file(file_struct_pntr, this->tree_root);
	if (temp == nullptr)
	{
		//do nothing
	}
	else
	{
		//if, for whatever reason, the node cannot be killed
		if (this->kill(temp) != nullptr)
		{
			throw std::exception();
		}
	}
}

//search the tree based on file pointer
nary_tree::Node* nary_tree::search_by_file(file_struct* file_struct_pntr, nary_tree::Node* target)
{
	if (target->file_struct_pntr == file_struct_pntr)
	{
		return target;
	}
	else
	{
		if (target->children.size() > 0)
		{
			for (int i = 0; i < target->children.size(); i++)
			{
				Node* temp = search_by_file(file_struct_pntr, target->children.at(i));
				if (temp != nullptr)
				{
					return temp;
				}
			}
		}
	}
	return nullptr;
}

//search the tree based on file directory
nary_tree::Node* nary_tree::search_by_directory(std::string directory, nary_tree::Node* target)
{
	if (target->file_struct_pntr->directory == directory)
	{
		return target;
	}
	else
	{
		if (target->children.size() > 0)
		{
			for (int i = 0; i < target->children.size(); i++)
			{
				Node* temp = search_by_directory(directory, target->children.at(i));
				if (temp != nullptr)
				{
					return temp;
				}
			}
		}
	}
	return nullptr;
}



//get pointer to root file
file_struct* nary_tree::get_root_file()
{
	return this->tree_root->file_struct_pntr;
}

//remove from tree
uint32_t nary_tree::getnodecount()
{
	return this->node_count;
}

//kill yourself and your children
nary_tree::Node* nary_tree::kill(nary_tree::Node* target)
{
	if (target == nullptr)
	{
		return target;
	}
	else if (target->children.size() == 0)
	{
		target == nullptr;
		delete target;
	}
	else
	{
		for (int i = 0; i < target->children.size(); i++)
		{
			kill(target->children.at(i));
		}
		target->children.clear();
		target = nullptr;
		delete target;
	}
	this->node_count--;
	return target;
}

nary_tree::~nary_tree()
{
	//shut it down!
	this->kill(this->tree_root);
}


file_struct* nary_tree::get_file_struct_directory(std::string directory)
{
	if (this->search_by_directory(directory, this->tree_root) == nullptr)
	{
		return nullptr;
	}
	return this->search_by_directory(directory, this->tree_root)->file_struct_pntr;
}

file_struct* nary_tree::get_file_struct_byname(std::string name)
{
	if (this->search_by_name(name, this->tree_root) == nullptr)
	{
		return nullptr;
	}
	return this->search_by_name(name, this->tree_root)->file_struct_pntr;
}

nary_tree::Node* nary_tree::search_by_name(std::string name, nary_tree::Node* target)
{
	if (target->file_struct_pntr->fullname == name)
	{
		return target;
	}
	else
	{
		if (target->children.size() > 0)
		{
			for (int i = 0; i < target->children.size(); i++)
			{
				Node* temp = search_by_name(name, target->children.at(i));
				if (temp != nullptr)
				{
					return temp;
				}
			}
		}
	}
	return nullptr;
}

uint32_t nary_tree::get_number_of_children(std::string directory)
{
	return this->search_by_directory(directory, this->tree_root)->children.size();
}


std::vector<file_struct*> nary_tree::get_descriptor_depthtraverse(std::string nodename)
{
	std::queue<file_struct*> q;
	std::vector<file_struct*> vec;
	get_filestruct_depthtraverse(this->tree_root, &q);
	while (q.size() != 0)
	{
		if (q.front()->fullname != nodename)
		{
			q.pop();
		}
		else
		{
			break;
		}
	}
	while (q.size() != 0)
	{
		vec.push_back(q.front());
		q.pop();
	}

	return vec;
}

void nary_tree::get_filestruct_depthtraverse(Node* root, std::queue<file_struct*>* vec)
{
	vec->push(root->file_struct_pntr);
	for (int i = 0; i < root->children.size(); i++)
	{
		get_filestruct_depthtraverse(root->children.at(i),vec);
	}
}

std::vector<file_struct*> nary_tree::get_descriptor_list()
{

	std::vector<file_struct*> vec;

	return get_node_descriptor_list(&vec, this->tree_root);
}

std::vector<std::string> nary_tree::get_node_kids_names(std::string path)
{
	Node* parent = this->search_by_directory(path, this->tree_root);
	std::vector<std::string> kid_directories;
	if (parent == nullptr)
	{
		return kid_directories;
	}
	for (int i = 0; i < parent->children.size(); i++)
	{
		if (!parent->children.at(i)->file_struct_pntr->isend)
		{
			kid_directories.push_back(parent->children.at(i)->file_struct_pntr->name);
		}
	}
	return kid_directories;
}

std::vector<file_struct*> nary_tree::get_node_descriptor_list(std::vector<file_struct*>* vec, Node* root)
{
	vec->push_back(root->file_struct_pntr);
	for (int i = 0; i < root->children.size(); i++)
	{
		get_node_descriptor_list(vec, root->children.at(i));
	}
	return *vec;
}
