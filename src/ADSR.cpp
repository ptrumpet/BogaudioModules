
#include "ADSR.hpp"

#define INVERT "invert"

void ADSR::Engine::reset() {
	gateTrigger.reset();
	envelope.reset();
}

void ADSR::Engine::sampleRateChange() {
	envelope.setSampleRate(APP->engine->getSampleRate());
}

void ADSR::reset() {
	for (int c = 0; c < _channels; ++c) {
		_engines[c]->reset();
	}
}

void ADSR::sampleRateChange() {
	for (int c = 0; c < _channels; ++c) {
		_engines[c]->sampleRateChange();
	}
}

json_t* ADSR::dataToJson() {
	json_t* root = json_object();
	json_object_set_new(root, INVERT, json_real(_invert));
	return root;
}

void ADSR::dataFromJson(json_t* root) {
	json_t* i = json_object_get(root, INVERT);
	if (i) {
		_invert = json_real_value(i);
	}
}

bool ADSR::active() {
	return inputs[GATE_INPUT].isConnected() || outputs[OUT_OUTPUT].isConnected();
}

int ADSR::channels() {
	return inputs[GATE_INPUT].getChannels();
}

void ADSR::addEngine(int c) {
	_engines[c] = new Engine();
	_engines[c]->reset();
	_engines[c]->sampleRateChange();
}

void ADSR::removeEngine(int c) {
	delete _engines[c];
	_engines[c] = NULL;
}

void ADSR::modulateChannel(int c) {
	Engine& e = *_engines[c];

	e.envelope.setAttack(powf(params[ATTACK_PARAM].getValue(), 2.0f) * 10.f);
	e.envelope.setDecay(powf(params[DECAY_PARAM].getValue(), 2.0f) * 10.f);
	e.envelope.setSustain(params[SUSTAIN_PARAM].getValue());
	e.envelope.setRelease(powf(params[RELEASE_PARAM].getValue(), 2.0f) * 10.f);
	e.envelope.setLinearShape(_linearMode);
}

void ADSR::always(const ProcessArgs& args) {
	lights[LINEAR_LIGHT].value = _linearMode = params[LINEAR_PARAM].getValue() > 0.5f;
	_attackLightSum = _decayLightSum = _sustainLightSum = _releaseLightSum = 0;
}

void ADSR::processChannel(const ProcessArgs& args, int c) {
	Engine& e = *_engines[c];

	e.gateTrigger.process(inputs[GATE_INPUT].getVoltage(c));
	e.envelope.setGate(e.gateTrigger.isHigh());
	outputs[OUT_OUTPUT].setChannels(_channels);
	outputs[OUT_OUTPUT].setVoltage(e.envelope.next() * 10.0f * _invert, c);

	_attackLightSum += e.envelope.isStage(dsp::ADSR::ATTACK_STAGE);
	_decayLightSum += e.envelope.isStage(dsp::ADSR::DECAY_STAGE);
	_sustainLightSum += e.envelope.isStage(dsp::ADSR::SUSTAIN_STAGE);
	_releaseLightSum += e.envelope.isStage(dsp::ADSR::RELEASE_STAGE);
}

void ADSR::postProcess(const ProcessArgs& args) {
	lights[ATTACK_LIGHT].value = _attackLightSum / (float)_channels;
	lights[DECAY_LIGHT].value = _decayLightSum / (float)_channels;
	lights[SUSTAIN_LIGHT].value = _sustainLightSum / (float)_channels;
	lights[RELEASE_LIGHT].value = _releaseLightSum / (float)_channels;
}

struct ADSRWidget : ModuleWidget {
	static constexpr int hp = 3;

	ADSRWidget(ADSR* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SvgPanel *panel = new SvgPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ADSR.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto attackParamPosition = Vec(8.0, 33.0);
		auto decayParamPosition = Vec(8.0, 90.0);
		auto sustainParamPosition = Vec(8.0, 147.0);
		auto releaseParamPosition = Vec(8.0, 204.0);
		auto linearParamPosition = Vec(32.0, 245.7);

		auto gateInputPosition = Vec(10.5, 265.0);

		auto outOutputPosition = Vec(10.5, 303.0);

		auto attackLightPosition = Vec(20.8, 65.0);
		auto decayLightPosition = Vec(20.8, 122.0);
		auto sustainLightPosition = Vec(20.8, 179.0);
		auto releaseLightPosition = Vec(20.8, 236.0);
		auto linearLightPosition = Vec(4.0, 247.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob29>(attackParamPosition, module, ADSR::ATTACK_PARAM));
		addParam(createParam<Knob29>(decayParamPosition, module, ADSR::DECAY_PARAM));
		addParam(createParam<Knob29>(sustainParamPosition, module, ADSR::SUSTAIN_PARAM));
		addParam(createParam<Knob29>(releaseParamPosition, module, ADSR::RELEASE_PARAM));
		addParam(createParam<StatefulButton9>(linearParamPosition, module, ADSR::LINEAR_PARAM));

		addInput(createInput<Port24>(gateInputPosition, module, ADSR::GATE_INPUT));

		addOutput(createOutput<Port24>(outOutputPosition, module, ADSR::OUT_OUTPUT));

		addChild(createLight<TinyLight<GreenLight>>(attackLightPosition, module, ADSR::ATTACK_LIGHT));
		addChild(createLight<TinyLight<GreenLight>>(decayLightPosition, module, ADSR::DECAY_LIGHT));
		addChild(createLight<TinyLight<GreenLight>>(sustainLightPosition, module, ADSR::SUSTAIN_LIGHT));
		addChild(createLight<TinyLight<GreenLight>>(releaseLightPosition, module, ADSR::RELEASE_LIGHT));
		addChild(createLight<SmallLight<GreenLight>>(linearLightPosition, module, ADSR::LINEAR_LIGHT));
	}

	struct InvertMenuItem : MenuItem {
		ADSR* _module;

		InvertMenuItem(ADSR* module, const char* label, int offset)
		: _module(module)
		{
			this->text = label;
		}

		void onAction(const event::Action& e) override {
			if (_module->_invert < 0.0f) {
				_module->_invert = 1.0f;
			}
			else {
				_module->_invert = -1.0f;
			}
		}

		void step() override {
			MenuItem::step();
			rightText = _module->_invert == -1.0f ? "✔" : "";
		}
	};

	void appendContextMenu(Menu* menu) override {
		ADSR* m = dynamic_cast<ADSR*>(module);
		assert(m);
		menu->addChild(new MenuLabel());
		menu->addChild(new InvertMenuItem(m, "Invert output", -1));
	}
};

Model* modelADSR = bogaudio::createModel<ADSR, ADSRWidget>("Bogaudio-ADSR", "ADSR", "Utility ADSR envelope generator", "Envelope generator", "Polyphonic");
