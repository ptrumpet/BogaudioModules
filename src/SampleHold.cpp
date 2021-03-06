
#include "SampleHold.hpp"

#define NOISE_TYPE "noise_type"
#define RANGE_OFFSET "range_offset"
#define RANGE_SCALE "range_scale"

void SampleHold::reset() {
	for (int i = 0; i < maxChannels; ++i) {
		_trigger1[i].reset();
		_value1[i] = 0.0f;
		_trigger2[i].reset();
		_value2[i] = 0.0f;
	}
}

json_t* SampleHold::dataToJson() {
	json_t* root = json_object();
	json_object_set_new(root, NOISE_TYPE, json_integer((int)_noiseType));
	json_object_set_new(root, RANGE_OFFSET, json_real(_rangeOffset));
	json_object_set_new(root, RANGE_SCALE, json_real(_rangeScale));
	return root;
}

void SampleHold::dataFromJson(json_t* root) {
	json_t* nt = json_object_get(root, NOISE_TYPE);
	if (nt) {
		_noiseType = (NoiseType)json_integer_value(nt);
	}

	json_t* ro = json_object_get(root, RANGE_OFFSET);
	if (ro) {
		_rangeOffset = json_real_value(ro);
	}

	json_t* rs = json_object_get(root, RANGE_SCALE);
	if (rs) {
		_rangeScale = json_real_value(rs);
	}
}

void SampleHold::processChannel(const ProcessArgs& args, int c) {
	assert(c == 0);

	processChannel(
		lights[TRACK1_LIGHT],
		params[TRACK1_PARAM],
		_trigger1,
		params[TRIGGER1_PARAM],
		inputs[TRIGGER1_INPUT],
		NULL,
		inputs[IN1_INPUT],
		_value1,
		outputs[OUT1_OUTPUT]
	);
	processChannel(
		lights[TRACK2_LIGHT],
		params[TRACK2_PARAM],
		_trigger2,
		params[TRIGGER2_PARAM],
		inputs[TRIGGER2_INPUT],
		&inputs[TRIGGER1_INPUT],
		inputs[IN2_INPUT],
		_value2,
		outputs[OUT2_OUTPUT]
	);
}

void SampleHold::processChannel(
	Light& trackLight,
	Param& trackParam,
	Trigger* trigger,
	Param& triggerParam,
	Input& triggerInput,
	Input* altTriggerInput,
	Input& in,
	float* value,
	Output& out
) {
	trackLight.value = trackParam.getValue();

	int n = std::max(1, std::max(triggerInput.getChannels(), in.getChannels()));
	out.setChannels(n);
	for (int i = 0; i < n; ++i) {
		float triggerIn = 0.0f;
		if (triggerInput.isConnected()) {
			triggerIn = triggerInput.getPolyVoltage(i);
		} else if (altTriggerInput) {
			triggerIn = altTriggerInput->getPolyVoltage(i);
		}
		bool triggered = trigger[i].process(triggerParam.getValue() + triggerIn);
		if (trackParam.getValue() > 0.5f ? trigger[i].isHigh() : triggered) {
			if (in.isConnected()) {
				value[i] = in.getPolyVoltage(i);
			}
			else {
				value[i] = (noise() + _rangeOffset) * _rangeScale;
			}
		}
		out.setVoltage(value[i], i);
	}
}

float SampleHold::noise() {
	switch (_noiseType) {
		case BLUE_NOISE_TYPE: {
			return clamp(2.0f * _blue.next(), -1.0f, 1.0f);
		}
		case PINK_NOISE_TYPE: {
			return clamp(1.5f * _pink.next(), -1.0f, 1.0f);
		}
		case RED_NOISE_TYPE: {
			return clamp(2.0f * _red.next(), -1.0f, 1.0f);
		}
		default: {
			return clamp(_white.next(), -1.0f, 1.0f);
		}
	}
}

struct SampleHoldWidget : ModuleWidget {
	static constexpr int hp = 3;

