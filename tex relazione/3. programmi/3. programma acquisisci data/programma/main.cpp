// boost_bluetooth.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <string>
#include <math.h>

#include "real_time.h"
#include "kin_file_manager.h"
#include "ard_file_manager.h"
#include "data_structure.h"

#include <boost/thread.hpp>

#include <boost/log/core.hpp>
#include <boost/call_traits.hpp>
#include <boost/circular_buffer.hpp>

#include <boost/container/vector.hpp>

#include <boost/log/attributes.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>


#include "Serial_handler.h"


#include <Windows.h>
#include <Kinect.h>
#include <Shlobj.h>



#define tot_esempi 20000

using namespace std;


namespace logger
{

	enum severity_level
	{
		normal,
		warning,
		error,
		critical
	};

	//BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
	BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level)
	BOOST_LOG_ATTRIBUTE_KEYWORD(tag_attr, "Tag", std::string)
	BOOST_LOG_ATTRIBUTE_KEYWORD(timeline, "Timeline", boost::log::attributes::timer::value_type)
	BOOST_LOG_ATTRIBUTE_KEYWORD(tread_id, "Tread_id", boost::thread::id)
	BOOST_LOG_ATTRIBUTE_KEYWORD(tread_name, "tread_name", std::string)

	// The operator puts a human-friendly representation of the severity level to the stream
	std::ostream& operator<< (std::ostream& strm, severity_level level)
	{
		static const char* strings[] =
		{
			" normal ",
			" warning ",
			"  error  ",
			"critical"
		};

		if (static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings))
		{
			strm << strings[level];
		}
		else
		{
			strm << static_cast<int>(level);
		}

		return strm;
	}

	//[ example_tutorial_trivial_with_filtering
	void init()
	{
		boost::shared_ptr<boost::log::sinks::synchronous_sink< boost::log::sinks::text_ostream_backend > > sink = boost::log::add_console_log();

		sink->locked_backend()->auto_flush(true);

		sink->set_formatter
		(
			boost::log::expressions::stream
			<< "id->(0x" << std::hex << std::setw(4) << std::setfill('0') << tread_id << "): "
			<< boost::log::expressions::if_(boost::log::expressions::has_attr(tread_name))
			[
				boost::log::expressions::stream << tread_name << " "
			]
		<< "<" << severity << ">\t"
			<< boost::log::expressions::if_(boost::log::expressions::has_attr(tag_attr))
			[
				boost::log::expressions::stream << "tag->[" << tag_attr << "] "
			]
		<< boost::log::expressions::if_(boost::log::expressions::has_attr(timeline))
			[
				boost::log::expressions::stream << "time->[" << timeline << "] "
			]
		<< " -> " << boost::log::expressions::smessage
			);

		boost::log::add_common_attributes();
	}


	boost::log::sources::severity_logger< severity_level > f_init()
	{
		boost::log::sources::severity_logger< logger::severity_level > slg;
		return slg;
	}

	void attr_thread(boost::log::sources::severity_logger< severity_level > *slg)
	{
		slg->add_attribute("Tread_id", boost::log::attributes::constant< boost::thread::id >(boost::this_thread::get_id()));
	}

	void attr_time(boost::log::sources::severity_logger< severity_level > *slg)
	{
		slg->add_attribute("Timeline", boost::log::attributes::timer());
	}

	void attr_tag(boost::log::sources::severity_logger< severity_level > *slg, std::string tag)
	{
		slg->add_attribute("Tag", boost::log::attributes::constant< std::string >(tag));
	}

	void attr_thread_name(boost::log::sources::severity_logger< severity_level > *slg, std::string name)
	{
		slg->add_attribute("tread_name", boost::log::attributes::constant< std::string >(name));
	}

};


template <class T>
class bounded_buffer
{
public:

	typedef boost::circular_buffer<T> container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::value_type value_type;
	typedef typename boost::call_traits<value_type>::param_type param_type;

