#pragma once

#include "bogaudio.hpp"
#include "mixer.hpp"
#include "dsp/signal.hpp"

using namespace bogaudio::dsp;

extern Model* modelMix8;

namespace bogaudio {

struct Mix8 : BGModule {
	enum ParamsIds {
		LEVEL1_PARAM,
		MUTE1_PARAM,
		PAN1_PARAM,
		LEVEL2_PARAM,
		MUTE2_PARAM,
		PAN2_PARAM,
		LEVEL3_PARAM,
		MUTE3_PARAM,
		PAN3_PARAM,
		LEVEL4_PARAM,
		MUTE4_PARAM,
		PAN4_PARAM,
		LEVEL5_PARAM,
		MUTE5_PARAM,
		PAN5_PARAM,
		LEVEL6_PARAM,
		MUTE6_PARAM,
		PAN6_PARAM,
		LEVEL7_PARAM,
		MUTE7_PARAM,
		PAN7_PARAM,
		LEVEL8_PARAM,
		MUTE8_PARAM,
		PAN8_PARAM,
		MIX_PARAM,
		MIX_MUTE_PARAM,
		NUM_PARAMS
	};

	enum InputsIds {
		CV1_INPUT,
		PAN1_INPUT,
		IN1_INPUT,
		CV2_INPUT,
		PAN2_INPUT,
		IN2_INPUT,
		CV3_INPUT,
		PAN3_INPUT,
		IN3_INPUT,
		CV4_INPUT,
		PAN4_INPUT,
		IN4_INPUT,
		CV5_INPUT,
		PAN5_INPUT,
		IN5_INPUT,
		CV6_INPUT,
		PAN6_INPUT,
		IN6_INPUT,
		CV7_INPUT,
		PAN7_INPUT,
		IN7_INPUT,
		CV8_INPUT,
		PAN8_INPUT,
		IN8_INPUT,
		MIX_CV_INPUT,
		NUM_INPUTS
	};

	enum OutputsIds {
		L_OUTPUT,
		R_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightsIds {
		NUM_LIGHTS
	};

	int _polyChannelOffset = -1;
	MixerChannel* _channels[8] {};
	Amplifier _amplifier;
	bogaudio::dsp::SlewLimiter _slewLimiter;
	Saturator _saturator;
	RootMeanSquare _rms;
	float _rmsLevel = 0.0f;

