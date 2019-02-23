
#include "pch.h"
#include "kin_file_manager.h"



kin_file_manager::kin_file_manager(std::string file_name, std::ios::_Openmode mode)
{
	this->mode = mode;
	f.open(file_name, mode | std::ios::binary);
}


void kin_file_manager::write_data_line(kinect_data dat)
{
	if (mode != std::ios::out) { std::cout << "################# errore file" << std::endl;  return; }

	f << line_id << std::endl;
	f << dat.frame_time << std::endl;

	for (int i = 0; i < dat.number_of_joints; i++)
	{
		f << dat.needed_joints[i].joint_name << std::endl;
		for (int j = 0; j < 3; j++)
		{
			f << dat.needed_joints[i].angle[j] << std::endl;
		}
	}
	line_id++;
	f.flush();
}


bool kin_file_manager::read_data_line(kinect_data* dat)
{
	if (mode != std::ios::in) { std::cout << "################# errore file" << std::endl; return 0; }
	if (!f) { std::cout << "end of file" << std::endl; return 0; }

	f >> line_id >> dat->frame_time;

	for (int i = 0; i < dat->number_of_joints; i++)
	{
		f >> dat->needed_joints[i].joint_name;
		for (int j = 0; j < 3; j++)
		{
			f >> dat->needed_joints[i].angle[j];
		}
	}

	return 1;
}
