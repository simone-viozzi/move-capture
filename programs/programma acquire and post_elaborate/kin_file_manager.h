#pragma once


#ifndef _KIN_FILE_MANAGER_H_
#define _KIN_FILE_MANAGER_H_

#include <iostream>
#include <ostream>
#include <fstream>
#include <string>

#include "data_structure.h"

class kin_file_manager
{
private:
	std::fstream f;
	std::ios::_Openmode mode;
	unsigned long int line_id = 0;

public:
	kin_file_manager(std::string file_name, std::ios::_Openmode mode);

	void write_data_line(kinect_data dat);

	bool read_data_line(kinect_data* dat);

	~kin_file_manager()
	{
		f.close();
	}

	void close()
	{
		f.close();
	}

};





#endif // #ifndef _KIN_FILE_MANAGER_H_