	explicit bounded_buffer(size_type capacity) : m_unread(0), m_container(capacity) {}

	void push_front(param_type item)
	{
		boost::unique_lock<boost::mutex> lock(m_mutex);
		m_not_full.wait(lock, boost::bind(&bounded_buffer<value_type>::is_not_full, this));
		m_container.push_front(item);
		++m_unread;
		//assert(m_unread == m_container.size());
		//cout << "m_unread: " << m_unread << " m_container.size(): " << m_container.size() << endl;
		lock.unlock();
		m_not_empty.notify_one();
	}

	void pop_back(value_type* pItem)
	{
		boost::unique_lock<boost::mutex> lock(m_mutex);
		m_not_empty.wait(lock, boost::bind(&bounded_buffer<value_type>::is_not_empty, this));
		*pItem = m_container[--m_unread];
		//cout << "m_unread: " << m_unread << " m_container.size(): " << m_container.size() << endl;
		//assert(m_unread == m_container.size());
		lock.unlock();
		m_not_full.notify_one();
	}

	float percentage_of_filling()
	{
		return (m_unread / m_container.capacity())*100.0;
	}

	void flush()
	{
		m_container.clear();
		m_unread = 1;
		//assert(m_unread == m_container.size());
	}

private:
	bounded_buffer(const bounded_buffer&);              // Disabled copy constructor
	bounded_buffer& operator = (const bounded_buffer&); // Disabled assign operator

	bool is_not_empty() const
	{
		return m_unread > 0;
	}

	bool is_not_full() const
	{
		return m_unread < m_container.capacity();
	}

	size_type m_unread;
	container_type m_container;
	boost::mutex m_mutex;
	boost::condition_variable m_not_empty;
	boost::condition_variable m_not_full;
};



/// <summary>
/// classe per la gestione del kinect, da ristrutturare un po
/// </summary>
class kinect_class
{
public:

	// Current Kinect
	IKinectSensor* m_pKinectSensor;
	ICoordinateMapper* m_pCoordinateMapper;

	// Body reader
	IBodyFrameReader* m_pBodyFrameReader;

	real_time t;

	bounded_buffer<kinect_data>* FIFO_kin;

	kinect_class(bounded_buffer<kinect_data>* data_FIFO)
	{
		InitializeDefaultSensor();
		FIFO_kin = data_FIFO;
	}

	void start()
	{
		
		bool flag = false;
		while (true)
		{
			try
			{
				kinect_data data;

				uint64_t t1 = t.get_curr_time();

				flag = Update(&data);

				if (flag)
				{

					uint64_t t2 = t.get_curr_time();
					data.frame_time = (t1 + t2) / 2;

					//data.print_data();
					FIFO_kin->push_front(data);

					boost::this_thread::sleep(boost::posix_time::milliseconds(35)); //// se chidi piu' di 30 fps al kinect scoppia

				}

			}
			catch(boost::thread_interrupted const&)
			{
				break;
			}

		}
	}

	int InitializeDefaultSensor()
	{
		HRESULT hr;

		hr = GetDefaultKinectSensor(&m_pKinectSensor);
		if (FAILED(hr))
		{
			return 0;
		}

		if (m_pKinectSensor)
		{
			// Initialize the Kinect and get coordinate mapper and the body reader
			IBodyFrameSource* pBodyFrameSource = NULL;

			hr = m_pKinectSensor->Open();

			if (SUCCEEDED(hr))
			{
				hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
			}

			if (SUCCEEDED(hr))
			{
				hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
			}

			if (SUCCEEDED(hr))
			{
				hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
			}

			SafeRelease(pBodyFrameSource);
		}

		if (!m_pKinectSensor || FAILED(hr))
		{
			return 0;
		}

		return 1;
	}


	bool Update(kinect_data* data)
	{
		bool flag = false;
		if (!m_pBodyFrameReader)
		{
			return false;
		}

		IBodyFrame* pBodyFrame = NULL;
		
		HRESULT hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);

