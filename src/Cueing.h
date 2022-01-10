/*
 * UnityServer.hpp
 *
 */
#pragma once
#ifndef CUEING_HPP_
#define CUEING_HPP_

#include <iostream>
#include <thread>
#include <condition_variable>

namespace Cueing
{
	#define BUFLEN 52	//Max length of buffer

	//functions
	void CueingTest(float*, std::atomic<bool>&, std::mutex&, std::condition_variable&);
}
#endif