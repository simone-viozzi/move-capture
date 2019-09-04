
#include "pch.h"
#include "syncSocketServer.h"


syncSocketServer::syncSocketServer(int port, string ipV = "v4")
{
	io_context = new boost::asio::io_context;
	//assert(ipV == "v4" || ipV == "v6");
	if (ipV == "v4") { acceptor = new boost::asio::ip::tcp::acceptor(*io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)); }
	else if (ipV == "v6") { acceptor = new boost::asio::ip::tcp::acceptor(*io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v6(), port)); }
	socket = new boost::asio::ip::tcp::socket(*io_context);
}

void syncSocketServer::waitConnections()
{
	try
	{
		cout << "sto aspettando" << endl;
		acceptor->accept(*socket); // istanza di connessione (una volta lanciato si blocca fino alla connessione)
	}
	catch (exception& e)
	{
		cerr << e.what() << endl;
	}
}




void syncSocketServer::closeConnection()
{
	socket->close();
	cout << "connessione terminata" << endl;
}



string syncSocketServer::reciveString()
{

	string recivedMsg = "";
	boost::system::error_code error;
	try
	{
		boost::array<char, 128> buf;
		while (recivedMsg == "")
		{
			//ricezione
			size_t len = socket->read_some(boost::asio::buffer(buf), error);
			recivedMsg = string(buf.data(), len);
			cout << recivedMsg << endl;
			//cout.write(buf.data(), len);
		}
	}
	catch (exception& e)
	{
		cerr << e.what() << endl;
	}
	return recivedMsg;
}


void syncSocketServer::sendString(string msg)
{
	try
	{
		boost::system::error_code ignored_error;
		boost::asio::write(*socket, boost::asio::buffer(msg), ignored_error);
	}
	catch (exception& e)
	{
		cerr << e.what() << endl;
	}
}


void syncSocketServer::reciveValues()
{
	try
	{
		boost::system::error_code error;
		boost::array<char, 128> buf;
		size_t len = socket->read_some(boost::asio::buffer(buf), error);
		if (len == sizeof(bones.strBones))
		{
			for (int i = 0; i < len; i++)bones.strBones[i] = buf[i];
		}
		else
		{
			cout << "Valori ricevuti non corretti" << endl;
		}
	}
	catch (exception& e)
	{
		cerr << e.what() << endl;
	}
}



void syncSocketServer::sendValues()
{
	try
	{
		boost::array<char, 128> buf;
		boost::system::error_code ignored_error;
		boost::asio::write(*socket, boost::asio::buffer(bones.strBones), ignored_error);
	}
	catch (exception& e)
	{
		cerr << e.what() << endl;
	}
}

void syncSocketServer::initBones(float value)
{
	for (int i = 0; i < number_of_joints * 3; i++)bones.fBones[i] = value;
}

void syncSocketServer::printBones()
{
	for (int i = 0; i < number_of_joints * 3; i++) { cout << bones.fBones[i] << "  "; if (((i + 1) % 3) == 0) cout << "||"; }
	cout << endl << endl;
}

void syncSocketServer::testComunication()
{
	string request = reciveString();
	if (request == "gf")
	{
		sendValues();
		printBones();
		initBones(x);
		x += 1.0f;
		Sleep(1);
	}
}



void syncSocketServer::sendSkeleton()
{
	string request = reciveString();
	if (request == "gf")
	{
		sendValues();
		printBones();
	}
}



void  syncSocketServer::refreshUbones(kinect_data *data)
{
	for (int i = 0; i < data->number_of_joints; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			bones.fBones[(i * 3) + j] = data->needed_joints[i].angle[j];
		}
	}
}
















