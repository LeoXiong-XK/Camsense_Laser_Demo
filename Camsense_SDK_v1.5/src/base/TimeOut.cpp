#include "TimeOut.h"
// ******************************************
//  Class TimeOut
// ******************************************

/*!
    \brief      Constructor of the class TimeOut.
*/
// Constructor
//using namespace std::chrono;

/*!
    \brief      Initialise the timer. It writes the current time of the day in the structure PreviousTime.
*/
//Initialize the timer
void TimeOut::InitTimer()
{
#ifdef WIN32
	QueryPerformanceCounter(&begin_time_);  
#else
	struct timeval begintime;
	gettimeofday(&begintime,NULL);
	begin_time_ = (begintime.tv_sec*1000000+begintime.tv_usec);
#endif
	//start = high_resolution_clock::now();
	//duration = timepoint.time_since_epoch();
}



//Return the elapsed time since initialization
double  TimeOut::ElapsedTime_ms()
{
#ifdef WIN32
	LARGE_INTEGER end_time;
    QueryPerformanceCounter(&end_time);
    elapsed_ = (end_time.QuadPart - begin_time_.QuadPart) * 1000000 / freq_.QuadPart;
#else
	struct timeval endtime;
	gettimeofday(&endtime,NULL);
	long long end_time_ = (endtime.tv_sec*1000000+endtime.tv_usec);
	elapsed_ = end_time_ - begin_time_;
#endif
	//double costTime= (StopTime-PreviousTime)*1000.0/dfFreq;
	
	//std::chrono::high_resolution_clock::time_point end = high_resolution_clock::now();
	//int64_t costTime = duration<microseconds>(end- start).count();
	InitTimer();
    //return  costTime  //ms
	return elapsed_ /1000.0;
}

long long TimeOut::beginTime()
{
#ifdef WIN32
	return begin_time_.QuadPart * 1000000 / freq_.QuadPart;
#endif
#ifdef ANDROID
	return begin_time_;
#endif
}


double  TimeOut::Duation_ms()
{
#ifdef WIN32
	LARGE_INTEGER end_time;
    QueryPerformanceCounter(&end_time);
    elapsed_ = (end_time.QuadPart - begin_time_.QuadPart) * 1000000 / freq_.QuadPart;
#else
	struct timeval endtime;
	gettimeofday(&endtime,NULL);
	long long end_time_ = (endtime.tv_sec*1000000+endtime.tv_usec);
	elapsed_ = end_time_ - begin_time_;
#endif
	//double costTime= (StopTime-PreviousTime)*1000.0/dfFreq;
	
	//std::chrono::high_resolution_clock::time_point end = high_resolution_clock::now();
	//int64_t costTime = duration<microseconds>(end- start).count();
	//InitTimer();
    //return  costTime  //ms
	return elapsed_ /1000.0;
}