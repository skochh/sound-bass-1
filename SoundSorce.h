#ifndef SOUNDSORCE_H_
#define SOUNDSORCE_H_

#include "globals.h"

#include "WaveGenerator.h"
#include "EnvelopeGenerator.h"

class SoundSource
{
protected:
	std::shared_ptr<WaveGenerator> _wg { std::make_shared<WaveGenerator>() };
	std::shared_ptr<EnvelopeGenerator> _eg { std::make_shared<EnvelopeGenerator>() };

public:
	SoundSource() = default;
	SoundSource(const SoundSource&) = delete;
	virtual ~SoundSource() = default;

	virtual double next_sample() { return _wg->next_sample() * _eg->next_sample(); }

	virtual void set_envelope_generator(const std::shared_ptr<EnvelopeGenerator>& eg)
	{
		if (eg == nullptr)
			_eg = std::make_shared<EnvelopeGenerator>();
		else
			_eg = eg;
	}

	virtual void set_wave_generator(const std::shared_ptr<WaveGenerator>& wg)
	{
		if (wg == nullptr)
			_wg = std::make_shared<WaveGenerator>();
		else
			_wg = wg;
	}
};




#endif
