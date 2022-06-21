#include "pch.h"
#include "SP7Client.h"

uint32_t pid = 0;

void SP7Client::SendDataToSP7(DataThreadSP7& sp7dth)
{
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	WSADATA wsa;
	//Initialise winsock
	SP7Client::InitialiseWinsock(wsa);
	SP7Client::CreateSocketAndSetupAddress(s, si_other);

	//start communication
	while (1)
	{
		char dataArray[BUFLEN_SP7];
		SP7Client::Pose x;
		SP7Client::Velocity xdot;
		SP7Client::ExtractMotionData(x, xdot, sp7dth);
		SP7Client::ExtractMotionData(x, xdot, sp7dth);
		SP7Client::PackEETargets(x, xdot, dataArray);
		SP7Client::SendCommand(SP7Client::CtrlCommand::EE_TARGETS, dataArray, s, slen, si_other);
	}

	closesocket(s);
	WSACleanup();
}


void SP7Client::InitialiseWinsock(WSADATA& wsa)
{
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");
}

void SP7Client::CreateSocketAndSetupAddress(int& s, sockaddr_in& si_other)
{
	//create socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//setup address structure
	memset((char*)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT_SP7);
	inet_pton(AF_INET, SERVER_SP7, &si_other.sin_addr.S_un.S_addr);
}



void SP7Client::ExtractMotionData(SP7Client::Pose& x, SP7Client::Velocity& xdot, DataThreadSP7& sp7dth)
{
	x.x = sp7dth.motion_data[1];
	x.y = sp7dth.motion_data[2];
	x.z = sp7dth.motion_data[3];
	x.roll = sp7dth.motion_data[4];
	x.pitch = sp7dth.motion_data[5];
	x.yaw = sp7dth.motion_data[6];
	xdot.vx = sp7dth.motion_data[7];
	xdot.vy = sp7dth.motion_data[8];
	xdot.vz = sp7dth.motion_data[9];
	xdot.vroll = sp7dth.motion_data[10];
	xdot.vpitch = sp7dth.motion_data[11];
	xdot.vyaw = sp7dth.motion_data[12];
}


void SP7Client::PackEETargets(SP7Client::Pose x, SP7Client::Velocity xdot, char* data)
{
	data = {};
	/*strcpy(data, (char*)(uint32_t)(x.x * (1 << 16)));
	strcat(data, (char*)(uint32_t)(x.y * (1 << 16)));
	strcat(data, (char*)(uint32_t)(x.z * (1 << 16)));
	strcat(data, (char*)(uint32_t)(x.roll * (1 << 16)));
	strcat(data, (char*)(uint32_t)(x.pitch * (1 << 16)));
	strcat(data, (char*)(uint32_t)(x.yaw * (1 << 16)));
	strcat(data, (char*)(uint32_t)(xdot.vx * (1 << 16)));
	strcat(data, (char*)(uint32_t)(xdot.vy * (1 << 16)));
	strcat(data, (char*)(uint32_t)(xdot.vz * (1 << 16)));
	strcat(data, (char*)(uint32_t)(xdot.vroll * (1 << 16)));
	strcat(data, (char*)(uint32_t)(xdot.vpitch * (1 << 16)));
	strcat(data, (char*)(uint32_t)(xdot.vyaw * (1 << 16)));*/
}

void SP7Client::SendCommand(SP7Client::CtrlCommand cmd, char* dataArray, int& s, int& slen, sockaddr_in& si_other)
{
	char buf[BUFLEN_SP7] = {};
	PackSP7NetworkDataStream(cmd, dataArray, buf);
	int buf_length = strlen(buf);
	auto soc_send = sendto(s, buf, buf_length, 0, (struct sockaddr*)&si_other, slen);

	if (soc_send == SOCKET_ERROR)
	{
		printf("sendto() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//clear the buffer by filling null, it might have previously received data
	memset(buf, '\0', BUFLEN_SP7);
}

void SP7Client::PackSP7NetworkDataStream(SP7Client::CtrlCommand cmd, const char* dataArray, char* buf)
{
	//Package data
	uint32_t id = pid++;
	char buf1[] = { (char)MAGIC , (char)id, (char)(uint16_t)cmd };
	char* byteDataArray = (char*)dataArray;
	int buf1_len = strlen(buf1);

	for (int i = 0; i < buf1_len; i++)
	{
		//memcpy(buf, buf1);
		buf[i] = buf1[i];
	}

	for (uint32_t j = 0; j < strlen(byteDataArray); j++)
	{
		//std::strcat(buf, byteDataArray);
		buf[buf1_len + j] = byteDataArray[j];
	}

	int delta = (int)BUFLEN_SP7 - (strlen(dataArray) + 10);

	for (int k = delta; k < BUFLEN_SP7 - 1; k++)
	{
		buf[k] = '0';
	}
	buf[BUFLEN_SP7 - 1] = '\0';
}



