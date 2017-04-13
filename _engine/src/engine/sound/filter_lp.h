#ifndef __FILTER_LP__
#define __FILTER_LP__

/**
*Filtre abstrait
*/
#include "filter.h"


class FilterLP : public FilterAudio
{
public:

	void setAlpha(float alpha)
	{
		_Alpha = alpha;
	}

	FilterLP() : FilterAudio()
	{
		
	}

	virtual float doFilter(float ech)
	{
		_Output = _Output + _Alpha*(ech - _Output);
		return _Output;
	}	

private :
	float _Output = 0;
	float _Alpha = 1.0f;
};


#endif