		if (SUCCEEDED(hr))
		{
			INT64 nTime = 0;

			hr = pBodyFrame->get_RelativeTime(&nTime);

			IBody* ppBodies[BODY_COUNT] = { 0 };

			if (SUCCEEDED(hr))
			{
				hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
			}

			if (SUCCEEDED(hr))
			{

				flag = ProcessBody(BODY_COUNT, ppBodies, data);
			}

			for (int i = 0; i < _countof(ppBodies); ++i)
			{
				SafeRelease(ppBodies[i]);
			}
		}

		SafeRelease(pBodyFrame);
		return flag;
		
	}


	bool handle_data(Joint* joints, kinect_data* data)
	{

		// ne sono JointType_Count

		for (int i = 0; i < data->number_of_joints; i++)
		{
			if (joints[data->needed_joint[i]].TrackingState == 2 || joints[data->needed_joint[i]].TrackingState == 1)
			{

				data->needed_joints[i].joint_name = data->needed_joint[i];//joints[data->needed_joint[i]].JointType;
				data->needed_joints[i].to_joint = data->to_joint[data->needed_joint[i]];
				data->needed_joints[i].position[0] = joints[data->needed_joint[i]].Position.X;
				data->needed_joints[i].position[1] = joints[data->needed_joint[i]].Position.Y;
				data->needed_joints[i].position[2] = joints[data->needed_joint[i]].Position.Z;

				//cout << "Joint " << data->needed_joint[i] << "  x: " << data->needed_joints[i].position[0] << "  y: " << data->needed_joints[i].position[1] << "  z: " << data->needed_joints[i].position[2] << endl;

			}
		}


		// do someting whith the data
		if (data->full_frame())
		{
			//data->print_data();
			return true;
		}
		else
		{
			return false;
		}

	}

	/// <summary>
	/// Handle new body data
	/// <param name="nBodyCount">body data count</param>
	/// <param name="ppBodies">body data in frame</param>
	/// </summary>
	bool ProcessBody(int nBodyCount, IBody** ppBodies, kinect_data* data)
	{
		HRESULT hr;

		if (m_pCoordinateMapper)
		{
			for (int i = 0; i < nBodyCount; ++i)
			{
				IBody* pBody = ppBodies[i];
				if (pBody)
				{
					BOOLEAN bTracked = false;
					hr = pBody->get_IsTracked(&bTracked);

					if (SUCCEEDED(hr) && bTracked)
					{
						Joint joints[JointType_Count];

						hr = pBody->GetJoints(_countof(joints), joints);
						if (SUCCEEDED(hr))
						{
							return handle_data(joints, data);
						}
					}
				}
			}
		}

		return false;
	}

	/// <summary>
	/// Destructor
	/// </summary>
	~kinect_class()
	{
		// done with body frame reader
		SafeRelease(m_pBodyFrameReader);

		// done with coordinate mapper
		SafeRelease(m_pCoordinateMapper);

		// close the Kinect Sensor
		if (m_pKinectSensor)
		{
			m_pKinectSensor->Close();
		}

		SafeRelease(m_pKinectSensor);
	}

	template<class Interface>
	inline void SafeRelease(Interface *& pInterfaceToRelease)
	{
		if (pInterfaceToRelease != NULL)
		{
			pInterfaceToRelease->Release();
			pInterfaceToRelease = NULL;
		}
	}

};


class arduino_class
{
public:
	Serial* ser;

	real_time t;

	bounded_buffer<arduino_data>* FIFO_ard;

	arduino_class(bounded_buffer<arduino_data>* data_FIFO, string com_port)
	{
		ser = new Serial(com_port);
		cout << "serial ok" << endl;
		FIFO_ard = data_FIFO;
		ser->sinc();
	}


