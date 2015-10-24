#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


struct Configuration
{
public:
	enum CommunicationMode {
		ETHERNET,
		SERIAL,
		COM
	};

protected:
	typedef std::unordered_map<std::string, CommunicationMode> CommunicationModeMap;
	typedef boost::property_tree::ptree PropertyTree;

	Configuration(PropertyTree& pt, CommunicationModeMap communicationModeMap) :
		camera	{ pt.get_child("camera") },
		mBed	{ pt.get_child("mBed"), communicationModeMap },
		field	{ pt.get_child("field") },
		robot	{ pt.get_child("robot") } {}

public:
	static Configuration* newInstance(std::string configFileName)
	{
		PropertyTree propertyTree;
		boost::property_tree::read_json(configFileName, propertyTree);
		CommunicationModeMap communicationModeMap = CommunicationModeMap({
			{ "ETHERNET", ETHERNET },
			{ "SERIAL", SERIAL },
			{ "COM", COM }
		});
		try {
			return new Configuration(propertyTree, communicationModeMap);
		}
		catch (const std::out_of_range& e) {
			std::ostringstream message;
			message
				<< e.what() << std::endl
				<< "Could not translate string from configuration to enum - check configuratin file.";
			throw std::runtime_error(message.str());
		}
		catch (const boost::property_tree::ptree_bad_path& e)
		{
			std::ostringstream message;
			message
				<< e.what() << std::endl
				<< "Probably a key is missing from configuration file";
			throw std::runtime_error(message.str());
		}
	}


	struct CameraConfiguration
	{
		CameraConfiguration(PropertyTree pt) :
			frontSerial{ pt.get<int>("serial.front") },
			rearSerial{ pt.get<int>("serial.rear") },
			width{ pt.get<int>("resolution.width") },
			height{ pt.get<int>("resolution.height") },
			gain{ pt.get<int>("settings.gain") },
			exposure{ pt.get<int>("settings.exposure") }

		{}
		const int frontSerial;
		const int rearSerial;
		const int width;
		const int height;
		const int gain;
		const int exposure;
	}camera;

	struct MBedConfiguration
	{
		MBedConfiguration(PropertyTree pt, CommunicationModeMap communicationModeMap) :
			communicationMode(communicationModeMap.at(pt.get<std::string>("communicationMode"))),
			ethernetIp(pt.get<std::string>("ethernet.ip")),
			ethernetPort{ pt.get<int>("ethernet.port") },
			serialIdentificatonString(pt.get<std::string>("serial.identificationString")),
			serialBaud{ pt.get<int>("serial.baud") }
		{}
		const CommunicationMode communicationMode;
		const std::string ethernetIp;
		const int ethernetPort;
		const std::string serialIdentificatonString;
		const int serialBaud;
	}mBed;

	struct Field
	{
		Field(boost::property_tree::ptree pt) :
			width{ pt.get<float>("width") },
			height{ pt.get<float>("height") } {}

		const float width;
		const float height;
	}field;

	struct Robot
	{
		Robot(boost::property_tree::ptree pt) :
			radius{ pt.get<float>("radius") },
			dribblerDistance{ pt.get<float>("dribblerDistance") },
			wheelRadius{ pt.get<float>("wheel.radius") },
			wheelAngles{_wheelAngles},
			wheelDiagonalOffset{ pt.get<float>("wheel.diagonalOffset") }
		{
			_wheelAngles[0] = pt.get<float>("wheel.angle1");
			_wheelAngles[1] = pt.get<float>("wheel.angle2");
			_wheelAngles[2] = pt.get<float>("wheel.angle3");
			_wheelAngles[3] = pt.get<float>("wheel.angle4");
		}

		const float radius;
		const float dribblerDistance;
		const float wheelRadius;
		const float * const wheelAngles;
		const float wheelDiagonalOffset;
		
	private:
		float _wheelAngles[4];
	}robot;
	
};



#endif
