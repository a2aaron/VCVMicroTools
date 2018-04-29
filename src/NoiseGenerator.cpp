#include "MicroTools.hpp"
#include "dsp/digital.hpp"

struct NoiseGenerator : Module {
    enum ParamIds {
        FREQUENCY_KNOB = 0,
        VOLUME_KNOB = 1,
        FREQUENCY_MULTIPLIER = 2,
        NUM_PARAMS = 3,
    };

    enum OutputIds {
        NUM_OUTPUTS = 1,
    };

    enum LightIds {
        NUM_LIGHTS = 0,
    };

    enum InputIds {
        NUM_INPUTS = 0,
    };

    NoiseGenerator() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
    int timer = 0;
    float last_out = 0.0f;
};

void NoiseGenerator::step() {
    int clock_length = static_cast<int>(params[FREQUENCY_KNOB].value);
    bool frequency_multiplier = params[FREQUENCY_MULTIPLIER].value;

    if (frequency_multiplier) {
        clock_length *= 100;
    }

    float volume = params[VOLUME_KNOB].value;

    // Every clock_length frames, set the current output to a new sample.
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
    setPanel(SVG::load(assetPlugin(plugin, "res/Recorder.svg")));

    // Mounting Screws
    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

    addOutput(Port::create<PJ301MPort>(Vec(10, 50), Port::OUTPUT, module, 0));
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(10, 100), module, NoiseGenerator::FREQUENCY_KNOB, 1.0f, 100.0f, 1.0f));
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(10, 150), module, NoiseGenerator::VOLUME_KNOB, 0.0f, 12.0f, 1.0f));
    addParam(ParamWidget::create<CKSS>(Vec(15, 260), module, NoiseGenerator::FREQUENCY_MULTIPLIER, 0.0f, 1.0f, 0.0f));
}

Model *modelNoiseGenerator = Model::create<NoiseGenerator, NoiseGeneratorWidget>("MicroTools", "Noise Generator", "Noise Generator", NOISE_TAG);