	void start()
	{
		while (true)
		{

			arduino_data data;

			uint64_t t1 = t.get_curr_time();

			ser->receive_data(data.acc_xyz, data.gy_xyz, data.magn_xyz, &data.temp);

			uint64_t t2 = t.get_curr_time();

			data.frame_time = (t1 + t2) / 2;

			FIFO_ard->push_front(data);

		}
	}

};


/// <summary>
/// handle the data from the kinect and write it in the file
/// </summary>
class kin_handler
{
private:
	bounded_buffer<kinect_data>* kin;
	kin_file_manager* f;

public:
	kin_handler(bounded_buffer<kinect_data>* kin, string file_name)
	{
		this->kin = kin;
		f = new kin_file_manager(file_name, std::ios::out);
	}

	void start()
	{
		auto slg = logger::f_init();
		attr_thread(&slg);
		logger::attr_thread_name(&slg, "kin_handler");

		BOOST_LOG_SEV(slg, logger::normal) << "hello log";

		for (int i = 0; i < tot_esempi; i++)
		{
			kinect_data data_kin;
			kin->pop_back(&data_kin);

			data_kin.updateAngles();

			f->write_data_line(data_kin);
			//data_kin.print_data();
		}

		BOOST_LOG_SEV(slg, logger::normal) << "end";
	}

};



/// <summary>
/// handle the data from arduino and write it on a file
/// </summary>
class ard_handler
{
private:
	bounded_buffer<arduino_data>* ard;
	ard_file_manager* f;

public:
	ard_handler(bounded_buffer<arduino_data>* ard, string file_name)
	{
		this->ard = ard;
		f = new ard_file_manager(file_name, std::ios::out);
	}

	void start()
	{
		auto slg = logger::f_init();
		attr_thread(&slg);
		logger::attr_thread_name(&slg, "ard_handler");

		BOOST_LOG_SEV(slg, logger::normal) << "hello log";

		for (int i = 0; i < tot_esempi; i++)
		{
			arduino_data data_ard;
			ard->pop_back(&data_ard);

			f->write_data_line(data_ard);

			//data_ard.print_data();

		}

		BOOST_LOG_SEV(slg, logger::normal) << "end";
	}

};



template<typename type_in, typename type_out>
class dataset_file_manager
{
private:
	std::fstream f;
	std::ios::_Openmode mode;

	std::size_t n_in;
	std::size_t n_out;

public:
	dataset_file_manager(std::string file_name, std::ios::_Openmode mode)
	{
		this->mode = mode;
		f.open(file_name, mode);
	}

	template<std::size_t N, std::size_t M>
	void write_data_line(dataset_data<type_in, N, type_out, M> dat)
	{
		if (mode != std::ios::out) { cout << "################# errore file" << endl;  return; }

		int i = 0;
		for (i; i < dat.in.size() - 1; i++)
		{
			f << dat.in[i] << " ";
		}
		f << dat.in[i] << std::endl;
		i = 0;
		for (i; i < dat.out.size() - 1; i++)
		{
			f << dat.out[i] << " ";
		}
		f << dat.out[i] << std::endl;

		f.flush();
	}


	template<std::size_t N, std::size_t M>
	bool read_data_line(dataset_data<type_in, N, type_out, M> *dat)
	{
		if (mode != std::ios::in) { cout << "################# errore file" << endl; return 0; }
		if (!f) { cout << "end of file" << endl; return 0; }

	}


	/// <summary>
	/// structure
	/// - NE xxx -> numero esempi
	/// - IN xx -> numero input
	/// - out xx -> numero output
	/// - ES -> tag di inizio esempi
	/// </summary>
	void write_header(uint32_t numero_esempi, uint32_t numero_input, uint32_t numero_out)
	{
		if (mode != std::ios::out) { cout << "################# errore file" << endl; return; }

		f << "NE " << numero_esempi << std::endl << "IN " << numero_input << std::endl << "OUT " << numero_out << std::endl << "ES" << std::endl;

		n_in = numero_input;
		n_out = numero_out;
	}


