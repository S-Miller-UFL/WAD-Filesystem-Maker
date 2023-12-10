




//filestruct.h
/*
* Author: Steven Miller
* Date created: November 16, 2023
* Purpose: File struct for files and directories
* Used by: Wad.h
* Uses: N/A
*/

#pragma once

typedef struct wad_file_struct
{
	bool ismapmarker = false;
	bool isnamespacemarker = false;
	bool iscontentfile = false;
	bool isstart = false;
	bool isend = false;

	std::string parent_directory = "";
	std::string directory = "";
	//names can be a max of 8 bytes
	std::string name = "";
	std::string fullname = "";
	uint32_t length = 0;
	uint32_t offset = 0;
	char* content;

}wad_struct;
