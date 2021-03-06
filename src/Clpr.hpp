#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelClpr;

namespace bogaudio {

struct Clpr : BGModule {
	enum ParamsIds {
		THRESHOLD_PARAM,
		OUTPUT_GAIN_PARAM,
		KNEE_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		LEFT_INPUT,
		RIGHT_INPUT,
		THRESHOLD_INPUT,
		OUTPUT_GAIN_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		LEFT_OUTPUT,
		RIGHT_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	float _thresholdDb = 0.0f;
	float _outGain = -1.0f;
	float _outLevel = 0.0f;
	bool _softKnee = true;

	Compressor _compressor;
	Amplifier _amplifier;
	Saturator _saturator;

	Clpr() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(THRESHOLD_PARAM, 0.0f, 1.0f, 0.8f, "Threshold", " dB", 0.0f, 30.0f, -24.0f);
		configParam(OUTPUT_GAIN_PARAM, 0.0f, 1.0f, 0.0f, "Output gain", " dB", 0.0f, 24.0f);
		configParam(KNEE_PARAM, 0.0f, 1.0f, 0.0f, "Knee");

		reset();
	}

	bool active() override;
	void modulate() override;
	void processChannel(const ProcessArgs& args, int _c) override;
};

} // namespace bogaudio
