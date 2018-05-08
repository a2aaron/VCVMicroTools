#include "MicroTools.hpp"
#include "dsp/digital.hpp"

struct NoiseGenerator : Module {
    enum ParamIds {
        PERIOD_KNOB = 0,
        VOLUME_KNOB = 1,
        PERIOD_MULTIPLIER = 2,
        NUM_PARAMS = 3,
    };

    enum OutputIds {
        NUM_OUTPUTS = 1,
    };

    enum LightIds {
        NUM_LIGHTS = 0,
    };

    enum InputIds {
        PERIOD_CV = 0,
        VOLUME_CV = 1,
        NUM_INPUTS = 2,
    };

    NoiseGenerator() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
    int timer = 0;
    float last_out = 0.0f;
};

void NoiseGenerator::step() {

    // The knob and CV inputs are from 0V to 10V. (note that negative CV inputs will still work)
    // We then multiply the knob and CV inputs by 10V to obtain a nicer distribution of values.
    // This way, clock length can range from 1 to 1000 instead of just 1 to 10.
    // PERIOD_MULTIPLIER will increase the range from 1 to 100,000
    float f_clock_length = (params[PERIOD_KNOB].value + inputs[PERIOD_CV].value) * 10.0f;
    f_clock_length *= params[PERIOD_MULTIPLIER].value ? 1000.0f : 1.0f;
    
    int clock_length = max(static_cast<int>(f_clock_length), 1);

    float volume = params[VOLUME_KNOB].value + inputs[VOLUME_CV].value;
    // Every clock_length frames, set the current output to a new sample.
    // TODO: don't use the set frame rate, actually do this via time.
    if (timer % clock_length == 0){
        float sample = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        last_out = sample * volume;
    }
    timer = (timer + 1) % clock_length;

    outputs[0].value = last_out;
}

struct NoiseGeneratorWidget : ModuleWidget {
    NoiseGeneratorWidget(NoiseGenerator *module);
};

NoiseGeneratorWidget::NoiseGeneratorWidget(NoiseGenerator *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/NoiseGenerator.svg")));

    // Mounting Screws
    addChild(Widget::create<ScrewSilver>(Vec(5, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(5, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(70, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(50, 365)));

    addOutput(Port::create<PJ301MPort>(Vec(50, 260), Port::OUTPUT, module, 0));

    int knob_x = 15;
    int volume_y = 55;
    int period_y = 105;
    int cv_x = 58;
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(knob_x, volume_y), module, NoiseGenerator::VOLUME_KNOB, 0.0f, 12.0f, 1.0f));
    addInput(Port::create<PJ301MPort>(Vec(cv_x, volume_y), Port::INPUT, module, NoiseGenerator::VOLUME_CV));

    addParam(ParamWidget::create<RoundBlackKnob>(Vec(knob_x, period_y), module, NoiseGenerator::PERIOD_KNOB, 0.0f, 10.0f, 0.0f));

    addParam(ParamWidget::create<CKSS>(Vec(30, 150), module, NoiseGenerator::PERIOD_MULTIPLIER, 0.0f, 1.0f, 0.0f));
    addInput(Port::create<PJ301MPort>(Vec(cv_x, period_y), Port::INPUT, module, NoiseGenerator::PERIOD_CV));
}

Model *modelNoiseGenerator = Model::create<NoiseGenerator, NoiseGeneratorWidget>("MicroTools", "Noise Generator", "Noise Generator", NOISE_TAG);
