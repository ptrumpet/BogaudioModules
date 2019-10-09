
#include "ShaperPlus.hpp"

void ShaperPlus::reset() {
	for (int c = 0; c < _channels; ++c) {
		_core[c]->reset();
	}
}

int ShaperPlus::channels() {
	return std::max(inputs[SIGNAL_INPUT].getChannels(), inputs[TRIGGER_INPUT].getChannels());
}

void ShaperPlus::addEngine(int c) {
	_core[c] = new ShaperCore(
		params[ATTACK_PARAM],
		params[ON_PARAM],
		params[DECAY_PARAM],
		params[OFF_PARAM],
		params[ENV_PARAM],
		params[SIGNAL_PARAM],
		params[TRIGGER_PARAM],
		params[SPEED_PARAM],
		params[LOOP_PARAM],

		inputs[SIGNAL_INPUT],
		inputs[TRIGGER_INPUT],
		&inputs[ATTACK_INPUT],
		&inputs[ON_INPUT],
		&inputs[DECAY_INPUT],
		&inputs[OFF_INPUT],
		&inputs[ENV_INPUT],
		&inputs[SIGNALCV_INPUT],

		outputs[SIGNAL_OUTPUT],
		outputs[ENV_OUTPUT],
		outputs[INV_OUTPUT],
		outputs[TRIGGER_OUTPUT],
		&outputs[ATTACK_OUTPUT],
		&outputs[ON_OUTPUT],
		&outputs[DECAY_OUTPUT],
		&outputs[OFF_OUTPUT],

		_attackLights,
		_onLights,
		_decayLights,
		_offLights,

		_triggerOnLoad,
		_shouldTriggerOnLoad
	);
}

void ShaperPlus::removeEngine(int c) {
	delete _core[c];
	_core[c] = NULL;
}

void ShaperPlus::processChannel(const ProcessArgs& args, int c) {
	_core[c]->step(c, _channels);
}

void ShaperPlus::postProcess(const ProcessArgs& args) {
	float attackSum = 0.0f;
	float onSum = 0.0f;
	float decaySum = 0.0f;
	float offSum = 0.0f;
	for (int c = 0; c < _channels; ++c) {
		attackSum += _attackLights[c];
		onSum += _onLights[c];
		decaySum += _decayLights[c];
		offSum += _offLights[c];
	}
	lights[ATTACK_LIGHT].value = attackSum / (float)_channels;
	lights[ON_LIGHT].value = onSum / (float)_channels;
	lights[DECAY_LIGHT].value = decaySum / (float)_channels;
	lights[OFF_LIGHT].value = offSum / (float)_channels;
}

bool ShaperPlus::shouldTriggerOnNextLoad() {
	for (int c = 0; c < _channels; ++c) {
		if (_core[c]->_stage != _core[c]->STOPPED_STAGE) {
			return true;
		}
	}
	return false;
}

struct ShaperPlusWidget : TriggerOnLoadModuleWidget {
	static constexpr int hp = 15;

