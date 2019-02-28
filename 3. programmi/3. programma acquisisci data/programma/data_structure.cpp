#include "pch.h"

#include "data_structure.h"

#define M_PI 3.14159265358979323846  /* pi */


bool kinect_data::full_frame()
{
	for (int i = 0; i < number_of_joints; i++)
	{
		if (needed_joints[i].joint_name == -1)
		{
			return false;
		}
	}
	return true;
}

void kinect_data::print_data()
{
	//if (full_frame())
	//{
	std::cout << "kinect data: " << std::endl;
	for (int i = 0; i < number_of_joints; i++)
	{
		std::cout << "\t "
			<< needed_joints[i].joint_name << ": " << joint_Enum_ToStr(needed_joint[i], "ita")
			<< " x->" << needed_joints[i].position[0]
			<< " y->" << needed_joints[i].position[1]
			<< " z->" << needed_joints[i].position[2]
			<< std::endl;
	}
	std::cout << "\t time: " << frame_time << std::endl << std::endl;
	//}
}

float kinect_data::jointAngleX(float *P1, float *P2)
{
	float alfa = (180 / M_PI)*atan((((P2[1] - P1[1]))) / ((P2[2] - P1[2])));
	if (((P2[2] - P1[2])) > 0)
	{
		return alfa; //risultato in gradi
	}
	else
	{
		return alfa + 180.0f; //risultato in gradi
	}
}

float kinect_data::jointAngleY(float *P1, float *P2)
{
	float beta = (180 / M_PI)*atan(((P2[0] - P1[0])) / (-(P2[2] - P1[2])));
	if ((-(P2[2] - P1[2])) > 0)
	{
		return beta; //risultato in gradi
	}
	else
	{
		return beta /*+ 180.0f*/; //risultato in gradi
	}
}

float kinect_data::jointAngleZ(float *P1, float *P2)
{
	float alfa = (180 / M_PI)*atan((((P2[1] - P1[1]))) / (-(P2[2] - P1[2])));
	if ((-(P2[2] - P1[2])) > 0)
	{
		return alfa; //risultato in gradi
	}
	else
	{
		return alfa + 180.0f; //risultato in gradi
	}
}

void kinect_data::updateAngles()
{
	for (int i = 0; i < number_of_joints; i++)
	{
		if (needed_joints[i].to_joint != -1)
		{ //calcolo gli angoloi solo per i nodi con un nodo figlio

			if (i == 6)
			{ //schiena
				needed_joints[i].angle[0] = ref_Ang_x[i] + jointAngleX(needed_joints[i].position, needed_joints[to_ref_joint[needed_joints[i].to_joint]].position);
				needed_joints[i].angle[1] = ref_Ang_y[i];
				needed_joints[i].angle[2] = ref_Ang_z[i] + jointAngleY(needed_joints[i].position, needed_joints[to_ref_joint[needed_joints[i].to_joint]].position);
			}
			else
			{ //braccia
				needed_joints[i].angle[0] = 0;
				needed_joints[i].angle[1] = mul_fctor_y[i] * (ref_Ang_y[i] + jointAngleY(needed_joints[i].position, needed_joints[to_ref_joint[needed_joints[i].to_joint]].position));
				needed_joints[i].angle[2] = mul_fctor_z[i] * (ref_Ang_z[i] + jointAngleZ(needed_joints[i].position, needed_joints[to_ref_joint[needed_joints[i].to_joint]].position));
			}
		}
		else
		{
			//cout << "giunto " << needed_joint[i] << " non possiede un giunto successivo" << endl;
		}
	}
}

std::string kinect_data::joint_Enum_ToStr(int n, std::string language)
{
	std::string name_ita("unknown");
	std::string name_eng("unknown");
	switch (n)
	{
	case 0:
		name_eng = "Spine Base";
		name_ita = "Base della colonna vertebrale";
		break;
	case 1:
		name_eng = "Spine Mid";
		name_ita = "centro colonna vertebrtale";
		break;
	case 2:
		name_eng = "Neck";
		name_ita = "collo";
		break;
	case 3:
		name_eng = "Head";
		name_ita = "testa";
		break;
	case 4:
		name_eng = "Shoulder Left";
		name_ita = "Spalla sinistra";
		break;
	case 5:
		name_eng = "Elbow Left";
		name_ita = "Gomito sinistro";
		break;
	case 6:
		name_eng = "Wrist Left";
		name_ita = "Polso sinistro";
		break;
	case 7:
		name_eng = "Hand Left";
		name_ita = "mano sinistra";
		break;
	case 8:
		name_eng = "Shoulder Right";
		name_ita = "Mano destra";
		break;
	case 9:
		name_eng = "Elbow Right";
		name_ita = "gomito destro";
		break;
	case 10:
		name_eng = "Wrist Right";
		name_ita = "Polso destro";
		break;
	case 11:
		name_eng = "Hand Right";
		name_ita = "Mano destra";
		break;
	case 12:
		name_eng = "Hip Left";
		name_ita = "Anca sinistra";
		break;
	case 13:
		name_eng = "Knee Left";
		name_ita = "ginocchio sinistro";
		break;
	case 14:
		name_eng = "Ankle Left";
		name_ita = "caviglia sinistra";
		break;
	case 15:
		name_eng = "Foot Left";
		name_ita = "piede sinistro";
		break;
	case 16:
		name_eng = "Hip Right";
		name_ita = "Anca destra";
		break;
	case 17:
		name_eng = "Knee Right";
		name_ita = "ginocchio destro";
		break;
	case 18:
		name_eng = "Ankle Right";
		name_ita = "caviglia destra";
		break;
	case 19:
		name_eng = "Foot Right";
		name_ita = "Piede destro";
		break;
	case 20:
		name_eng = "Spine Shoulder";
		name_ita = "Spina dorsale";
		break;
	case 21:
		name_eng = "Hand Tip Left";
		name_ita = "Punta della mano sinistra";
		break;
	case 22:
		name_eng = "Thumb Left";
		name_ita = "pollice sinistro";
		break;
	case 23:
		name_eng = "Hand Tip Right";
		name_ita = "Punta mano destra";
		break;
	case 24:
		name_eng = "Thumb Right";
		name_ita = "pollice destro";
		break;
	}
	if (language == "eng")
	{
		return name_eng;
	}
	else if (language == "ita")
	{
		return name_ita;
	}
}




void arduino_data::print_data()
{
	std::cout << "arduino data: " << std::endl
		<< "\t acc_xyz: " << acc_xyz[0] << "\t" << acc_xyz[1] << "\t" << acc_xyz[2] << std::endl
		<< "\t gy_xyz: " << gy_xyz[0] << "\t" << gy_xyz[1] << "\t" << gy_xyz[2] << std::endl
		<< "\t magn_xyz: " << magn_xyz[0] << "\t" << magn_xyz[1] << "\t" << magn_xyz[2] << std::endl
		<< "\t temp: " << temp << std::endl
		<< "\t time: " << frame_time << std::endl
		<< std::endl;
}


template<typename type_in, std::size_t N, typename type_out, std::size_t M>
void dataset_data< type_in, N, type_out, M>::print_data()
{
	std::cout << std::endl << "input" << std::endl;
	for (auto i : in)
	{
		std::cout << i << " ";
	}
	std::cout << std::endl << "output" << std::endl;
	for (auto i : out)
	{
		std::cout << i << " ";
	}
	std::cout << std::endl << std::endl;
}