	SampleHoldWidget(SampleHold* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SvgPanel *panel = new SvgPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SampleHold.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto trigger1ParamPosition = Vec(13.5, 27.0);
		auto track1ParamPosition = Vec(29.0, 122.7);
		auto trigger2ParamPosition = Vec(13.5, 190.0);
		auto track2ParamPosition = Vec(29.0, 285.7);

		auto trigger1InputPosition = Vec(10.5, 49.0);
		auto in1InputPosition = Vec(10.5, 86.0);
		auto trigger2InputPosition = Vec(10.5, 212.0);
		auto in2InputPosition = Vec(10.5, 249.0);

		auto out1OutputPosition = Vec(10.5, 137.0);
		auto out2OutputPosition = Vec(10.5, 300.0);

		auto track1LightPosition = Vec(7.0, 124.0);
		auto track2LightPosition = Vec(7.0, 287.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Button18>(trigger1ParamPosition, module, SampleHold::TRIGGER1_PARAM));
		addParam(createParam<Button18>(trigger2ParamPosition, module, SampleHold::TRIGGER2_PARAM));
		addParam(createParam<StatefulButton9>(track1ParamPosition, module, SampleHold::TRACK1_PARAM));
		addParam(createParam<StatefulButton9>(track2ParamPosition, module, SampleHold::TRACK2_PARAM));

		addInput(createInput<Port24>(trigger1InputPosition, module, SampleHold::TRIGGER1_INPUT));
		addInput(createInput<Port24>(in1InputPosition, module, SampleHold::IN1_INPUT));
		addInput(createInput<Port24>(trigger2InputPosition, module, SampleHold::TRIGGER2_INPUT));
		addInput(createInput<Port24>(in2InputPosition, module, SampleHold::IN2_INPUT));

		addOutput(createOutput<Port24>(out1OutputPosition, module, SampleHold::OUT1_OUTPUT));
		addOutput(createOutput<Port24>(out2OutputPosition, module, SampleHold::OUT2_OUTPUT));

		addChild(createLight<SmallLight<GreenLight>>(track1LightPosition, module, SampleHold::TRACK1_LIGHT));
		addChild(createLight<SmallLight<GreenLight>>(track2LightPosition, module, SampleHold::TRACK2_LIGHT));
	}

	struct NoiseTypeMenuItem : MenuItem {
		SampleHold* _module;
		SampleHold::NoiseType _noiseType;

		NoiseTypeMenuItem(SampleHold* module, const char* label, SampleHold::NoiseType noiseType)
		: _module(module)
		, _noiseType(noiseType)
		{
			this->text = label;
		}

		void onAction(const event::Action& e) override {
			_module->_noiseType = _noiseType;
		}

		void step() override {
			MenuItem::step();
			rightText = _module->_noiseType == _noiseType ? "✔" : "";
		}
	};

	struct RangeMenuItem : MenuItem {
		SampleHold* _module;
		float _offset, _scale;

		RangeMenuItem(SampleHold* module, const char* label, float offset, float scale)
		: _module(module)
		, _offset(offset)
		, _scale(scale)
		{
			this->text = label;
		}

		void onAction(const event::Action& e) override {
			_module->_rangeOffset = _offset;
			_module->_rangeScale = _scale;
		}

		void step() override {
			MenuItem::step();
			rightText = (_module->_rangeOffset == _offset && _module->_rangeScale == _scale) ? "✔" : "";
		}
	};

	void appendContextMenu(Menu* menu) override {
		SampleHold* m = dynamic_cast<SampleHold*>(module);
		assert(m);
		menu->addChild(new MenuLabel());
		menu->addChild(new NoiseTypeMenuItem(m, "Normal noise: blue", SampleHold::BLUE_NOISE_TYPE));
		menu->addChild(new NoiseTypeMenuItem(m, "Normal noise: white", SampleHold::WHITE_NOISE_TYPE));
		menu->addChild(new NoiseTypeMenuItem(m, "Normal noise: pink", SampleHold::PINK_NOISE_TYPE));
		menu->addChild(new NoiseTypeMenuItem(m, "Normal noise: red", SampleHold::RED_NOISE_TYPE));
		menu->addChild(new MenuLabel());
		menu->addChild(new RangeMenuItem(m, "Normal range: +/-10V", 0.0f, 10.0f));
		menu->addChild(new RangeMenuItem(m, "Normal range: +/-5V", 0.0f, 5.0f));
		menu->addChild(new RangeMenuItem(m, "Normal range: +/-3V", 0.0f, 3.0f));
		menu->addChild(new RangeMenuItem(m, "Normal range: +/-1V", 0.0f, 1.0f));
		menu->addChild(new RangeMenuItem(m, "Normal range: 0V-10V", 1.0f, 5.0f));
		menu->addChild(new RangeMenuItem(m, "Normal range: 0V-5V", 1.0f, 2.5f));
		menu->addChild(new RangeMenuItem(m, "Normal range: 0V-3V", 1.0f, 1.5f));
		menu->addChild(new RangeMenuItem(m, "Normal range: 0V-1V", 1.0f, 0.5f));
	}
};

Model* modelSampleHold = bogaudio::createModel<SampleHold, SampleHoldWidget>("Bogaudio-SampleHold", "S&H", "Dual sample (or track) and hold", "Sample and hold", "Dual", "Polyphonic");
