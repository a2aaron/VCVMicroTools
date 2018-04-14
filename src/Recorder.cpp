#include "MicroTools.hpp"
#include "dsp/digital.hpp"
#include "wavwriter.hpp"

#include <vector>
#include <iostream>

// Returns true if the filename exists
bool file_exists(const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}

struct Recorder : Module {
    // The record button
    enum ParamIds {
        NUM_PARAMS = 2,
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

    std::vector<float> buffer; // keeps track of the currently written samples
    bool recording = false;

    int num_channels = 1;
    
    Recorder() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void Recorder::step() {
    bool button_on = params[0].value > 0 ? true : false;
    bool is_stereo = params[1].value > 0 ? true : false;

    if (not recording) {
        num_channels = is_stereo ? 2 : 1;
        printf("Num Channels: %d\n", num_channels);
    }

    // Went from not recording state to recording state
    if (not recording and button_on) {
        lights[0].setBrightness(1.0f);
        buffer.clear();
    }

    if (recording and not button_on) {
        lights[0].setBrightness(0.0f);
        int i = 0;
        std::string filename = "recording0.wav";

        do {
            i++;
            filename = "recording" + std::to_string(i) + ".wav";
        }
        while (file_exists(filename));
        
        writewav(&buffer[0], num_channels, buffer.size(), engineGetSampleRate(), filename.c_str());
    }

    if (recording) {
        // Audio output from Eurorack devices is +-12V, but wav files are expecting
        // floats between -1 and 1, so we must divide to be within that range.
        if (num_channels == 1) {
            buffer.push_back(inputs[0].value / 12);
        } else {
            // When stereo, the samples are interleaved.
            buffer.push_back(inputs[0].value / 12);
            buffer.push_back(inputs[1].value / 12);
        }
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
    addInput(Port::create<PJ301MPort>(Vec(86.393, 100.414), Port::INPUT, module, 1));

    // Mono/Stereo switch
    addParam(ParamWidget::create<CKSS>(Vec(62, 100), module, 1, 0.0f, 1.0f, 0.0f));
}

Model *modelRecorder = Model::create<Recorder, RecorderWidget>("MicroTools", "Recorder", "Recorder", UTILITY_TAG);

