#include "pch.h"
#include "UnityServer.h"
#include "Cueing.h"
#include "SP7Client.h"




int main()
{
	DataThreadUnity unity_cue_dt;
	DataThreadSP7 cue_sp7_dt;

	std::thread unityserver_thread(UnityServer::GetInputMotionDataFromUnity, std::ref(unity_cue_dt));
	std::thread cueing_theard(Cueing::CueMain, std::ref(unity_cue_dt), std::ref(cue_sp7_dt));
	std::thread sp7client_theard(SP7Client::SendDataToSP7, std::ref(cue_sp7_dt));
	
	unityserver_thread.join();
	cueing_theard.join();
	sp7client_theard.join();
	return 0;
}
