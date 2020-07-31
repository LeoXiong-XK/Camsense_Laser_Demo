#pragma once
#ifdef WIN32
#include <windows.h> 
#define EXPORTS __declspec(dllexport)
#else
#include <stdlib.h>
#include <sys/time.h>
#define EXPORTS 
#endif

/*!  \class     TimeOut
     \brief     This class can manage a timer which is used as a timeout.
   */
// Class TimeOut



class EXPORTS TimeOut
{
private:

#ifdef WIN32
	LARGE_INTEGER  freq_;
	LARGE_INTEGER  begin_time_;
	long long elapsed_;
#else
	 long long begin_time_;
	 long long elapsed_;
#endif
public:

    // Constructor
	TimeOut():elapsed_(0){
#ifdef WIN32
		QueryPerformanceFrequency(&freq_);
#endif
		InitTimer();
	}

    // Init the timer
    void  InitTimer();
    // Return the elapsed time since initialization
    double   ElapsedTime_ms();
	double   Duation_ms();
	long long beginTime();

};