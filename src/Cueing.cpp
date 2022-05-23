#include "pch.h"
#include "Cueing.h"

void Cueing::CueMain(DataThreadUnity& dth, DataThreadSP7& sp7dth)
{
	
	while (true)
	{
		
		std::unique_lock<std::mutex> lock(dth.mtx);
		while (!dth.isDataReceived)
		{
			dth.cond.wait(lock);
		}
		if (dth.isDataReceived)
		{
			//Cueing function goes here
			//----------------------------//
			Cueing::ScaleInputData(&sp7dth.motion_data[0], dth);

			printf("\n");
			for (int i = 0; i < BUFLEN_UNITY / 4; i++)
			{
				printf("%f, ", sp7dth.motion_data[i]);
			}

			//----------------------------//
			dth.isDataReceived = false;
		}
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::nanoseconds(10));
	}
}

void Cueing::ScaleInputData(float* temp_motion_data, DataThreadUnity& dth)
{
	//auto start_time = std::chrono::steady_clock::now();

	std::map<std::string, float> paramMap;
	Cueing::ExtractParameterFromFile(paramMap);
	
	/*auto end_time = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
	std::cout << std::setprecision(15) << "Duration:  " << duration << " nanoseconds" << std::endl;*/
	//Cueing::PrintParameterFileData(paramMap);
	temp_motion_data[0] = dth.motion_data[0]; //timestamp
	for (int i = 1; i < BUFLEN_UNITY / 4; i++)
	{
		temp_motion_data[i] = paramMap["k_ax"] * dth.motion_data[i];
	}
}


void Cueing::ExtractParameterFromFile(std::map<std::string, float> &paramMap)
{
	std::string filename = "src/param.yaml";
	std::ifstream param;


	param.open(filename);

	if (!param.is_open())
	{
		std::cout << "file" << filename << "not open" << std::endl;
		return;
	}

	while (param)
	{
		std::string key;
		float value;
		std::getline(param, key, ':');
		param >> value;
		param.get(); // catch empty line
		if (!param)
		{
			return;
		}
		paramMap[key] = value; //paramMap.insert(std::pair<std::string, float>(key, value));
	}
	param.close();
	return ;
}

void Cueing::PrintParameterFileData(std::map<std::string, float>& paramMap)
{
	// printing map gquiz1
	std::map<std::string, float>::iterator itr;
	for (itr = paramMap.begin(); itr != paramMap.end(); ++itr)
	{
		std::cout << itr->first << ": " << itr->second << std::endl;
	}
}

//printf("\n");
//for (int i = 0; i < BUFLEN / 4; i++)
//{
//	printf("%f, ", dth.motion_data[i]);
//}
