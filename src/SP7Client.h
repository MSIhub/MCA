#pragma once
#ifndef SP7CLIENT_HPP_
#define SP7CLIENT_HPP_


namespace SP7Client
{
	//Const variables
	const double Z_ZERO = 0.401;
	

	//Structures
	struct Pose
	{
		double x, y, z, roll, pitch, yaw;
	};

	struct Velocity
	{
		double vx, vy, vz, vroll, vpitch, vyaw;
	};


	//Enumeration
	enum class CtrlCommand
	{
		ALIVE = 0,
		CONNECT,
		DISCONNECT,
		START,
		STOP,
		ZERO,
		JOINT_TARGETS,
		EE_TARGETS,
		RESET = 0xeffe,
		HALT = 0xefff,
		MOTION_DATA = 0xfffe,
		CMD_REPLY = 0xffff
	};

	//Function declaration
	void SendDataToSP7(DataThreadSP7& );
	void ExtractMotionData(SP7Client::Pose&, SP7Client::Velocity&, DataThreadSP7&);
	void CreateSocketAndSetupAddress(int&, sockaddr_in&);
	void InitialiseWinsock(WSADATA&);
	void SendCommand(SP7Client::CtrlCommand, char*, int&, int&, sockaddr_in&);
	void PackSP7NetworkDataStream(SP7Client::CtrlCommand, const char*, char*);
	void PackEETargets(SP7Client::Pose, SP7Client::Velocity, char*);
}
#endif
