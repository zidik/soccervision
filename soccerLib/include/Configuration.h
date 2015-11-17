#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/bind.hpp>


class PropertyTreeMerger
{
	boost::property_tree::ptree mPt;
public:
	void set_ptree(const boost::property_tree::ptree &pt) {
		mPt = pt;
	}

	void update_ptree(const boost::property_tree::ptree &pt) {
		traverse(pt, boost::bind(&PropertyTreeMerger::merge, this, _1, _2, _3));
	}

	boost::property_tree::ptree get_ptree() {
		return mPt;
	}

protected:
	void merge(const boost::property_tree::ptree &parent, const boost::property_tree::ptree::path_type &childPath, const boost::property_tree::ptree &child) {
		mPt.put(childPath, child.data());
	}
	template<typename T>
	void traverse_recursive(const boost::property_tree::ptree &parent, const boost::property_tree::ptree::path_type &childPath, const boost::property_tree::ptree &child, T &method)
	{
		using boost::property_tree::ptree;

		method(parent, childPath, child);
		for (ptree::const_iterator it = child.begin(); it != child.end(); ++it) {
			ptree::path_type curPath = childPath / ptree::path_type(it->first);
			traverse_recursive(parent, curPath, it->second, method);
		}
	}
	template<typename T>
	void traverse(const boost::property_tree::ptree &parent, T &method)
	{
		traverse_recursive(parent, "", parent, method);
	}
};

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
		robot	{ pt.get_child("robot") },
		particleFilter{ pt.get_child("particleFilter") } {}


public:
	static Configuration* newInstance(std::vector<std::string> configFilePaths)
	{
		PropertyTreeMerger merger;
		for(std::string path : configFilePaths)
		{
			PropertyTree tree;
			boost::property_tree::read_json(path, tree);
			merger.update_ptree(tree);
		}
		CommunicationModeMap communicationModeMap = CommunicationModeMap({
			{ "ETHERNET", ETHERNET },
			{ "SERIAL", SERIAL },
			{ "COM", COM }
		});

		PropertyTree propertyTree = merger.get_ptree();
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


	struct CameraConf
	{
		CameraConf(PropertyTree pt) :
			frontSerial{ pt.get<int>("serial.front") },
			rearSerial{ pt.get<int>("serial.rear") },
			width{ pt.get<int>("resolution.width") },
			height{ pt.get<int>("resolution.height") },
			gain{ pt.get<int>("settings.gain") },
			exposure{ pt.get<int>("settings.exposure") },
			frontA{pt.get<float>("translation.front.A")},
			frontB{ pt.get<float>("translation.front.B") },
			frontC{ pt.get<float>("translation.front.C") },
			frontHorizon{ pt.get<float>("translation.front.horizon") },
			rearA{ pt.get<float>("translation.rear.A") },
			rearB{ pt.get<float>("translation.rear.B") },
			rearC{ pt.get<float>("translation.rear.C") },
			rearHorizon{ pt.get<float>("translation.rear.horizon") },
			pathBlobberConf( pt.get<std::string>("path.blobber") ),
			pathDistortFrontX( pt.get<std::string>("path.distortFrontX") ),
			pathDistortFrontY( pt.get<std::string>("path.distortFrontY") ),
			pathDistortRearX( pt.get<std::string>("path.distortRearX") ),
			pathDistortRearY( pt.get<std::string>("path.distortRearY") ),
			pathScreenshotsDir( pt.get<std::string>("path.screenshotsDir") )

		{}
		const int frontSerial;
		const int rearSerial;
		const int width;
		const int height;
		const int gain;
		const int exposure;

		const float frontA;
		const float frontB;
		const float frontC;
		const float frontHorizon;
		const float rearA;
		const float rearB;
		const float rearC;
		const float rearHorizon;

		const std::string pathBlobberConf;
		const std::string pathDistortFrontX;
		const std::string pathDistortFrontY;
		const std::string pathDistortRearX;
		const std::string pathDistortRearY;
		const std::string pathScreenshotsDir;

	}camera;

	struct MBedConf
	{
		MBedConf(PropertyTree pt, CommunicationModeMap communicationModeMap) :
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

	struct FieldConf
	{
		FieldConf(boost::property_tree::ptree pt) :
			width{ pt.get<float>("width") },
			height{ pt.get<float>("height") } {}

		const float width;
		const float height;
	}field;
	
	struct RobotConf
	{
		RobotConf(boost::property_tree::ptree pt) :
			radius{ pt.get<float>("radius") },
			dribblerDistance{ pt.get<float>("dribblerDistance") },
			rotationDir{ pt.get<int>("rotateDir") },
			wheelRadius{ pt.get<float>("wheel.radius") },
			wheelAngles{_wheelAngles},
			maxWheelSpeed{ pt.get<int>("wheel.maxSpeed") },
			wheelSpeedToMetric{ pt.get<float>("wheel.speedToMetric") },
			wheelDiagonalOffset{ pt.get<float>("wheel.diagonalOffset") },
			dribblerSpeed{ pt.get<int>("dribbler.speed") },
			dribblerNormalLowerLimit{ pt.get<int>("dribbler.normalLowerLimit") },
			dribblerNormalUpperLimit{ pt.get<int>("dribbler.normalUpperLimit") },
			dribblerChipKickLowerLimit{ pt.get<int>("dribbler.chipKickLowerLimit") },
			dribblerChipKickUpperLimit{ pt.get<int>("dribbler.chipKickUpperLimit") },
			dribblerMoveDuration{ pt.get<float>("dribbler.moveDuration") },
			dribblerStabilityDelay{ pt.get<float>("dribbler.stabilityDelay") },
			minServoLimit{ pt.get<int>("dribbler.maxServoLimit") },
			maxServoLimit{ pt.get<int>("dribbler.minServoLimit") }
		{
			_wheelAngles[0] = pt.get<float>("wheel.angle1");
			_wheelAngles[1] = pt.get<float>("wheel.angle2");
			_wheelAngles[2] = pt.get<float>("wheel.angle3");
			_wheelAngles[3] = pt.get<float>("wheel.angle4");
		}

		const float radius;
		const float dribblerDistance;
		const int rotationDir;
		const float wheelRadius;
		const float * const wheelAngles;
		const float wheelDiagonalOffset;
		const int maxWheelSpeed;
		const float wheelSpeedToMetric;
		const int dribblerSpeed;
		const int dribblerNormalLowerLimit;
		const int dribblerNormalUpperLimit;
		const int dribblerChipKickLowerLimit;
		const int dribblerChipKickUpperLimit;
		const float dribblerMoveDuration;
		const float dribblerStabilityDelay;
		const int minServoLimit;
		const int maxServoLimit;
		
	private:
		float _wheelAngles[4];
	}robot;

	struct ParticleFilterConf
	{
		ParticleFilterConf(boost::property_tree::ptree pt) :
			particleCount{ pt.get<int>("particleCount") },
			forwardNoise{ pt.get<float>("forwardNoise") },
			turnNoise{ pt.get<float>("turnNoise") } {}

		const int particleCount;
		const float forwardNoise;
		const float turnNoise;
	}particleFilter;
	
};



#endif
