#pragma once

#ifndef _SYNCSOCKETSERVER_H_
#define _SYNCSOCKETSERVER_H_

#include <iostream>
#include <boost/asio.hpp> 
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include "data_structure.h"

using namespace std;

class syncSocketServer
{

private:
	static const int number_of_joints = 8;
	union Ubones
	{
		float fBones[number_of_joints * 3]; // 4 joints
		char strBones[number_of_joints * 12];
	}bones;

	boost::asio::io_context* io_context;// oggetto di interfaccia al servizzio di comunicazione del sistema operativo
	boost::asio::ip::tcp::acceptor* acceptor;  //(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v6(), 9999));
	boost::asio::ip::tcp::socket* socket; //(io_context); // oggetto tcp socket (riceve il canale di comunicazione del sistema operativo)

	float x = 0.0f;

public:

	syncSocketServer(int port, string ipV);

	void waitConnections();

	void closeConnection();

	string reciveString();

	void sendString(string msg);

	void reciveValues();

	void sendValues();

	void initBones(float value);

	void printBones();

	void testComunication();

	void sendSkeleton();

	void  refreshUbones(kinect_data *data);
};



#endif //#ifndef _SYNCSOCKETSERVER_H_