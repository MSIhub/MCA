#include "pch.h"
#include "UnityServer.h"
#include "Cueing.h"


int main()
{
	DataTheard unity_cue_dt;
	std::thread data_input_thread(UnityServer::GetInputMotionDataFromUnity, std::ref(unity_cue_dt));
	std::thread cue_theard(Cueing::CueingTest, std::ref(unity_cue_dt));
	
	data_input_thread.join();
	cue_theard.join();
	return 0;
}
