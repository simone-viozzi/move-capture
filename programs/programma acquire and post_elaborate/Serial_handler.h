#pragma once


#ifndef _SERIAL_HADLER_H_
#define _SERIAL_HADLER_H_

#include <iostream>
#include <string>
#include <boost/asio.hpp> 
#include <boost/bind.hpp>
#include <boost/asio/serial_port.hpp> 



class Serial
{
private:


	boost::asio::serial_port* port;

	union Scomp_float
	{
		float n_float;
		uint32_t n_int;
		uint8_t n_bytes[4];
	};

	union Scomp_msg_tag
	{
		uint16_t n_16b;
		uint8_t n_bytes[2];
	};

public:


	Serial(std::string com);


	/// <summary>
	/// sincronizza arduino ed il pc
	/// </summary>
	void sinc();

	/// <summary>
	/// invia un floar ad arduino
	/// </summary>
	/// <param name="n"></param>
	void send_float(float n);

	/// <summary>
	/// riceve un float da arduino
	/// </summary>
	/// <returns></returns>
	float receive_float();

	void send_char(char ch);

	char receive_char();

	/// <summary>
	/// receive the data of acc, gy, magn from arduino
	/// </summary>
	/// <param name="acc_xyz"></param>
	/// <param name="g_xyz"></param>
	/// <param name="magn"></param>
	/// <param name="temp"></param>
	/// <returns>
	/// -> 0 if the trasmission is good
	/// -> 1 if it falied
	/// </returns>
	int receive_data(float* acc_xyz, float* g_xyz, float* magn, float* temp);

	~Serial()
	{
		port->close();
	}
};


#endif // #ifndef _SERIAL_HADLER_H_