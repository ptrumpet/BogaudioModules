
#include "Mix4.hpp"

#define POLY_OFFSET "poly_channel_offset"

json_t* Mix4::dataToJson() {
	json_t* root = json_object();
	json_object_set_new(root, POLY_OFFSET, json_integer(_polyChannelOffset));
	return root;
}

void Mix4::dataFromJson(json_t* root) {
	json_t* o = json_object_get(root, POLY_OFFSET);
	if (o) {
		_polyChannelOffset = json_integer_value(o);
	}
}

void Mix4::sampleRateChange() {
	float sr = APP->engine->getSampleRate();
	for (int i = 0; i < 4; ++i) {
		_channels[i]->setSampleRate(sr);
	}
	_slewLimiter.setParams(sr, MixerChannel::levelSlewTimeMS, MixerChannel::maxDecibels - MixerChannel::minDecibels);
	_rms.setSampleRate(sr);
}

void Mix4::processChannel(const ProcessArgs& args, int _c) {
	bool stereo = outputs[L_OUTPUT].isConnected() && outputs[R_OUTPUT].isConnected();
	bool solo =
		params[MUTE1_PARAM].getValue() > 1.5f ||
		params[MUTE2_PARAM].getValue() > 1.5f ||
		params[MUTE3_PARAM].getValue() > 1.5f ||
		params[MUTE4_PARAM].getValue() > 1.5f;

	{
		float sample = 0.0f;
		if (_polyChannelOffset >= 0) {
			sample = inputs[IN1_INPUT].getPolyVoltage(_polyChannelOffset);
		} else {
			sample = inputs[IN1_INPUT].getVoltageSum();
		}
		_channels[0]->next(sample, stereo, solo);

		for (int i = 1; i < 4; ++i) {
			float sample = 0.0f;
			if (inputs[IN1_INPUT + 3 * i].isConnected()) {
				sample = inputs[IN1_INPUT + 3 * i].getVoltageSum();
			}
			else if (_polyChannelOffset >= 0) {
				sample = inputs[IN1_INPUT].getPolyVoltage(_polyChannelOffset + i);
			}
			_channels[i]->next(sample, stereo, solo);
		}
	}

	float level = Amplifier::minDecibels;
	if (params[MIX_MUTE_PARAM].getValue() < 0.5f) {
		level = params[MIX_PARAM].getValue();
		if (inputs[MIX_CV_INPUT].isConnected()) {
			level *= clamp(inputs[MIX_CV_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f);
		}
		level *= MixerChannel::maxDecibels - MixerChannel::minDecibels;
		level += MixerChannel::minDecibels;
	}
	_amplifier.setLevel(_slewLimiter.next(level));

	float mono = 0.0f;
	for (int i = 0; i < 4; ++i) {
		mono += _channels[i]->out;
	}
	mono = _amplifier.next(mono);
	mono = _saturator.next(mono);
	_rmsLevel = _rms.next(mono) / 5.0f;

	if (stereo) {
		float left = 0.0f;
		for (int i = 0; i < 4; ++i) {
			left += _channels[i]->left;
		}
		left = _amplifier.next(left);
		left = _saturator.next(left);
		outputs[L_OUTPUT].setVoltage(left);

		float right = 0.0f;
		for (int i = 0; i < 4; ++i) {
			right += _channels[i]->right;
		}
		right = _amplifier.next(right);
		right = _saturator.next(right);
		outputs[R_OUTPUT].setVoltage(right);
	}
	else {
		outputs[L_OUTPUT].setVoltage(mono);
		outputs[R_OUTPUT].setVoltage(mono);
	}
}

struct Mix4Widget : ModuleWidget {
	static constexpr int hp = 15;

	Mix4Widget(Mix4* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SvgPanel *panel = new SvgPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Mix4.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

		// generated by svg_widgets.rb
		auto level1ParamPosition = Vec(17.5, 32.0);
		auto mute1ParamPosition = Vec(17.2, 185.7);
		auto pan1ParamPosition = Vec(18.5, 223.0);
		auto level2ParamPosition = Vec(60.5, 32.0);
		auto mute2ParamPosition = Vec(60.2, 185.7);
		auto pan2ParamPosition = Vec(61.5, 223.0);
		auto level3ParamPosition = Vec(103.5, 32.0);
		auto mute3ParamPosition = Vec(103.2, 185.7);
		auto pan3ParamPosition = Vec(104.5, 223.0);
		auto level4ParamPosition = Vec(146.5, 32.0);
		auto mute4ParamPosition = Vec(146.2, 185.7);
		auto pan4ParamPosition = Vec(147.5, 223.0);
		auto mixParamPosition = Vec(189.5, 32.0);
		auto mixMuteParamPosition = Vec(189.2, 185.7);

		auto cv1InputPosition = Vec(14.5, 255.0);
		auto pan1InputPosition = Vec(14.5, 290.0);
		auto in1InputPosition = Vec(14.5, 325.0);
		auto cv2InputPosition = Vec(57.5, 255.0);
		auto pan2InputPosition = Vec(57.5, 290.0);
		auto in2InputPosition = Vec(57.5, 325.0);
		auto cv3InputPosition = Vec(100.5, 255.0);
		auto pan3InputPosition = Vec(100.5, 290.0);
		auto in3InputPosition = Vec(100.5, 325.0);
		auto cv4InputPosition = Vec(143.5, 255.0);
		auto pan4InputPosition = Vec(143.5, 290.0);
		auto in4InputPosition = Vec(143.5, 325.0);
		auto mixCvInputPosition = Vec(186.5, 252.0);

		auto lOutputPosition = Vec(186.5, 290.0);
		auto rOutputPosition = Vec(186.5, 325.0);
		// end generated by svg_widgets.rb

		addSlider(level1ParamPosition, module, Mix4::LEVEL1_PARAM, module ? &module->_channels[0]->rms : NULL);
		addParam(createParam<Knob16>(pan1ParamPosition, module, Mix4::PAN1_PARAM));
		addParam(createParam<SoloMuteButton>(mute1ParamPosition, module, Mix4::MUTE1_PARAM));
		addSlider(level2ParamPosition, module, Mix4::LEVEL2_PARAM, module ? &module->_channels[1]->rms : NULL);
		addParam(createParam<Knob16>(pan2ParamPosition, module, Mix4::PAN2_PARAM));
		addParam(createParam<SoloMuteButton>(mute2ParamPosition, module, Mix4::MUTE2_PARAM));
		addSlider(level3ParamPosition, module, Mix4::LEVEL3_PARAM, module ? &module->_channels[2]->rms : NULL);
		addParam(createParam<Knob16>(pan3ParamPosition, module, Mix4::PAN3_PARAM));
		addParam(createParam<SoloMuteButton>(mute3ParamPosition, module, Mix4::MUTE3_PARAM));
		addSlider(level4ParamPosition, module, Mix4::LEVEL4_PARAM, module ? &module->_channels[3]->rms : NULL);
		addParam(createParam<Knob16>(pan4ParamPosition, module, Mix4::PAN4_PARAM));
		addParam(createParam<SoloMuteButton>(mute4ParamPosition, module, Mix4::MUTE4_PARAM));
		addSlider(mixParamPosition, module, Mix4::MIX_PARAM, module ? &module->_rmsLevel : NULL);
		addParam(createParam<MuteButton>(mixMuteParamPosition, module, Mix4::MIX_MUTE_PARAM));

		addInput(createInput<Port24>(cv1InputPosition, module, Mix4::CV1_INPUT));
		addInput(createInput<Port24>(pan1InputPosition, module, Mix4::PAN1_INPUT));
		addInput(createInput<Port24>(in1InputPosition, module, Mix4::IN1_INPUT));
		addInput(createInput<Port24>(cv2InputPosition, module, Mix4::CV2_INPUT));
		addInput(createInput<Port24>(pan2InputPosition, module, Mix4::PAN2_INPUT));
		addInput(createInput<Port24>(in2InputPosition, module, Mix4::IN2_INPUT));
		addInput(createInput<Port24>(cv3InputPosition, module, Mix4::CV3_INPUT));
		addInput(createInput<Port24>(pan3InputPosition, module, Mix4::PAN3_INPUT));
		addInput(createInput<Port24>(in3InputPosition, module, Mix4::IN3_INPUT));
		addInput(createInput<Port24>(cv4InputPosition, module, Mix4::CV4_INPUT));
		addInput(createInput<Port24>(pan4InputPosition, module, Mix4::PAN4_INPUT));
		addInput(createInput<Port24>(in4InputPosition, module, Mix4::IN4_INPUT));
		addInput(createInput<Port24>(mixCvInputPosition, module, Mix4::MIX_CV_INPUT));

		addOutput(createOutput<Port24>(lOutputPosition, module, Mix4::L_OUTPUT));
		addOutput(createOutput<Port24>(rOutputPosition, module, Mix4::R_OUTPUT));
	}

	void addSlider(Vec position, Mix4* module, int id, float* rms) {
		auto slider = createParam<VUSlider151>(position, module, id);
		if (rms) {
			dynamic_cast<VUSlider*>(slider)->setVULevel(rms);
		}
		addParam(slider);
	}

	struct PolySpreadMenuItem : MenuItem {
		Mix4* _module;
		int _offset;

		PolySpreadMenuItem(Mix4* module, const char* label, int offset)
		: _module(module)
		, _offset(offset)
		{
			this->text = label;
		}

		void onAction(const event::Action& e) override {
			_module->_polyChannelOffset = _offset;
		}

		void step() override {
			MenuItem::step();
			rightText = _module->_polyChannelOffset == _offset ? "✔" : "";
		}
	};

	void appendContextMenu(Menu* menu) override {
		Mix4* m = dynamic_cast<Mix4*>(module);
		assert(m);
		menu->addChild(new MenuLabel());
		menu->addChild(new PolySpreadMenuItem(m, "Input 1 poly spread: none", -1));
		menu->addChild(new PolySpreadMenuItem(m, "Input 1 poly spread: channels 1-4", 0));
		menu->addChild(new PolySpreadMenuItem(m, "Input 1 poly spread: channels 5-8", 4));
		menu->addChild(new PolySpreadMenuItem(m, "Input 1 poly spread: channels 9-12", 4));
		menu->addChild(new PolySpreadMenuItem(m, "Input 1 poly spread: channels 13-16", 4));
	}
};

Model* modelMix4 = bogaudio::createModel<Mix4, Mix4Widget>("Bogaudio-Mix4", "MIX4", "4-channel mixer and panner", "Mixer", "Panning");
