#include <iostream>
#include "UnityServer.h"

int main()
{
	std::cout << "In main" << std::endl;
	UnityServer::GetInputMotionDataFromUnity();
	return 0;
}