
#include "pch.h"
#include "Serial_handler.h"



Serial::Serial(std::string com)
{
	for (uint8_t i = 0; i < 3; i++)
	{
		try
		{
			boost::asio::io_service io;
			port = new boost::asio::serial_port(io);
			port->open(com);
			port->set_option(boost::asio::serial_port_base::baud_rate(115200));
			break;
		}
		catch (const boost::system::system_error& e)
		{
			std::cout << "errore apertura com" << std::endl;
		}
	}

}


void Serial::sinc()
{
	char c;
	std::cout << "sinc" << std::endl;
	c = 'k';
	boost::asio::write(*port, boost::asio::buffer(&c, 1));
	std::cout << "write ok" << std::endl;
	do
	{
		boost::asio::read(*port, boost::asio::buffer(&c, 1));
		//cout << "carattere: " << c << endl;
	}
	while (c != 'y');
	std::cout << "sinc ok" << std::endl;
}


void Serial::send_float(float n)
{
	Scomp_float S_float;
	S_float.n_float = n;
	int writed_bytes = boost::asio::write(*port, boost::asio::buffer(&S_float.n_int, 4));
	if (writed_bytes != 4)
	{
		std::cout << "ho scritto meno di 4 byte: " << writed_bytes << std::endl;
	}
}


float Serial::receive_float()
{
	Scomp_float S_float;
	int readed_bytes = boost::asio::read(*port, boost::asio::buffer(&S_float.n_int, 4));
	if (readed_bytes != 4)
	{
		std::cout << "ho letto meno di 4 byte: " << readed_bytes << std::endl;
	}

	return S_float.n_float;
}


void Serial::send_char(char ch)
{
	boost::asio::write(*port, boost::asio::buffer(&ch, 1));
}



char Serial::receive_char()
{
	char ch;
	boost::asio::read(*port, boost::asio::buffer(&ch, 1));
	return ch;
}


int Serial::receive_data(float* acc_xyz, float* g_xyz, float* magn, float* temp)
{
	if (receive_char() != 'a')
	{
		//cout << "male 1" << endl;
		return 1;
	}
	for (int i = 0; i < 3; i++)
	{
		acc_xyz[i] = receive_float();
	}
	//cout << "a ok" << endl;
	if (receive_char() != 'g')
	{
		//cout << "male 2" << endl;
		return 1;
	}
	for (int i = 0; i < 3; i++)
	{
		g_xyz[i] = receive_float();
	}
	//cout << "g ok" << endl;
	if (receive_char() != 'm')
	{
		//cout << "male 3" << endl;
		return 1;
	}
	for (int i = 0; i < 3; i++)
	{
		magn[i] = receive_float();
	}
	//cout << "m ok" << endl;
	if (receive_char() != 't')
	{
		//cout << "male 4" << endl;
		return 1;
	}
	temp[0] = receive_float();
	//cout << "t ok" << endl;
	if (receive_char() != 4)
	{
		//cout << "male 5" << endl;
		return 1;
	}
	return 0;
}