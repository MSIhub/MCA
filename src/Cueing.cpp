#include "pch.h"
#include "Cueing.h"

void Cueing::CueMain(DataThreadUnity& dth, DataThreadSP7& sp7dth)
{
	
	bool log_data = true;
	double high_pass_kernel[KERNEL_LENGTH];
	double low_pass_kernel[KERNEL_LENGTH];
	bool init_run = true;

	initial_time = 0.0;
	//Extract Parameters from File
	std::map<std::string, float> paramMap;
	Cueing::ExtractParameterFromFile(paramMap);
	Cueing::PrintParameterFileData(paramMap);

	//Kernel calculation beforehand
	calc_kernel_high_pass(KERNEL_LENGTH, paramMap["hp_ax"], &high_pass_kernel[0], log_data);
	calc_kernel_low_pass(KERNEL_LENGTH, paramMap["lp_ax"], &low_pass_kernel[0], log_data);


	// CueData
	CueData* c_ax = new CueData{};
	CueData* c_ay = new CueData{};
	CueData* c_az = new CueData{};
	CueData* c_tcx = new CueData{};
	CueData* c_tcy = new CueData{};
	CueDataVel* c_vroll = new CueDataVel{};
	CueDataVel* c_vpitch = new CueDataVel{};
	CueDataVel* c_vyaw = new CueDataVel{};
	
	#pragma region Log data
	//logging 
	std::string delimiter = "\t"; //tab limited text file with 8 point precision
	std::string log_filename = "log_data_";
	log_filename += std::to_string(std::time(nullptr));
	log_filename += ".dat";
	std::fstream log_fptr;
	log_fptr.open(log_filename, std::fstream::out);
	#pragma endregion

	while (true)
	{		
		std::unique_lock<std::mutex> lock(dth.mtx);
		while (!dth.isDataReceived)
		{
			dth.cond.wait(lock);
		}
		if (dth.isDataReceived)
		{
			//Extract sent data 
			float t = dth.motion_data[0];
			float ax = dth.motion_data[1];
			float ay = dth.motion_data[2];
			float az = dth.motion_data[3];
			float vroll = dth.motion_data[4];
			float vpitch = dth.motion_data[5];
			float vyaw = dth.motion_data[6];
			
			if (init_run)
			{
				initial_time = double(t);
				init_run = false;
			}
			//------------- Cueing function -------------------------//
			SP7Pose* pos = new SP7Pose{ 0,0,0,0,0,0 };// initialization to avoid sending garbage value
			SP7Vel* vel = new SP7Vel{ 0,0,0,0,0,0 };
			double timestamp = 0.0;

			//Translational channel
			cue_translational_channel(ax, t, &high_pass_kernel[0], paramMap["k_ax"], 0, c_ax, &(pos->x), &(vel->vx), &timestamp);
			cue_translational_channel(ay, t, &high_pass_kernel[0], paramMap["k_ay"], 1, c_ay, &(pos->y), &(vel->vy), &timestamp);
			cue_translational_channel(az, t, &high_pass_kernel[0], paramMap["k_az"], 2, c_az, &(pos->z), &(vel->vz), &timestamp);

			// Tilt coordination channel
			double tilt_x = 0.0;
			cue_tilt_coordination_channel(ax, t, &low_pass_kernel[0], paramMap["k_ax"], 0, c_tcx, &tilt_x, &timestamp);
			double tilt_y = 0.0;
			cue_tilt_coordination_channel(ay, t, &low_pass_kernel[0], paramMap["k_ay"], 1, c_tcy, &tilt_y, &timestamp);

			// Rotational channel
			cue_rotational_channel(vroll, t, &high_pass_kernel[0], paramMap["k_vroll"], 3, c_vroll, &(pos->roll), &(vel->vroll), &timestamp);
			pos->roll += tilt_x; // adding tilt effect
			cue_rotational_channel(vpitch,t, &high_pass_kernel[0], paramMap["k_vpitch"], 4, c_vpitch, &(pos->pitch), &(vel->vpitch), &timestamp);
			pos->pitch += tilt_y;// adding tilt effect
			cue_rotational_channel(vyaw, t, &high_pass_kernel[0], paramMap["k_vyaw"], 5, c_vyaw, &(pos->yaw), &(vel->vyaw), &timestamp);

			//Pack SP7 data
			sp7dth.motion_data[0] = (float)t;
			sp7dth.motion_data[1] = (float)pos->x;
			sp7dth.motion_data[2] = (float)pos->y;
			sp7dth.motion_data[3] = (float)pos->z;
			sp7dth.motion_data[4] = (float)pos->roll;
			sp7dth.motion_data[5] = (float)pos->pitch;
			sp7dth.motion_data[6] = (float)pos->yaw;
			sp7dth.motion_data[7] = (float)vel->vx;
			sp7dth.motion_data[8] = (float)vel->vy;
			sp7dth.motion_data[9] = (float)vel->vz;
			sp7dth.motion_data[10] = (float)vel->vroll;
			sp7dth.motion_data[11] = (float)vel->vpitch;
			sp7dth.motion_data[12] = (float)vel->vyaw;
			

			#pragma region log data
			if (log_data)
			{
				//Preparing the data stream
				std::stringstream ss;
				ss.precision(8);// max to micro meter
				ss << std::fixed <<
					timestamp << delimiter <<
					pos->x << delimiter <<
					pos->y << delimiter <<
					pos->z << delimiter <<
					pos->roll << delimiter <<
					pos->pitch << delimiter <<
					pos->yaw << delimiter <<
					vel->vx << delimiter <<
					vel->vy << delimiter <<
					vel->vz << delimiter <<
					vel->vroll << delimiter <<
					vel->vpitch << delimiter <<
					vel->vyaw;
			log_fptr.open(log_filename, std::fstream::app);
				if (log_fptr.is_open())
				{
					log_fptr << "\n";
					log_fptr << ss.str();
					log_fptr.close();

				}

			}
			#pragma endregion

			//----------------------------//
			delete pos, vel;
			dth.isDataReceived = false;
		}		
		lock.unlock();
		std::this_thread::sleep_for(std::chrono::nanoseconds(10));
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

void Cueing::calc_kernel_high_pass(int filter_length, double cut_off, double* fltr_kernel_arr_, bool isLogging)
{
	double threshold = 0.0;
	//Calculate the first low-pass filter kernel 
	for (int i = 0; i < filter_length; i++)
	{
		threshold = (i - (double)(filter_length / 2));
		if (threshold == 0)
		{
			fltr_kernel_arr_[i] = 2 * M_PI * cut_off;
		}
		if (threshold != 0)
		{
			fltr_kernel_arr_[i] = sin(2 * M_PI * cut_off * threshold) / threshold;
			fltr_kernel_arr_[i] *= (0.42 - 0.5 * cos(2 * M_PI * i / filter_length) + 0.08 * cos(4 * M_PI * i / filter_length));
		}
	}

	//Change low-pass filter to a high-pass filter using spectral inversion
	for (int i = 0; i < filter_length; i++)
	{
		fltr_kernel_arr_[i] = -fltr_kernel_arr_[i];
	}
	fltr_kernel_arr_[filter_length / 2] += 1;

	if (isLogging)
	{
		std::string log_filename = "high_pass_kernel_";
		log_filename += std::to_string(std::time(nullptr));
		log_filename += ".dat";

		std::fstream output_sig_rt_kernel_fptr;
		output_sig_rt_kernel_fptr.open("log/" + log_filename, std::fstream::in | std::fstream::out | std::fstream::app);

		if (output_sig_rt_kernel_fptr.is_open())
		{
			for (int i = 0; i < filter_length; i++)
			{
				output_sig_rt_kernel_fptr << "\n";
				output_sig_rt_kernel_fptr << fltr_kernel_arr_[i];//column vector
			}
			output_sig_rt_kernel_fptr.close();
		}

	}

}

void Cueing::calc_kernel_low_pass(int filter_length, double cut_off, double* fltr_kernel_arr_, bool isLogging)
{
	double threshold = 0.0;
	//Calculate the first low-pass filter kernel 
	for (int i = 0; i < filter_length; i++)
	{
		threshold = (i - (double)(filter_length / 2));
		if (threshold == 0)
		{
			fltr_kernel_arr_[i] = 2 * M_PI * cut_off;
		}
		if (threshold != 0)
		{
			fltr_kernel_arr_[i] = sin(2 * M_PI * cut_off * threshold) / threshold;
			fltr_kernel_arr_[i] *= (0.42 - 0.5 * cos(2 * M_PI * i / filter_length) + 0.08 * cos(4 * M_PI * i / filter_length));
		}
	}

	if (isLogging)
	{
		std::string log_filename = "low_pass_kernel_";
		log_filename += std::to_string(std::time(nullptr));
		log_filename += ".dat";

		std::fstream output_sig_rt_kernel_fptr;
		output_sig_rt_kernel_fptr.open("log" + log_filename, std::fstream::in | std::fstream::out | std::fstream::app);

		if (output_sig_rt_kernel_fptr.is_open())
		{
			for (int i = 0; i < filter_length; i++)
			{
				output_sig_rt_kernel_fptr << "\n";
				output_sig_rt_kernel_fptr << fltr_kernel_arr_[i];//column vector
			}
			output_sig_rt_kernel_fptr.close();
		}

	}

}

void Cueing::cue_translational_channel(double sig_acc_input, double sig_time, double* kernel, double scale_factor, int data_index, Cueing::CueData* cue_data, double* out_pos_, double* out_vel_, double* out_t_)
{
	/* filter => scale => integrate => integrate => update prev*/
	//Convolving with designed kernel will filter the signal
	//Offestting should not affect the integration value as the value will be modified  based on that

	cue_data->t = sig_time - initial_time;
	cue_data->acc_fltrd = Convolve_rt(&kernel[0], KERNEL_LENGTH, sig_acc_input, cue_data->Input_Buff, &(cue_data->circ_buff_idx));
	cue_data->acc_fltrd_scaled = scale_factor * cue_data->acc_fltrd; //scale 
	cue_data->velocity = Intergration_Trapezoidal(cue_data->acc_fltrd_scaled, cue_data->acc_fltrd_scaled_prev, cue_data->velocity_prev, cue_data->t_prev, cue_data->t); //Integration
	cue_data->position = Intergration_Trapezoidal(cue_data->velocity, cue_data->velocity_prev, cue_data->position_prev, cue_data->t_prev, cue_data->t);

	//Updating the output values
	*(out_pos_) = cue_data->position + SP7_ZERO_POSE[data_index];//Offseting with respect to zeropose of SP7
	*(out_vel_) = cue_data->velocity;
	*(out_t_) = cue_data->t;
	//Updating the previous to current
	cue_data->t_prev = cue_data->t;
	cue_data->acc_fltrd_scaled_prev = cue_data->acc_fltrd_scaled;
	cue_data->velocity_prev = cue_data->velocity;
	cue_data->position_prev = cue_data->position;
}

void Cueing::cue_rotational_channel(double sig_vel_input, double sig_time, double* kernel, double scale_factor, int data_index, Cueing::CueDataVel* cue_data, double* out_pos_, double* out_vel_, double* out_t_)
{
	/* filter => scale => integrate => integrate => update prev*/
	//Convolving with designed kernel will filter the signal
	//Offestting should not affect the integration value as the value will be modified  based on that

	cue_data->t = sig_time - initial_time;
	cue_data->velocity_fltr = Convolve_rt(&kernel[0], KERNEL_LENGTH, sig_vel_input, cue_data->Input_Buff, &(cue_data->circ_buff_idx));
	cue_data->velocity_fltr_scaled = scale_factor * cue_data->velocity_fltr; //scale 
	cue_data->position = Intergration_Trapezoidal(cue_data->velocity_fltr_scaled, cue_data->velocity_fltr_scaled_prev, cue_data->position_prev, cue_data->t_prev, cue_data->t);

	//Updating the output values
	*(out_pos_) = cue_data->position + SP7_ZERO_POSE[data_index];//Offseting with respect to zeropose of SP7
	*(out_vel_) = cue_data->velocity_fltr_scaled;
	*(out_t_) = cue_data->t;
	//Updating the previous to current
	cue_data->t_prev = cue_data->t;
	cue_data->velocity_fltr_scaled_prev = cue_data->velocity_fltr_scaled;
	cue_data->position_prev = cue_data->position;
}

void Cueing::cue_tilt_coordination_channel(double sig_acc_input, double sig_time, double* kernel, double scale_factor, int data_index, Cueing::CueData* cue_data, double* out_ang_, double* out_t_)
{
	// LPF => Tilt cooridnation =>Integration => rate limit [x and y axis only -> not the up axis] 
	/*
	Max tilt angle	      = 5 deg
	Max tilt rate		  = 5 deg/s
	Max tilt acceleration = 8 deg/s^2
	Ref - 10.1177/0037549716675955*/

	double rate_limit_factor = 1;
	double g = 9.81;
	cue_data->t = sig_time - initial_time;
	cue_data->acc_fltrd = Cueing::Convolve_rt(&kernel[0], KERNEL_LENGTH, sig_acc_input, cue_data->Input_Buff, &(cue_data->circ_buff_idx));
	cue_data->acc_fltrd_scaled = scale_factor * cue_data->acc_fltrd; //scale 
	// Tilt scaling factor
	double K = 5;//form factor [3-6]
	double theta_max = 5 * (M_PI / 180);
	double Acc_max = g * theta_max;
	double y_tilt_ref = Acc_max * tanh(cue_data->acc_fltrd_scaled / K * Acc_max);
	// Tilt coordination --> Linerzation [Reid and Nahon]
	cue_data->velocity = y_tilt_ref / g;
	if (data_index == 0)
	{
		cue_data->velocity = -cue_data->velocity;
	}
	// Integration 
	cue_data->position = Cueing::Intergration_Trapezoidal(cue_data->velocity, cue_data->velocity_prev, cue_data->position_prev, cue_data->t_prev, cue_data->t);
	//Rate Limit [1-5 deg]
	cue_data->position = rate_limit_factor * cue_data->position;

	//Updating the output values
	*(out_ang_) = cue_data->position;
	*(out_t_) = cue_data->t;
	//Updating the previous to current
	cue_data->t_prev = cue_data->t;
	cue_data->acc_fltrd_scaled_prev = cue_data->acc_fltrd_scaled;
	cue_data->velocity_prev = cue_data->velocity;
	cue_data->position_prev = cue_data->position;
}


#pragma region Helper functions RT

double Cueing::Convolve_rt(double* h, int h_size, double x_in, double* x, int* circ_index)
{
	/*
	Real time convolution for one input signaland filter kernel
	Covolution with MAC(multiply and accumulate) and circultion buffering (shifting)
	executes only when run first and then treats as a global variable
	*/
	//circ_index = h_size - 1;

	x[*(circ_index)] = x_in;

	//Covolution
	double y_out = 0.0;
	for (int k = 0; k < h_size; k++)
		y_out += h[k] * x[(k + *(circ_index)) % h_size]; //MAC

	//Signal shift circularly through  array x in time-reversed order
	*(circ_index) += h_size - 1;
	*(circ_index) %= h_size;
	return y_out;
}

double Cueing::Intergration_Trapezoidal(double input_curr, double input_prev, double output_prev, double t_prev, double t_curr)
{
	if (input_curr == 0 && input_prev == 0)
		return 0.0;
	return output_prev + ((t_curr - t_prev) * ((input_curr + input_prev) / 2)); // Trapedzoidal intergral
}

#pragma endregion

//void Cueing::ScaleInputData(float* temp_motion_data, DataThreadUnity& dth)
//{
//	//auto start_time = std::chrono::steady_clock::now();
//
//
//	/*auto end_time = std::chrono::steady_clock::now();
//	auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
//	std::cout << std::setprecision(15) << "Duration:  " << duration << " nanoseconds" << std::endl;*/
//	//Cueing::PrintParameterFileData(paramMap);
//	temp_motion_data[0] = dth.motion_data[0]; //timestamp
//	for (int i = 1; i < BUFLEN_UNITY / 4; i++)
//	{
//		temp_motion_data[i] = paramMap["k_ax"] * dth.motion_data[i];
//	}
//}
