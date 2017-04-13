#ifndef __GRANULAR_SOUND__
#define __GRANULAR_SOUND__

/**
* Synthé granulaire.
* Lui charger un fichier de base ou il pioche les grains
* utiliser setGrainParam pour modifier les params d'extraction et d'ajout des grains
*/

#include "continuous_sound.h"

#define GRAIN_SYNTHE_BUFFER_LENGTH 1.0f //On prend 1s de buffer en memoire

/**
  * Utilise plusieurs buffers :
  * un buffer qui contient le fichier de base
  * un buffer dans lequel on fait la synthèse : on y ajoute les grains les un a la suite des autres
  *   en tenant compte de params de synthèse. C'est un buffer tournant.
  * un buffer temporaire qui permet de créer
  */
class SoundGrain : public ContinuousSound
{
private:

	//Datas du fichier de base
	float _DureeBase; ///< Duree du buffer de base
	void * _BaseBuffer; ///< Buffer qui contient les échantillons du fichier de base
	uint8 * _PtRdFinBaseBuffer; ///< Pointeur sur la fin du buffer de base
	ALenum _Format; ///< Format du fichier de base (enum OpenAL comme AL_FORMAT_MONO16)
	ALsizei _Size; ///< Taille du fichier de base en octets
	ALfloat _Frequency; ///< Frequence d'échantillonage du fichier de base (44100 par exemple)
	int _NbPistes; ///< Nombre de pistes du fichier de base (1 mono, 2 stéréo)
	int _NbBits; ///< Nombre de bits par échantillon (16 ou 8, surtout utile pour affichage)
	int _TailleEchantillon; ///< Taille d'un échantillon en octets (plus utile pour les calculs)

	//Buffer pour faire la synthèse (buffer tournant)
	//Nécessaire car overlap des grains 
	float _DureeBufferSyntheGrain; ///<Duree du buffer de synthèse
	void * _DatasSyntheGrain; ///< Pointeur sur le début du buffer de synthèse
	sint16 * _PtFinBufferSyntheGrain; ///< Pointeur sur la fin du buffer de synthe
	int _TailleBufferSyntheGrain; ///< Taille du buffer de synthèse
	sint16 *_PtRdSyntheGrain; ///< Pointeur de lecture du buffer de synthese
	sint16 *_PtWrSyntheGrain; ///< Pointeur d'ecriture dans le buffer de synthe
	
	//Paramètres de la generation de grains
	float _PosGrainf; ///< Position de prise du grain entre 0 et 1
	float _DureeGrain; ///< Durée du grain en secondes 
	int _TailleGrain; ///< Taille du grain en octets
	int _PosGraini; ///< Position du grain en octets
	int _RandomWidth; //Taille du random quand on chope un grain
	int _SizeOverlap; //Taille d'overlapp entre les grains
	float _CrossFade; //Si on utilise un crossfade linéaire ou equalpower (signal corelé ou pas)

	//Pour la lecture des grains. 
	sint16 * _PtRdGrain; ///< Pointeur de lecture du grain courant
	sint16 * _PtRdFinGrain; ///< Pointeur de fin du grain courant
	int _TailleLastGrain; ///< Taille du grain courant

	int writeSize;

public :
	void loadBaseFile(char * filename)
	{
		string mess_fichier = "Chargement du fichier :";
		mess_fichier+=filename;
		Log::log(Log::ENGINE_INFO, mess_fichier.c_str());

		

		_BaseBuffer = alutLoadMemoryFromFile(filename, &_Format, &_Size, &_Frequency);
		if (_BaseBuffer == NULL) {
			Log::log(Log::ENGINE_ERROR, alutGetErrorString(alutGetError()));
		}
		else {
			//Chargement des données du fichier
			if (_Format == AL_FORMAT_MONO8 || _Format == AL_FORMAT_MONO16) {
				_NbPistes = 1;
				if (_Format == AL_FORMAT_MONO8)
					_NbBits = 8;
				else 
					_NbBits = 16;
			}
			else{
				_NbPistes = 2;
				if (_Format == AL_FORMAT_STEREO8)
					_NbBits = 8;
				else
					_NbBits = 16;
			}
			if(_Format != AL_FORMAT_MONO16)
				Log::log(Log::ENGINE_INFO, "Warning : format not supported.");
			_TailleEchantillon = (_NbBits/8) * _NbPistes;
			_DureeBase = (_Size / _TailleEchantillon) / _Frequency;
			_PtRdFinBaseBuffer = (uint8*)_BaseBuffer + _Size;

			string message = "- Nombre pistes : "+ std::to_string(_NbPistes);
			Log::log(Log::ENGINE_INFO, message.c_str()); 
			message = "- Format : " + std::to_string(_NbBits) + " bits";
			Log::log(Log::ENGINE_INFO, message.c_str());
			message = "- Frequence : " + std::to_string((int)_Frequency) + " hz";
			Log::log(Log::ENGINE_INFO, message.c_str());
			message = "- Taille d'un echantillon : " + std::to_string(_TailleEchantillon) + " octets";
			Log::log(Log::ENGINE_INFO, message.c_str());
			message = "- Nombre total d'echantillons : " + std::to_string(_Size/_TailleEchantillon);
			Log::log(Log::ENGINE_INFO, message.c_str());
			message = "- Durée du fichier : " + std::to_string(_DureeBase) + " secondes";
			Log::log(Log::ENGINE_INFO, message.c_str());

			//Création du buffer de grain

			//Initialisation pour échantillon 16 bits
			_DureeBufferSyntheGrain = 0.3f;
			_DatasSyntheGrain = new sint16[_Frequency*_DureeBufferSyntheGrain];
			_TailleBufferSyntheGrain = _Frequency*_DureeBufferSyntheGrain;
			_PtFinBufferSyntheGrain = (sint16*)_DatasSyntheGrain + _TailleBufferSyntheGrain;
			_PtRdSyntheGrain = (sint16*)_DatasSyntheGrain;
			_PtWrSyntheGrain = (sint16*)_DatasSyntheGrain+1;
			
			//On set des params par defaut au cas ou
			setGrainParam(0.5f, 0.1f, 0.2f, 0.00f, 0.5f);
			selectGrain();
			writeSize = _TailleBufferSyntheGrain-1;
		}
		

	}