	ShaperPlusWidget(ShaperPlus* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SvgPanel *panel = new SvgPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ShaperPlus.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 30, 365)));

		// generated by svg_widgets.rb
		auto attackParamPosition = Vec(29.08, 33.08);
		auto triggerParamPosition = Vec(89.04, 43.04);
		auto onParamPosition = Vec(29.08, 89.08);
		auto speedParamPosition = Vec(119.0, 96.0);
		auto decayParamPosition = Vec(29.08, 145.08);
		auto loopParamPosition = Vec(119.0, 152.0);
		auto offParamPosition = Vec(29.08, 201.08);
		auto envParamPosition = Vec(82.38, 257.08);
		auto signalParamPosition = Vec(82.38, 313.08);

		auto triggerInputPosition = Vec(114.0, 40.0);
		auto attackInputPosition = Vec(152.0, 40.0);
		auto onInputPosition = Vec(152.0, 96.0);
		auto decayInputPosition = Vec(152.0, 152.0);
		auto offInputPosition = Vec(152.0, 208.0);
		auto envInputPosition = Vec(152.0, 264.0);
		auto signalInputPosition = Vec(11.5, 320.0);
		auto signalcvInputPosition = Vec(152.0, 320.0);

		auto attackOutputPosition = Vec(189.0, 40.0);
		auto onOutputPosition = Vec(189.0, 96.0);
		auto decayOutputPosition = Vec(189.0, 152.0);
		auto offOutputPosition = Vec(189.0, 208.0);
		auto envOutputPosition = Vec(11.5, 264.0);
		auto invOutputPosition = Vec(40.5, 264.0);
		auto triggerOutputPosition = Vec(189.0, 264.0);
		auto signalOutputPosition = Vec(40.5, 320.0);

		auto attackLightPosition = Vec(12.0, 80.0);
		auto onLightPosition = Vec(12.0, 121.0);
		auto decayLightPosition = Vec(12.0, 189.0);
		auto offLightPosition = Vec(12.0, 237.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob38>(attackParamPosition, module, ShaperPlus::ATTACK_PARAM));
		addParam(createParam<Knob38>(onParamPosition, module, ShaperPlus::ON_PARAM));
		addParam(createParam<Knob38>(decayParamPosition, module, ShaperPlus::DECAY_PARAM));
		addParam(createParam<Knob38>(offParamPosition, module, ShaperPlus::OFF_PARAM));
		addParam(createParam<Knob38>(envParamPosition, module, ShaperPlus::ENV_PARAM));
		addParam(createParam<Knob38>(signalParamPosition, module, ShaperPlus::SIGNAL_PARAM));

		addParam(createParam<Button18>(triggerParamPosition, module, ShaperPlus::TRIGGER_PARAM));
		addInput(createInput<Port24>(triggerInputPosition, module, ShaperPlus::TRIGGER_INPUT));

		addParam(createParam<SliderSwitch2State14>(speedParamPosition, module, ShaperPlus::SPEED_PARAM));
		addParam(createParam<SliderSwitch2State14>(loopParamPosition, module, ShaperPlus::LOOP_PARAM));
		addOutput(createOutput<Port24>(triggerOutputPosition, module, ShaperPlus::TRIGGER_OUTPUT));

		addOutput(createOutput<Port24>(envOutputPosition, module, ShaperPlus::ENV_OUTPUT));
		addOutput(createOutput<Port24>(invOutputPosition, module, ShaperPlus::INV_OUTPUT));

		addInput(createInput<Port24>(signalInputPosition, module, ShaperPlus::SIGNAL_INPUT));
		addOutput(createOutput<Port24>(signalOutputPosition, module, ShaperPlus::SIGNAL_OUTPUT));

		addInput(createInput<Port24>(attackInputPosition, module, ShaperPlus::ATTACK_INPUT));
		addInput(createInput<Port24>(onInputPosition, module, ShaperPlus::ON_INPUT));
		addInput(createInput<Port24>(decayInputPosition, module, ShaperPlus::DECAY_INPUT));
		addInput(createInput<Port24>(offInputPosition, module, ShaperPlus::OFF_INPUT));
		addInput(createInput<Port24>(envInputPosition, module, ShaperPlus::ENV_INPUT));
		addInput(createInput<Port24>(signalcvInputPosition, module, ShaperPlus::SIGNALCV_INPUT));

		addOutput(createOutput<Port24>(attackOutputPosition, module, ShaperPlus::ATTACK_OUTPUT));
		addOutput(createOutput<Port24>(onOutputPosition, module, ShaperPlus::ON_OUTPUT));
		addOutput(createOutput<Port24>(decayOutputPosition, module, ShaperPlus::DECAY_OUTPUT));
		addOutput(createOutput<Port24>(offOutputPosition, module, ShaperPlus::OFF_OUTPUT));

		addChild(createLight<TinyLight<GreenLight>>(attackLightPosition, module, ShaperPlus::ATTACK_LIGHT));
		addChild(createLight<TinyLight<GreenLight>>(onLightPosition, module, ShaperPlus::ON_LIGHT));
		addChild(createLight<TinyLight<GreenLight>>(decayLightPosition, module, ShaperPlus::DECAY_LIGHT));
		addChild(createLight<TinyLight<GreenLight>>(offLightPosition, module, ShaperPlus::OFF_LIGHT));
	}
};

Model* modelShaperPlus = bogaudio::createModel<ShaperPlus, ShaperPlusWidget>("Bogaudio-ShaperPlus", "SHAPER+", "envelope and amplifier", "Envelope generator", "VCA", "Polyphonic");
