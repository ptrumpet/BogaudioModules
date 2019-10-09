
#include "Nsgt.hpp"


void Nsgt::sampleRateChange() {
	float sampleRate = APP->engine->getSampleRate();
	for (int c = 0; c < _channels; ++c) {
		_engines[c]->detector.setSampleRate(sampleRate);
		_engines[c]->attackSL.setParams(sampleRate, 150.0f);
		_engines[c]->releaseSL.setParams(sampleRate, 600.0f);
	}
}

bool Nsgt::active() {
	return outputs[LEFT_OUTPUT].isConnected() || outputs[RIGHT_OUTPUT].isConnected();
}

int Nsgt::channels() {
	return inputs[LEFT_INPUT].getChannels() + inputs[RIGHT_INPUT].getChannels();
}

void Nsgt::addEngine(int c) {
	_engines[c] = new Engine();
}

void Nsgt::removeEngine(int c) {
	delete _engines[c];
	_engines[c] = NULL;
}

void Nsgt::modulate() {
	_softKnee = params[KNEE_PARAM].getValue() > 0.5f;
}

void Nsgt::modulateChannel(int c) {
	Engine& e = *_engines[c];

	if (!_engines[c]) {
		return;
	}

	e.thresholdDb = params[THRESHOLD_PARAM].getValue();
	if (inputs[THRESHOLD_INPUT].isConnected()) {
		e.thresholdDb *= clamp(inputs[THRESHOLD_INPUT].getPolyVoltage(c) / 10.0f, 0.0f, 1.0f);
	}
	e.thresholdDb *= 30.0f;
	e.thresholdDb -= 24.0f;

	float ratio = params[RATIO_PARAM].getValue();
	if (inputs[RATIO_INPUT].isConnected()) {
		ratio *= clamp(inputs[RATIO_INPUT].getPolyVoltage(c) / 10.0f, 0.0f, 1.0f);
	}
	if (e.ratioKnob != ratio) {
		e.ratioKnob = ratio;
		ratio = powf(e.ratioKnob, 1.5f);
		ratio = 1.0f - ratio;
		ratio *= M_PI;
		ratio *= 0.25f;
		ratio = tanf(ratio);
		ratio = 1.0f / ratio;
		e.ratioKnob = ratio;
	}
}

void Nsgt::processChannel(const ProcessArgs& args, int c) {
	Engine& e = *_engines[c];

	if (!_engines[c]) {
		return;
	}

	float leftInput = inputs[LEFT_INPUT].getPolyVoltage(c);
	float rightInput = inputs[RIGHT_INPUT].getPolyVoltage(c);
	float env = e.detector.next(leftInput + rightInput);
	if (env > e.lastEnv) {
		env = e.attackSL.next(env, e.lastEnv);
	}
	else {
		env = e.releaseSL.next(env, e.lastEnv);
	}
	e.lastEnv = env;

	float detectorDb = amplitudeToDecibels(env / 5.0f);
	float compressionDb = e.noiseGate.compressionDb(detectorDb, e.thresholdDb, e.ratio, _softKnee);
	e.amplifier.setLevel(-compressionDb);
	if (outputs[LEFT_OUTPUT].isConnected()) {
		outputs[LEFT_OUTPUT].setChannels(_channels);
		outputs[LEFT_OUTPUT].setVoltage(e.saturator.next(e.amplifier.next(leftInput)), c);
	}
	if (outputs[RIGHT_OUTPUT].isConnected()) {
		outputs[RIGHT_OUTPUT].setChannels(_channels);
		outputs[RIGHT_OUTPUT].setVoltage(e.saturator.next(e.amplifier.next(rightInput)), c);
	}
}

struct NsgtWidget : ModuleWidget {
	static constexpr int hp = 6;

	NsgtWidget(Nsgt* module) {
		setModule(module);
		box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);

		{
			SvgPanel *panel = new SvgPanel();
			panel->box.size = box.size;
			panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Nsgt.svg")));
			addChild(panel);
		}

		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

		// generated by svg_widgets.rb
		auto thresholdParamPosition = Vec(26.0, 52.0);
		auto ratioParamPosition = Vec(26.0, 135.0);
		auto kneeParamPosition = Vec(39.5, 199.5);

		auto leftInputPosition = Vec(16.0, 244.0);
		auto rightInputPosition = Vec(50.0, 244.0);
		auto thresholdInputPosition = Vec(16.0, 280.0);
		auto ratioInputPosition = Vec(50.0, 280.0);

		auto leftOutputPosition = Vec(16.0, 320.0);
		auto rightOutputPosition = Vec(50.0, 320.0);
		// end generated by svg_widgets.rb

		addParam(createParam<Knob38>(thresholdParamPosition, module, Nsgt::THRESHOLD_PARAM));
		addParam(createParam<Knob38>(ratioParamPosition, module, Nsgt::RATIO_PARAM));
		addParam(createParam<SliderSwitch2State14>(kneeParamPosition, module, Nsgt::KNEE_PARAM));

		addInput(createInput<Port24>(leftInputPosition, module, Nsgt::LEFT_INPUT));
		addInput(createInput<Port24>(rightInputPosition, module, Nsgt::RIGHT_INPUT));
		addInput(createInput<Port24>(thresholdInputPosition, module, Nsgt::THRESHOLD_INPUT));
		addInput(createInput<Port24>(ratioInputPosition, module, Nsgt::RATIO_INPUT));

		addOutput(createOutput<Port24>(leftOutputPosition, module, Nsgt::LEFT_OUTPUT));
		addOutput(createOutput<Port24>(rightOutputPosition, module, Nsgt::RIGHT_OUTPUT));
	}
};

Model* modelNsgt = bogaudio::createModel<Nsgt, NsgtWidget>("Bogaudio-Nsgt", "NSGT", "noise gate", "Dynamics", "Polyphonic");
