#include "Cueing.h"


void Cueing::CueingTest(float* MotionData, std::atomic<bool>& isDataRecv, std::mutex& m_mu, std::condition_variable& m_cond)
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(m_mu);
		while (!isDataRecv)
		{
			m_cond.wait(lock);
		}
		if (isDataRecv)
		{
			printf("\n");
			for (int i = 0; i < BUFLEN / 4; i++)
			{
				printf("%f, ", MotionData[i]);
			}
		}
		isDataRecv = false;
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}