	void unload()
	{
		SAFEDELETE_TAB(_DatasSyntheGrain);
		free(_BaseBuffer); _BaseBuffer = NULL; //AlutLoad* = malloc;
	}

	void setGrainParam(float posGrain, float dureeGrain, float randomPos, float partOverlap, float crossFade)
	{
		_PosGrainf = posGrain;
		_DureeGrain = dureeGrain;
		_TailleGrain = (int) ( _DureeGrain * _Frequency * _TailleEchantillon );
		_TailleGrain = (_TailleGrain/4)*4; //On s'aligne sur stereo 16 bits
		_PosGraini = (int) (_Size * _PosGrainf);
		_PosGraini = (_PosGraini / 4) * 4;  //On s'aligne sur 16 bits deux voies au cas ou 
		_RandomWidth = (int) ( randomPos * _Frequency *_TailleEchantillon );
		_SizeOverlap = (int ) ( partOverlap * _DureeGrain * _Frequency * _TailleEchantillon );
		_SizeOverlap = (_SizeOverlap/4)*4; //On aligne
		_CrossFade = crossFade;
	}


private :

	/**
	  * Phase de synthèse
	  * Cette méthode va lire le buffer qui contient le fichier de base pour en extraire de grains
	  * qu'elle va copier au fur et à mesure dans le buffer de synthèse
	  * Quand elle a fini la synthèse, elle retourne un échantillon pris dans le buffer de synthèse, 
	  * sous le pointeur de lecture, et avance ce pointeur de lecture.
	  */
	virtual float getNextSample()
	{
		sint16* buffer = (sint16*)_DatasSyntheGrain;
		while (_PtWrSyntheGrain != _PtRdSyntheGrain) {
			if (_PtRdGrain == _PtRdFinGrain) {
				selectGrain();
				_PtWrSyntheGrain -= _SizeOverlap;
				if (_PtWrSyntheGrain < buffer)
					_PtWrSyntheGrain += _TailleBufferSyntheGrain;
			}
			*_PtWrSyntheGrain = crossFade();
			_PtRdGrain++;
			_PtWrSyntheGrain++;
			if (_PtWrSyntheGrain >= _PtFinBufferSyntheGrain)
				_PtWrSyntheGrain -= _TailleBufferSyntheGrain;
		}
		float output = (float)*_PtRdSyntheGrain;
		*_PtRdSyntheGrain = 0;
		_PtRdSyntheGrain++;
		if(_PtRdSyntheGrain>= _PtFinBufferSyntheGrain)
			_PtRdSyntheGrain -= _TailleBufferSyntheGrain;
		return output/(65535.0f/2.0f);
	}

	void selectGrain() {
		int width = (randf() - 0.5f)*_RandomWidth;
		if (_TailleGrain <= 0)
			_TailleGrain = 1;
		_PtRdGrain = (sint16*)((uint8*)_BaseBuffer + _PosGraini + width);
		if ((uint8*)_BaseBuffer + _Size <= (uint8*)_PtRdGrain + _TailleGrain)
			_PtRdGrain = (sint16*)((uint8*)_BaseBuffer + _Size - _TailleGrain);
		_PtRdFinGrain = (sint16*)((uint8*)_PtRdGrain + _TailleGrain);
	}

	float crossFade() {
		float alpha = (_PtRdFinGrain-_PtRdGrain) / (float)_TailleGrain;
		float alpha2;
		if (_PtRdFinGrain - _SizeOverlap > _PtRdGrain)
			alpha2 = (_PtRdFinGrain - _SizeOverlap - _PtRdGrain) / (float)_TailleGrain;
		else
			alpha2 = 0;
		alpha = 1 - alpha;
		alpha2 = 1 - alpha2;
		return sin(alpha2*(3.14159265359/2))*(*_PtWrSyntheGrain) +
			sin(alpha*(3.14159265359 / 2))*(*_PtRdGrain);
	}
	
};

#endif
