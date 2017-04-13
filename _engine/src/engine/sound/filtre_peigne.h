#ifndef __FILTER_PEIGNE__
#define __FILTER_PEIGNE__

/**
*Filtre abstrait
*/
#include <queue>
#include "filter.h"
#include "filter_lp.h"


class FilterPeigne : public FilterAudio
{
public:

	void setReverb(float reverb)
	{
		_Reverb = reverb;
	}

	FilterPeigne() : FilterAudio()
	{
		FilterPeigne(0.5, 20000);
	}

	FilterPeigne(float reverb, int distance, bool allpass = false) : FilterAudio()
	{
		_Reverb = reverb;
		_Distance = distance;
		buffer = new float[_Distance];
		all_pass = allpass;
	}

	void init()
	{

	}

	virtual float doFilter(float ech)
	{
		if (ptr_buffer >= _Distance - 1)
			active = true;
		buffer[ptr_buffer] = ech;

		float output = ech;
		if (active) {
			output += buffer[(ptr_buffer+1)%_Distance] * _Reverb;
			if(all_pass)
				output += -_Reverb*ech;
		}
		ptr_buffer = (ptr_buffer + 1) % _Distance;
		return output;
	}

	void setAllpass(bool v) {
		all_pass = v;
	}

private:
	float *buffer;
	int ptr_buffer = 0;
	bool active = false;
	bool all_pass = false;
	float _Reverb = 0.5f;
	int _Distance = 100000;

};


#endif
