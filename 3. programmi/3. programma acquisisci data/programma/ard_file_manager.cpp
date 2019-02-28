
#include "pch.h"
#include "ard_file_manager.h"


ard_file_manager::ard_file_manager(std::string file_name, std::ios::_Openmode mode)
{
	this->mode = mode;
	f.open(file_name, mode | std::ios::binary);
}



void ard_file_manager::write_data_line(arduino_data dat)
{
	if (mode != std::ios::out) { std::cout << "################# errore file" << std::endl; return; }

	f << line_id << std::endl;

	f << dat.frame_time << std::endl;

	for (int i = 0; i < 3; i++)
	{
		f << dat.acc_xyz[i] << std::endl;
	}

	for (int i = 0; i < 3; i++)
	{
		f << dat.gy_xyz[i] << std::endl;
	}

	for (int i = 0; i < 3; i++)
	{
		f << dat.magn_xyz[i] << std::endl;
	}

	f << dat.temp << std::endl;

	line_id++;
	f.flush();
}



bool ard_file_manager::read_data_line(arduino_data* dat)
{
	if (mode != std::ios::in) { std::cout << "################# errore file" << std::endl;  return 0; }
	if (!f) { std::cout << "end of file" << std::endl; return 0; }

	f >> line_id >> dat->frame_time;

	for (int i = 0; i < 3; i++)
	{
		f >> dat->acc_xyz[i];
	}

	for (int i = 0; i < 3; i++)
	{
		f >> dat->gy_xyz[i];
	}

	for (int i = 0; i < 3; i++)
	{
		f >> dat->magn_xyz[i];
	}

	f >> dat->temp;

	return 1;
}