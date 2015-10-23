#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace Config {
	enum CommunicationMode {
		ETHERNET,
		SERIAL,
		COM
	};

	const CommunicationMode communicationMode = ETHERNET;
	//const CommunicationMode communicationMode = SERIAL;
	//const CommunicationMode communicationMode = COM;

	// camera serials
	//telliskivi kaamerad
	const int frontCameraSerial = 857769553;
	const int rearCameraSerial = 857735761;

	//Oskari kodus olev kaamera
	//const int frontCameraSerial = 391119441;

	//Üks laboris olevatest kaameratest, mille oskar kokku pani
	//const int frontCameraSerial = 374363729;

	// indexes of motors according to the communication messages
	const int wheelFLId = 0;
	const int wheelFRId = 1;
	const int wheelRLId = 2;
	const int wheelRRId = 3;
	const int dribblerId = 4;

	// ethernet communication host and port
	const std::string communicationHost = "192.168.4.1";
	const int communicationPort = 8042;

	// serial device and baud
	//const std::string serialDeviceContains = "mbed";
	const std::string serialDeviceContains = "Mbed Virtual";
	//const int serialBaud = 230400;
	const int serialBaud = 115200;
	//const int serialBaud = 57600;

	// camera resolution
	const int cameraWidth = 1280;
	const int cameraHeight = 1024;
	const float cameraFovDistance = 5.0f;
	//const float cameraFovAngle = 56.0f * 3.14f / 180.0f;
	const float cameraFovAngle = 100.0f * 3.14f / 180.0f;
	const float cameraFovWidth = tan(cameraFovAngle / 2.0f) * cameraFovDistance * 2.0f;
	const int cameraGain = 1;
	//const int cameraGain = 6;
	const int cameraExposure = 10000;

	// default startup controller name
	const std::string defaultController = "test";

	// how big of a buffer to allocate for generating jpeg images
	const int jpegBufferSize = 5000 * 1024;

	// constants for camera correction
	//const float cameraCorrectionK = 0.00000049f;
	//const float cameraCorrectionZoom = 0.969f;
	//const float cameraCorrectionK = -0.00000013f;
	//const float cameraCorrectionZoom = 1.100f;

	// field dimensions
	const float fieldWidth = 4.5f;
	const float fieldHeight = 3.0f;

	// confinement factor in meters for confining objects on the field
	const float confineMargin = 0.0f;

	// how big is the ball/goal blobs distance to still be considered overlapping
	const int goalOverlapMargin = 15;
	const int ballOverlapMargin = 5;

	// minimum areas of blobs for objects
	const int ballBlobMinArea = 4;
	const int goalBlobMinArea = 16;

	// minimum area for objects to be considered valid
	const int ballMinArea = 4;
	const int goalMinArea = 64;

	// maximum width/height ratio for objects to be considered valid
	const float maxBallSizeRatio = 5.0f;

	//minimum width of robot blob
	const float robotMinWidth = 0.04f;

	//Minimum density of merged robot blob
	const float robotMinDensity = 0.05f;

	//Maximum valid distance of robot, taken from field size
	const float robotMaxDistance = 5.5f;

	//Maximum ratio for valid pixels to scanned pixels when searching for robot markers
	const float robotScanMinMatchRatio = 0.15f;

	// goals with area over this value are definately considered to be valid
	const int goalCertainArea = 10000;

	// if a goal starts lower than this value then it's not considered valid
	//const int goalTopMaxY = 10;
	const int goalTopMaxY = 165; // the wide-angle lens is very distorted..

	// goal top corners should be at least this far away
	// TODO this should be higher to be helpful but causes false-positives
	const float goalTopMinDistance = 3.0f;

	// surround metric is taken into account if ball bottom is below this threshold
	const int surroundSenseThresholdY = cameraHeight - 180;

	// minimum object metric thresholds to be considered valid
	const float minValidBallSurroundThreshold = 0.4f;
	const float minValidBallPathThreshold = 0.75f;
	const float minValidGoalPathThreshold = 0.65f;
	const int maxGoalInvalidColorCount = 10;

	// the ball/goal bottom needs to be below this line to consider path metric
	const int ballPathSenseStartY = cameraHeight - 160;
	const int goalPathSenseStartY = cameraHeight - 200;

	// color sense start Y
	const int colorDistanceStartY = cameraHeight - 160;

	// maximum ball surround metric sense radius
	const int maxBallSenseRadius = 250;

	// ball is considered to be in the goal if it's surrounded by goal colors by more than this
	const float ballInGoalSurroundThreshold = 0.6f;

	// whether ball is in the goal is considered only if it's closer than this
	const float ballInGoalConsiderMaxDistance = 1.0f;

	// additional linear distance correction to apply to object distances lookup
	const float distanceCorrection = 0.0f;

	// base number of steps used for underside metric
	const int undersideMetricBaseSteps = 20;

	// if underside metric is calculated below this point and no white nor black is seen, the metric returns zero
	const int undersideMetricBlackWhiteMinY = 100;

	// at least how many goal colors needed to be matched in underside metric
	const int undersideMetricValidGoalMinMatches = 10;

	// obstruction calculation parameters
	const int obstructionsStartY = 60;
	const int obstructionsSenseWidth = 120;
	const int obstructionsSenseHeight = 150;
	const float obstructedThreshold = 0.5f;

	// robot wheen angles
	const float robotWheelAngle1 = -135.0f;
	const float robotWheelAngle2 = -45.0f;
	const float robotWheelAngle3 = 45.0f;
	const float robotWheelAngle4 = 135.0f;

	// distance between two robot wheels diagonally
	const float robotWheelOffset = 0.1167f;

	// how much to substract from observed distance to calculate distance from dribbler
	const float robotDribblerDistance = 0.17f;

	// robot radius
	const float robotRadius = 0.12425f;

	// radius of a wheel
	const float robotWheelRadius = 0.035f;

	// in how many seconds to spin around the dribbler
	//const float robotSpinAroundDribblerPeriod = 2.0f;
	const float robotSpinAroundDribblerPeriod = 1.5f;
	
	// in how big of a radius to spin around the dribbler
	const float robotSpinAroundDribblerRadius = 0.1f;

	// how fast to drive forward while spinning around the dribbler
	const float robotSpinAroundDribblerForwardSpeed = 0.2f;

	// fluid movement steps
	const float robotfluidSpeedStep = 1.5f;
	const float robotfluidOmegaStep = 6.28f;

	// how fast to spin the dribbler
	const int robotDribblerSpeed = 150;
	const int robotDribblerNormalLowerLimit = 34;
	const int robotDribblerNormalUpperLimit = 31;
	const int robotDribblerChipKickLowerLimit = 100;
	const int robotDribblerChipKickUpperLimit = 100;
	const float robotDribblerMoveDuration = 0.4f;
	const int robotDribblerLimitMin = 600;
	const int robotDribblerLimitMax = 2500;
	const float robotDribblerStabilityDelay = 0.2f;

	// coilgun voltage lower then this is considered to be low
	const float robotCoilgunLowVoltageThreshold = 240.0f;
	
	// proportional multiplier for looking at object, multiplied by object angle
	//const float lookAtP = 7.0f;
	const float lookAtP = 5.5f;
	const float lookAtI = 1.0f;
	const float lookAtD = 0.005f;

	// maximum look-at omega is achived is object is at this angle or more
	const float lookAtMaxSpeedAngle = 45.0f;

	// for how many frames must the real wheel speed vary considerably from target speed to be considered stalled
	const int robotWheelStalledThreshold = 60;

	// how long the ball needs to be in the dribbler to be considered stable (seconds)
	const float ballInDribblerThreshold = 0.0f;

	// how long must the ball have not been detected to be considered lost (seconds)
	//const float dribblerBallLostThreshold = 0.3f;
	const float dribblerBallLostThreshold = 0.1f;

	// omega threshold for the robot to be considered not spinning any more
	const float rotationStoppedOmegaThreshold = 0.25f;

	// multiplier to act against robot spinning
	const float rotationCancelMultiplier = 0.3f;

	// multipler to jump robot by certain angle
	const float jumpAngleStopMultiplier = 1.0f;

	// how much should the centerline be inside the goal to kick ball
	// 0.5 means that the camera centerline needs to be in the center 50% of the goal, 0.75 in the 75% so smaller means more accurate but takes longer
	//const float goalKickThreshold = 0.25f;
	const float goalKickThreshold = 0.70f;
	//const int goalKickValidFrames = 4;
	const int goalKickValidFrames = 3;
	//const int goalKickValidFrames = 10;

	// particle filter robot localizer parameters
	const int robotLocalizerParticleCount = 1000;
	const float robotLocalizerForwardNoise = 0.75f;
	const float robotLocalizerTurnNoise = 1.57f; //90deg

	// maximum acceleration/deacceleration the robot should attempt
	const float robotMaxAcceleration = 2.0f;

	// maximum attempted approach speed
	const float robotMaxApproachSpeed = 2.0f;

	// default kick strength in microseconds
	const int robotDefaultKickStrength = 2000;

	// minimum kick interval
	const double minKickInterval = 1.0;

	// maximum time object can be lost and still considered for updating its velocity
	const double velocityUpdateMaxTime = 0.025;

	// drag to apply to a rolling object
	const float rollingDrag = 0.2f;

	// object closer then this are considered to be the same object as observed before
	const float objectIdentityDistanceThreshold = 0.25f;

	// a localized ball will me marked for deletion after this amount of time of not being visible
	const double objectMarkForRemovalThreshold = 5.0;

	// object is purged from localization map if not seen for this amount of time
	const double objectPurgeLifetime = 10.0f;

	// maximum velocity of an object to be considered valid
	const float objectMaxVelocity = 8.0f;

	// how close to the field-of-view must the object be to be considered in view
	const float objectFovCloseEnough = 0.5f;

	// configuration filenames
	const std::string blobberConfigFilename = "../Config/blobber.cfg";
	const std::string frontDistanceLookupFilename = "../Config/distance-front.cfg";
	const std::string rearDistanceLookupFilename = "../Config/distance-rear.cfg";
	const std::string frontAngleLookupFilename = "../Config/angle-front.cfg";
	const std::string rearAngleLookupFilename = "../Config/angle-rear.cfg";
	const std::string undistortMappingFilenameFrontX = "../Config/undistort-mapping-front-x.csv";
	const std::string undistortMappingFilenameFrontY = "../Config/undistort-mapping-front-y.csv";
	const std::string undistortMappingFilenameRearX = "../Config/undistort-mapping-rear-x.csv";
	const std::string undistortMappingFilenameRearY = "../Config/undistort-mapping-rear-y.csv";
	const std::string distortMappingFilenameFrontX = "../Config/distort-mapping-front-x.csv";
	const std::string distortMappingFilenameFrontY = "../Config/distort-mapping-front-y.csv";
	const std::string distortMappingFilenameRearX = "../Config/distort-mapping-rear-x.csv";
	const std::string distortMappingFilenameRearY = "../Config/distort-mapping-rear-y.csv";
	const std::string screenshotsDirectory = "screenshots";

} // namespace Config

enum Side {
	BLUE = 0,
    YELLOW = 1,
	UNKNOWN = 2
};

enum RobotColor {
	YELLOWHIGH = 0,
	BLUEHIGH = 1,
	WHATEVER = 2
};

enum Dir {
    FRONT = 1,
    REAR = 2,
	ANY = 3
};

/*enum Obstruction {
	NONE = 0,
	LEFT = 1,
	RIGHT = 2,
	BOTH = 3,
};*/

enum Decision {
	UNDECIDED = 0,
	YES = 1,
	NO = -1
};

#endif // CONFIG_H
