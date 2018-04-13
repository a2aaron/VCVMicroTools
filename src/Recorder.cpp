#include "MicroTools.hpp"
#include "dsp/digital.hpp"
#include "wavwriter.hpp"

#include <vector>
#include <iostream>
struct Recorder : Module {
    // The record button
    enum ParamIds {
        NUM_PARAMS = 1,
    };
    
    enum OutputIds {
        NUM_OUTPUTS = 0,
    };

    // A status light for the record butotn
    enum LightIds {
        NUM_LIGHTS = 1,
    };

    // 2 inputs, for left and right channels
    enum InputIds {
        NUM_INPUTS = 2,
    };

    std::vector<uint8_t> buffer; // keeps track of the currently written samples
    bool recording = false;

    Recorder() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
    void onStart() {
        buffer = std::vector<uint8_t>();
        lights[0].setBrightness(0.9f);
        printf("START!!\n");
        // TODO: do some set up with a file??
    }

    void onEnd() {
        lights[0].setBrightness(0.0f);
        writewav(&buffer[0], buffer.size(), engineGetSampleRate(), "hewwo.wav");
        printf("END! %lu\n", buffer.size());
    }
};

void Recorder::step() {
    bool button_on = params[0].value > 0 ? true : false;
    // Went from not recording state to recording state
    if (!recording && button_on) {
        onStart();
    }

    if (recording && !button_on) {
        onEnd();
    }

    if (recording) {
        float input = (inputs[0].value + 12) * 255 / 24;
        // Typical values are from -12.0f to 12.0f, so to convert to range 
        buffer.push_back((uint8_t) input);
    }

    recording = button_on;
}

template <typename BASE>
struct TriggerLight : BASE {
    TriggerLight() {
        this->box.size = mm2px(Vec(6.0f, 6.0f));
    }
};

struct RecorderWidget : ModuleWidget {
    RecorderWidget(Recorder *module);
};


RecorderWidget::RecorderWidget(Recorder *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/PushButton.svg"))); // TODO actually make an svg for this

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

    addParam(ParamWidget::create<CKSS>(Vec(62, 150), module, 0, 0.0f, 1.0f, 0.0f));
    addChild(ModuleLightWidget::create<TriggerLight<GreenLight>>(Vec(52, 55), module, 0));
    addInput(Port::create<PJ301MPort>(Vec(86.393, 50.414), Port::INPUT, module, 0));
}

Model *modelRecorder = Model::create<Recorder, RecorderWidget>("MicroTools", "Recorder", "Recorder", UTILITY_TAG);

