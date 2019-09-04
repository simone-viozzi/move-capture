#pragma once

#ifndef _ARD_FILE_MANAGER_
#define _ARD_FILE_MANAGER_

#include <iostream>
#include <ostream>
#include <fstream>
#include <string>
#include "data_structure.h"


class ard_file_manager
{
private:
	std::fstream f;
	std::ios::_Openmode mode;
	unsigned long int line_id = 0;

public:
	ard_file_manager(std::string file_name, std::ios::_Openmode mode);

	void write_data_line(arduino_data dat);

	bool read_data_line(arduino_data* dat);

	~ard_file_manager()
	{
		f.close();
	}

	void close()
	{
		f.close();
	}
};



#endif //#ifndef _ARD_FILE_MANAGER_
