#ifndef __SINUS_SOUND__
#define __SINUS_SOUND__

/**
* Synthé sinusoidal.
*/
#include "continuous_sound.h"

#define MPI 3.14159265359

class SinusSound : public ContinuousSound
{
public:
	
	SinusSound() : ContinuousSound()
	{

	}

	void setFreq(float freq, float stepFreqPerPeriod)
	{
		_SinFreq = _NewFreq;
		_NewFreq = freq;
		_StepFreq = stepFreqPerPeriod;
		_Lerp = 0.0f;
	}
private:
	float _SinFreq = 440.f;
	float _NewFreq = 440.f;
	float _CurrentFreq = 440.f;
	float _Lerp = 1.0f;
	float _StepFreq = 5;
	float _Phase = 0.0f;
	double t = 0.f;

protected :
	virtual void init()
	{

	}

	/**
	  * Remplissage du buffer de synthèse, jusqu'au pointeur de lecture
	  */
	virtual float getNextSample()
	{
		t ++;
		float alpha = 3 * pow(_Lerp, 2) - 2 * pow(_Lerp, 3);
		float currentFreq = (1 - alpha)*_SinFreq + alpha*_NewFreq;
			
		if (_NewFreq != _SinFreq
			&& sin((2 * MPI)*((t - 1) / _Frequency)*_SinFreq) < 0.0f 
			&& sin((2 * MPI)*(t / _Frequency)*_SinFreq) > 0.0f) {
			_Phase = ((2 * MPI)*(t / _Frequency)*_CurrentFreq+_Phase);
			_CurrentFreq = currentFreq;
			t = 0.0f;
			if (_Lerp < 1.0f)
				_Lerp += (1 / (_StepFreq*10.0f));
		}

		return sin((2*MPI)*(t/_Frequency)*_CurrentFreq+_Phase)*0.8f;
	}	

};


#endif
