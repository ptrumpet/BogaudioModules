#pragma once

#include "bogaudio.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelVCA;

namespace bogaudio {

struct VCA : BGModule {
	enum ParamsIds {
		LEVEL1_PARAM,
		LEVEL2_PARAM,
		LINEAR_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		CV1_INPUT,
		IN1_INPUT,
		CV2_INPUT,
		IN2_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		LINEAR_LIGHT,
		NUM_LIGHTS
	};

	Amplifier _amplifier1[maxChannels];
	bogaudio::dsp::SlewLimiter _levelSL1[maxChannels];
	Amplifier _amplifier2[maxChannels];
	bogaudio::dsp::SlewLimiter _levelSL2[maxChannels];

	struct LevelParamQuantity : AmpliferParamQuantity {
		bool isLinear() override;
	};

	VCA() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam<LevelParamQuantity>(LEVEL1_PARAM, 0.0f, 1.0f, 0.8f, "Level 1");
		configParam<LevelParamQuantity>(LEVEL2_PARAM, 0.0f, 1.0f, 0.8f, "Level 2");
		configParam(LINEAR_PARAM, 0.0f, 1.0f, 0.0f, "Linear");

		sampleRateChange();
	}

	inline bool isLinear() { return params[LINEAR_PARAM].getValue() > 0.5f; }
	void sampleRateChange() override;
	void processChannel(const ProcessArgs& args, int c) override;
	void channelStep(Input& input, Output& output, Param& knob, Input& cv, Amplifier* amplifier, bogaudio::dsp::SlewLimiter* levelSL, bool linear);
};

} // namespace bogaudio
