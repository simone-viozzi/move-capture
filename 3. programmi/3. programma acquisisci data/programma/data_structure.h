#pragma once


#ifndef _DATA_STRUCTURE_H_
#define _DATA_STRUCTURE_H_

#include <iostream>
#include <string>
#include <boost/array.hpp>

struct kinect_data
{
	static const int number_of_joints = 3;

	struct joit_data
	{
		int joint_name = -1;
		int to_joint = -1;
		float position[3] = { 0, 0, 0 };
		float angle[3] = { 0, 0, 0 };
	};

	int needed_joint[number_of_joints] = { 8, 9, 10 };
	int to_joint[24] = { -1, -1, -1, -1, -1, -1, -1, -1, 9, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	int to_ref_joint[24] = { -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }; // tabella di accesso (chiave numero joint kinect) -> (restituisce l-indice del numero del giunto in needed_joint)    
	float ref_Ang_x[number_of_joints] = { 0, 0, 0 }; //per  8, 9
	float ref_Ang_y[number_of_joints] = { 0, 0, 0 }; //per 8, 9
	float ref_Ang_z[number_of_joints] = { 180, 180, 0 }; //per 8, 9
	float mul_fctor_x[number_of_joints] = { 0, 0, 0 }; //per 8, 9
	float mul_fctor_y[number_of_joints] = { 1, 1, 0 }; //per 8, 9
	float mul_fctor_z[number_of_joints] = { 1, 1, 0 }; //per 8, 9

	//int needed_joint[number_of_joints] = { 4, 5, 6, 8, 9, 10, 1, 20 };
	//int to_joint[24] = { -1, 20, -1, -1, 5, 6, -1, -1, 9, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, };
	//int to_ref_joint[24] = { -1, 6, -1, -1, -1, 1, 2, -1, 3, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, 7, -1, -1, -1 }; // tabella di accesso (chiave numero joint kinect)  
	//float ref_Ang_x[number_of_joints] = { 0, 0, 0, 0, 0, 0, -90, 0 }; //per 4, 5, 8, 9
	//float ref_Ang_y[number_of_joints] = { 0, 0, 0, 0, 0, 0, 90, 0 }; //per 4, 5, 8, 9
	//float ref_Ang_z[number_of_joints] = { 180, 180, 0, 180, 180, 0, 270, 0 }; //per 4, 5, 8, 9
	//float mul_fctor_x[number_of_joints] = { 0, 0, 0, 0, 0, 0, 0, 0 }; //per 4, 5, 8, 9
	//float mul_fctor_y[number_of_joints] = { 1, 1, 0, 1, 1, 0, 0, 0 }; //per 4, 5, 8, 9
	//float mul_fctor_z[number_of_joints] = { 1, 1, 0, 1, 1, 0, 0, 0 }; //per 4, 5, 8, 9

	joit_data needed_joints[number_of_joints];


	uint64_t frame_time = 0;

	uint64_t contatore = 0;



	bool full_frame();

	void print_data();

	float jointAngleX(float *P1, float *P2);

	float jointAngleY(float *P1, float *P2);

	float jointAngleZ(float *P1, float *P2);

	void updateAngles();

	std::string joint_Enum_ToStr(int n, std::string language);

};



struct arduino_data
{
	float acc_xyz[3];
	float gy_xyz[3];
	float magn_xyz[3];
	float temp;

	uint64_t frame_time = 0;

	uint64_t contatore = 0;

	void print_data();

};


/// <summary>
/// list of template parameter
/// <list>
/// type_in -> the type of the input
/// type_out -> the type of the output
/// N -> the number of input
/// M -> the number of output
/// </list>
/// </summary>
template<typename type_in, std::size_t N, typename type_out, std::size_t M>
struct dataset_data
{
	boost::array<type_in, N> in;
	boost::array<type_out, M> out;

	void print_data();

};



#endif //#ifndef _DATA_STRUCTURE_H_