#ifndef __FILTER_REVERB__
#define __FILTER_REVERB__

/**
*Filtre abstrait
*/
#include <vector>
#include "filter.h"
#include "filtre_peigne.h"
#include "filter_lp.h"

class FilterReverb : public FilterAudio
{
public:

	void setReverb(float reverb)
	{
		_Reverb = reverb;
	}

	FilterReverb() : FilterAudio()
	{
		
	}

	~FilterReverb() {
		for (int i = 0; i < filtres.size(); i++)
			delete filtres[i];
	}

	void init()
	{
		filtres.push_back(new FilterPeigne(0.7,30000));
		filtres.push_back(new FilterPeigne(0.5, 50000));
		filtres.push_back(new FilterPeigne(0.3, 70000));
		filtres.push_back(new FilterPeigne(0.1, 110000));
		//Passe tout
		filtres.push_back(new FilterPeigne(0.4, 13000, true));
		filtres.push_back(new FilterPeigne(0.2, 15000, true));
	}

	virtual float doFilter(float ech)
	{
		float output = 0;
		for (int i = 0; i < 4; i++)
			output += filtres[i]->filter(ech);
		output = filtres[4]->filter(output);
		output = filtres[5]->filter(output);
		output = ech + output*_Reverb + ech*-_Reverb;
		return output;
	}	

private :
	std::vector<FilterAudio*> filtres;
	float _Reverb;

};


#endif