	/// <summary>
	/// not used
	/// </summary>
	/// <param name="numero_esempi"></param>
	/// <param name="numero_input"></param>
	/// <param name="numero_out"></param>
	void read_header(uint32_t *numero_esempi, uint32_t *numero_input, uint32_t *numero_out)
	{

	}


	~dataset_file_manager()
	{
		f.close();
	}

};



class make_dataset
{
private:

	ard_file_manager *f_a;
	kin_file_manager *f_k;

	const int range = 35; /// delta di accettazione in millisecondi

	list<arduino_data> a_dat;
	list<kinect_data> k_dat;

	// assolutamente da mettere quelli giusti.
	static const std::size_t n_in = 9;
	static const std::size_t n_out = 4;

	list<dataset_data<float, n_in, float, n_out>> dataset;


	void write_dataset(string dataset_file_name)
	{
		dataset_file_manager<float, float> dat(dataset_file_name, std::ios::out);
		dat.write_header(dataset.size(), n_in, n_out);

		for (auto i : dataset)
		{
			//i.print_data();
			dat.write_data_line<n_in, n_out>(i);
		}
	}


	/// <summary>
	/// tell if the two time are in the established range
	/// </summary>
	/// <param name="t1">time 1</param>
	/// <param name="t2">time 2</param>
	/// <returns>
	/// 1 if the time between t1 and t2 < range
	/// 0 otherwise
	/// </returns>
	bool check_sync(uint64_t t1, uint64_t t2)
	{
		if (abs((long long)(t1 - t2)) < range)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

	void transfer_data_to_dataset(arduino_data* ard_iter, kinect_data* kin_iter)
	{
		dataset_data<float, n_in, float, n_out> es;
		int cont = 0;

		for (size_t i = 0; i < 3; i++, cont++)
		{
			es.in[cont] = ard_iter->acc_xyz[i];
		}
		for (size_t i = 0; i < 3; i++, cont++)
		{
			es.in[cont] = ard_iter->gy_xyz[i];
		}
		for (size_t i = 0; i < 3; i++, cont++)
		{
			es.in[cont] = ard_iter->magn_xyz[i];
		}

		cont = 0;
		for (int i = 0; i < 2; i++)
		{
			for (int j = 1; j < 3; j++)
			{
				es.out[cont] = kin_iter->needed_joints[i].angle[j];
				cont++;
			}
		}

		dataset.push_back(es);
	}

	uint32_t time_distance(uint32_t t1, uint32_t t2)
	{
		return abs((long long)(t1 - t2));
	}


	void sync_with_clones()
	{
		uint64_t time_old_good_pair = 0;

		uint64_t time_old_ard = 0;
		uint64_t time_old_kin = 0;

		auto ard_iter = a_dat.begin();
		auto kin_iter = k_dat.begin();

		uint32_t ard_cont = 0;
		uint32_t kin_cont = 0;

		uint32_t old_ard_cont = 0;
		uint32_t old_kin_cont = 0;

		while (true)
		{
			if (check_sync(ard_iter->frame_time, kin_iter->frame_time))
			{
				transfer_data_to_dataset(&(*ard_iter), &(*kin_iter));

				cout << abs((long long)(ard_iter->frame_time - kin_iter->frame_time)) << "  kin: " << kin_cont << " ard: " << ard_cont << endl;
				cout << "time since another good pair: " << ((ard_iter->frame_time + kin_iter->frame_time) / 2 - time_old_good_pair) << endl;
				cout << "ard frame rate: " << (1.0f / ((ard_iter->frame_time - time_old_ard) / 1000.0f)) << "Hz" << endl;
				cout << "kin frame rate: " << (1.0f / ((kin_iter->frame_time - time_old_kin) / 1000.0f)) << "Hz" << endl;


				time_old_good_pair = (ard_iter->frame_time + kin_iter->frame_time) / 2;

				time_old_ard = ard_iter->frame_time;
				time_old_kin = kin_iter->frame_time;


				if ((kin_cont == old_kin_cont) || (ard_cont == old_ard_cont))
				{
					cout << "dati multipli nello stesso range" << endl;
				}
				else
				{
					old_kin_cont = kin_cont;
					old_ard_cont = ard_cont;
				}

				cout << endl;

			}
			else
			{
				cout << "dato scartato" << endl << endl;
			}

			if (ard_iter->frame_time >= kin_iter->frame_time)
			{
				kin_iter++;
				kin_cont++;
			}
			else
			{
				ard_iter++;
				ard_cont++;
			}

			if ((kin_iter == k_dat.end()) || (ard_iter == a_dat.end())) { break; }
		}
	}


	void sync_without_clones()
	{
		uint64_t time_old_good_pair = 0;

		uint64_t time_old_ard = 0;
		uint64_t time_old_kin = 0;

		auto ard_iter = a_dat.begin();
		auto kin_iter = k_dat.begin();

		uint32_t ard_cont = 0;
		uint32_t kin_cont = 0;

		uint32_t old_ard_cont = 0;
		uint32_t old_kin_cont = 0;

		kinect_data k_tmp = *kin_iter;
		arduino_data a_tmp = *ard_iter;

		//for (size_t i = 0; i < 300; i++)
		for(;;)
		{
			if (check_sync(ard_iter->frame_time, kin_iter->frame_time))
			{

				cout << "common " << abs((long long)(ard_iter->frame_time - kin_iter->frame_time)) << "  kin: " << kin_iter->contatore << " ard: " << ard_iter->contatore << endl;
				cout << "ard frame rate: " << (1.0f / ((ard_iter->frame_time - time_old_ard) / 1000.0f)) << "Hz" << endl;
				cout << "kin frame rate: " << (1.0f / ((kin_iter->frame_time - time_old_kin) / 1000.0f)) << "Hz" << endl;

				time_old_ard = ard_iter->frame_time;
				time_old_kin = kin_iter->frame_time;


				if (kin_cont == old_kin_cont)
				{
					cout << "multi ard in the same kin" << endl;
					if (time_distance(kin_iter->frame_time, ard_iter->frame_time) < time_distance(k_tmp.frame_time, a_tmp.frame_time))
					{
						a_tmp = *ard_iter;
					}
				}
				else if (ard_cont == old_ard_cont)
				{
					cout << "multi kin in the same ard" << endl;
					if (time_distance(kin_iter->frame_time, ard_iter->frame_time) < time_distance(k_tmp.frame_time, a_tmp.frame_time))
					{
						k_tmp = *kin_iter;
					}
				}
				else
				{
					cout << "sequence reset" << endl;

					transfer_data_to_dataset(&a_tmp, &k_tmp);

					cout << "best " << abs((long long)(a_tmp.frame_time - k_tmp.frame_time)) << "  kin: " << k_tmp.contatore << " ard: " << a_tmp.contatore << endl;

					cout << "time since another good pair: " << ((a_tmp.frame_time + k_tmp.frame_time) / 2 - time_old_good_pair) << endl;
					

					time_old_good_pair = (a_tmp.frame_time + k_tmp.frame_time) / 2;

					old_kin_cont = kin_cont;
					old_ard_cont = ard_cont;

					// need to save the data

					a_tmp = *ard_iter;
					k_tmp = *kin_iter;
				}

				cout << endl;

			}
			else
			{
				cout << "dato scartato" << endl << endl;
			}

			if (ard_iter->frame_time >= kin_iter->frame_time)
			{
				kin_iter++;
				kin_cont++;
			}
			else
			{
				ard_iter++;
				ard_cont++;
			}

			if ((kin_iter == k_dat.end()) || (ard_iter == a_dat.end())) { break; }
		}
	}


	void load_data()
	{
		bool flag = true;
		uint64_t c = 0;

		while (flag)
		{
			arduino_data tmp;
			flag = f_a->read_data_line(&tmp);
			tmp.contatore = c;
			a_dat.push_back(tmp);
			c++;
		}

		flag = true;
		c = 0;

		while (flag)
		{
			kinect_data tmp;
			flag = f_k->read_data_line(&tmp);
			tmp.contatore = c;
			k_dat.push_back(tmp);
			c++;
		}
	}


public:
	make_dataset(string ard_file_name, string kin_file_name, string dataset_file_name, bool want_clones)
	{
		f_a = new ard_file_manager(ard_file_name, std::ios::in);
		f_k = new kin_file_manager(kin_file_name, std::ios::in);

		load_data();
		
		if (want_clones)
		{
			sync_with_clones();
		}
		else
		{
			sync_without_clones();
		}

		write_dataset(dataset_file_name);

	}

	~make_dataset()
	{
		delete f_a;
		delete f_k;
	}

};


/// <summary>
/// ok
/// </summary>
void start_aquire_data(string out_ard, string out_kin, string com_port)
{
	auto slg = logger::f_init();
	attr_thread(&slg);
	logger::attr_thread_name(&slg, "main");

	bounded_buffer<arduino_data> data_FIFO_ard(20);
	bounded_buffer<kinect_data> data_FIFO_kin(20);

	arduino_class a(&data_FIFO_ard, com_port);
	kinect_class k(&data_FIFO_kin);

	ard_handler a_h(&data_FIFO_ard, out_ard);
	kin_handler k_h(&data_FIFO_kin, out_kin);

	BOOST_LOG_SEV(slg, logger::normal) << "costructor end" << std::endl;

	boost::thread ard([&a]() { a.start(); });
	boost::thread kin([&k]() { k.start(); });

	BOOST_LOG_SEV(slg, logger::normal) << "producer thread launced" << std::endl;

	boost::thread ard_h([&a_h]() { a_h.start(); });
	boost::thread kin_h([&k_h]() { k_h.start(); });

	BOOST_LOG_SEV(slg, logger::normal) << "consumer thread launched" << std::endl;

	kin.join();
	ard.join();
	kin_h.join();
	ard_h.join();

}


void post_elaborate()
{
	string ard_file_mane = "";
	string kin_file_mane = "";
	string dataset_file_name = "";
	string tmp = "";
	bool clones = false;
	
	cout << "insert the input file names: " << endl
		<< "\t arduino -> ";
	std::getline(std::cin, ard_file_mane);
	cout << "\t kinekt -> ";
	std::getline(std::cin, kin_file_mane);

	cout << "insert the ouptut file name -> : ";
	std::getline(std::cin, dataset_file_name);

	cout << "do you want clones in the data? [y/n] -> : ";
	std::getline(std::cin, tmp);

	if (tmp == "y")
	{
		clones = true;
	}

	make_dataset(ard_file_mane, kin_file_mane, dataset_file_name, clones);
}



void acquire_data()
{
	string ard_file_mane = "";
	string kin_file_mane = "";
	string arduino_com_port = "";

	cout << "insert the file names: " << endl
		<< "\t arduino -> ";
	std::getline(std::cin, ard_file_mane);
	cout << "\t kinekt -> ";
	std::getline(std::cin, kin_file_mane);

	cout << "insert the arduino com port -> : ";
	std::getline(std::cin, arduino_com_port);

	start_aquire_data(ard_file_mane, kin_file_mane, arduino_com_port);
}


int main()
{
	logger::init();

	auto slg = logger::f_init();
	attr_thread(&slg);
	attr_time(&slg);
	logger::attr_thread_name(&slg, "main");

	BOOST_LOG_SEV(slg, logger::normal) << "hello log";

	acquire_data();

	//post_elaborate();

	return 0;
}