	Mix8() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		float levelDefault = fabsf(MixerChannel::minDecibels) / (MixerChannel::maxDecibels - MixerChannel::minDecibels);
		configParam(LEVEL1_PARAM, 0.0f, 1.0f, levelDefault, "Channel 1 level", " dB", 0.0f, MixerChannel::maxDecibels - MixerChannel::minDecibels, MixerChannel::minDecibels);
		configParam(PAN1_PARAM, -1.0f, 1.0f, 0.0f, "Channel 1 panning", "%", 0.0f, 100.0f);
		configParam(MUTE1_PARAM, 0.0f, 3.0f, 0.0f, "Channel 1 mute");
		configParam(LEVEL2_PARAM, 0.0f, 1.0f, levelDefault, "Channel 2 level", " dB", 0.0f, MixerChannel::maxDecibels - MixerChannel::minDecibels, MixerChannel::minDecibels);
		configParam(PAN2_PARAM, -1.0f, 1.0f, 0.0f, "Channel 2 panning", "%", 0.0f, 100.0f);
		configParam(MUTE2_PARAM, 0.0f, 3.0f, 0.0f, "Channel 2 mute");
		configParam(LEVEL3_PARAM, 0.0f, 1.0f, levelDefault, "Channel 3 level", " dB", 0.0f, MixerChannel::maxDecibels - MixerChannel::minDecibels, MixerChannel::minDecibels);
		configParam(PAN3_PARAM, -1.0f, 1.0f, 0.0f, "Channel 3 panning", "%", 0.0f, 100.0f);
		configParam(MUTE3_PARAM, 0.0f, 3.0f, 0.0f, "Channel 3 mute");
		configParam(LEVEL4_PARAM, 0.0f, 1.0f, levelDefault, "Channel 4 level", " dB", 0.0f, MixerChannel::maxDecibels - MixerChannel::minDecibels, MixerChannel::minDecibels);
		configParam(PAN4_PARAM, -1.0f, 1.0f, 0.0f, "Channel 4 panning", "%", 0.0f, 100.0f);
		configParam(MUTE4_PARAM, 0.0f, 3.0f, 0.0f, "Channel 4 mute");
		configParam(LEVEL5_PARAM, 0.0f, 1.0f, levelDefault, "Channel 5 level", " dB", 0.0f, MixerChannel::maxDecibels - MixerChannel::minDecibels, MixerChannel::minDecibels);
		configParam(PAN5_PARAM, -1.0f, 1.0f, 0.0f, "Channel 5 panning", "%", 0.0f, 100.0f);
		configParam(MUTE5_PARAM, 0.0f, 3.0f, 0.0f, "Channel 5 mute");
		configParam(LEVEL6_PARAM, 0.0f, 1.0f, levelDefault, "Channel 6 level", " dB", 0.0f, MixerChannel::maxDecibels - MixerChannel::minDecibels, MixerChannel::minDecibels);
		configParam(PAN6_PARAM, -1.0f, 1.0f, 0.0f, "Channel 6 panning", "%", 0.0f, 100.0f);
		configParam(MUTE6_PARAM, 0.0f, 3.0f, 0.0f, "Channel 6 mute");
		configParam(LEVEL7_PARAM, 0.0f, 1.0f, levelDefault, "Channel 7 level", " dB", 0.0f, MixerChannel::maxDecibels - MixerChannel::minDecibels, MixerChannel::minDecibels);
		configParam(PAN7_PARAM, -1.0f, 1.0f, 0.0f, "Channel 7 panning", "%", 0.0f, 100.0f);
		configParam(MUTE7_PARAM, 0.0f, 3.0f, 0.0f, "Channel 7 mute");
		configParam(LEVEL8_PARAM, 0.0f, 1.0f, levelDefault, "Channel 8 level", " dB", 0.0f, MixerChannel::maxDecibels - MixerChannel::minDecibels, MixerChannel::minDecibels);
		configParam(PAN8_PARAM, -1.0f, 1.0f, 0.0f, "Channel 8 panning", "%", 0.0f, 100.0f);
		configParam(MUTE8_PARAM, 0.0f, 3.0f, 0.0f, "Channel 8 mute");
		configParam(MIX_PARAM, 0.0f, 1.0f, levelDefault, "Master level", " dB", 0.0f, MixerChannel::maxDecibels - MixerChannel::minDecibels, MixerChannel::minDecibels);
		configParam(MIX_MUTE_PARAM, 0.0f, 1.0f, 0.0f, "Master mute");

		_channels[0] = new MixerChannel(params[LEVEL1_PARAM], params[PAN1_PARAM], params[MUTE1_PARAM], inputs[CV1_INPUT], inputs[PAN1_INPUT]);
		_channels[1] = new MixerChannel(params[LEVEL2_PARAM], params[PAN2_PARAM], params[MUTE2_PARAM], inputs[CV2_INPUT], inputs[PAN2_INPUT]);
		_channels[2] = new MixerChannel(params[LEVEL3_PARAM], params[PAN3_PARAM], params[MUTE3_PARAM], inputs[CV3_INPUT], inputs[PAN3_INPUT]);
		_channels[3] = new MixerChannel(params[LEVEL4_PARAM], params[PAN4_PARAM], params[MUTE4_PARAM], inputs[CV4_INPUT], inputs[PAN4_INPUT]);
		_channels[4] = new MixerChannel(params[LEVEL5_PARAM], params[PAN5_PARAM], params[MUTE5_PARAM], inputs[CV5_INPUT], inputs[PAN5_INPUT]);
		_channels[5] = new MixerChannel(params[LEVEL6_PARAM], params[PAN6_PARAM], params[MUTE6_PARAM], inputs[CV6_INPUT], inputs[PAN6_INPUT]);
		_channels[6] = new MixerChannel(params[LEVEL7_PARAM], params[PAN7_PARAM], params[MUTE7_PARAM], inputs[CV7_INPUT], inputs[PAN7_INPUT]);
		_channels[7] = new MixerChannel(params[LEVEL8_PARAM], params[PAN8_PARAM], params[MUTE8_PARAM], inputs[CV8_INPUT], inputs[PAN8_INPUT]);
		sampleRateChange();
		_rms.setSensitivity(0.05f);
	}
	virtual ~Mix8() {
		for (int i = 0; i < 8; ++i) {
			delete _channels[i];
		}
	}

	json_t* dataToJson() override;
	void dataFromJson(json_t* root) override;
	void sampleRateChange() override;
	void processChannel(const ProcessArgs& args, int _c) override;
};

} // namespace bogaudio
