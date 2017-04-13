#ifndef __NOISE_SOUND__
#define __NOISE_SOUND__

/**
* Bruit de variabilité ajustable
*/
#include "continuous_sound.h"
#include <time.h>

class NoiseSound : public ContinuousSound
{
public:
	
	NoiseSound() : ContinuousSound()
	{		
		value = new float[8];
		for (int i = 0; i < 8; i++) {
			value[i] = 0;
		}
	}

	void setFreq(float freq)
	{
		_FreqNoise = freq;
	}

private:
	float _FreqNoise = 1.0f;
	int _increment = 0;
	float *value;
	
protected :

	/**
	  * Remplissage du buffer de synthèse, jusqu'au pointeur de lecture
	  */
	virtual float getNextSample()
	{	
		_increment++;
		for (int i = 0; i < 8; i++) {
			if (_increment%(int)(_FreqNoise*(1<<i))==0) {
				value[i] = (randf() * 2 - 1)*0.5f;
			}
		}
		float final_value=0;
		for (int i = 0; i < 8; i++)
			final_value += value[i];
		return final_value*0.8f;
	}	

};

#endif
