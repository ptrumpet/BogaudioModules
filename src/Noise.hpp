#pragma once

#include "bogaudio.hpp"
#include "dsp/noise.hpp"

using namespace bogaudio::dsp;

extern Model* modelNoise;

namespace bogaudio {

struct Noise : BGModule {
	enum ParamsIds {
		NUM_PARAMS
	};

	enum InputsIds {
		ABS_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		WHITE_OUTPUT,
		PINK_OUTPUT,
		RED_OUTPUT,
		GAUSS_OUTPUT,
		ABS_OUTPUT,
		BLUE_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	int _noiseChannels = 1;
	BlueNoiseGenerator _blue;
	WhiteNoiseGenerator _white;
	PinkNoiseGenerator _pink;
	RedNoiseGenerator _red;
	GaussianNoiseGenerator _gauss;

	Noise() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	json_t* dataToJson() override;
	void dataFromJson(json_t* root) override;
	void processChannel(const ProcessArgs& args, int c) override;
};

} // namespace bogaudio
