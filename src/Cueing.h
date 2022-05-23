/*
 * Cueing.hpp
 *
 */
#pragma once
#ifndef CUEING_HPP_
#define CUEING_HPP_


namespace Cueing
{
	//functions
	void CueMain(DataThreadUnity&, DataThreadSP7&);
	void ScaleInputData(float* , DataThreadUnity& dth);
	void PrintParameterFileData(std::map<std::string, float>& paramMap);
	void ExtractParameterFromFile(std::map<std::string, float>&);
}
#endif