#include "MicroTools.hpp"
#include "dsp/digital.hpp"

struct NoiseGenerator : Module {
    enum ParamIds {
        NUM_PARAMS = 2,
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
    int t = 0;
    float last_out = 0.0f;
};

void NoiseGenerator::step() {
    int every = static_cast<int>(params[0].value);
    float amp = params[1].value;
    if (t % every == 0){
        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        last_out = r * amp;
    }
    t = (t + 1) % every;

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
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(10, 100), module, 0, 1.0f, 10000.0f, 1.0f));
    addParam(ParamWidget::create<RoundBlackKnob>(Vec(10, 150), module, 1, 0.0f, 12.0f, 1.0f));
}

Model *modelNoiseGenerator = Model::create<NoiseGenerator, NoiseGeneratorWidget>("MicroTools", "Noise Generator", "Noise Generator", NOISE_TAG);
