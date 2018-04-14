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
    enum ParamIds {
        RECORD_BUTTON = 0,
        MONO_STEREO = 1,
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
        LEFT_INPUT = 0,
        RIGHT_INPUT = 1,
        NUM_INPUTS = 2,
    };

    std::vector<float> buffer; // keeps track of the currently written samples
    bool recording = false;

    int num_channels = 1;

    Recorder() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    float getSeconds() {
        return float(buffer.size()) / engineGetSampleRate();
    }
};

void Recorder::step() {
    bool button_on = params[Recorder::RECORD_BUTTON].value;
    bool is_stereo = params[Recorder::MONO_STEREO].value;

    if (not recording) {
        num_channels = is_stereo ? 2 : 1;
    }

    // Went from not recording state to recording state
    if (not recording and button_on) {
        printf("Recording %d channels\n", num_channels);
        buffer.clear();
    }

    if (recording and not button_on) {
        int i = 0;
        std::string filename = "recording0.wav";

        do {
            i++;
            filename = "recording" + std::to_string(i) + ".wav";
        }
        while (file_exists(filename));

        writewav(&buffer[0], Format::FLOAT_32, num_channels, buffer.size(), engineGetSampleRate(), filename.c_str());
        printf("Wrote %s\n", filename.c_str());
    }

    if (recording) {
        // Audio output from Eurorack devices is +-12V, but wav files are expecting
        // floats between -1 and 1, so we must divide to be within that range.
        if (num_channels == 1) {
            buffer.push_back(inputs[Recorder::LEFT_INPUT].value / 12);
        } else {
            // When stereo, the samples are interleaved.
            buffer.push_back(inputs[Recorder::LEFT_INPUT].value / 12);
            buffer.push_back(inputs[Recorder::RIGHT_INPUT].value / 12);
        }
    }

    recording = button_on;
}

struct RecordButton : SVGSwitch, ToggleSwitch {
    RecordButton() {
        addFrame(SVG::load(assetPlugin(plugin, "res/DarkButton.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/LightButton.svg")));
    }
};

struct RecordingTimer : LedDisplay {
    char msg[8] = {0};
    LedDisplayChoice *text = nullptr;
    bool recording = false;

    RecordingTimer() {
        box.size = Vec(35, 22);
        text = Widget::create<LedDisplayChoice>(Vec(0, 0));
        text->textOffset = Vec(3, 14);
        text->box.size = Vec(35, 25);
        setSeconds(0);
        addChild(text);
    }

    void setSeconds(float seconds) {
        float frac_second = seconds - int(seconds);
        if (seconds > 60 * 60) {
            seconds /= 60;
        }

        int a = int(seconds) / 60, b = int(seconds) % 60;

        // Blink the : every second
        if (frac_second < 0.5 or not recording) {
            sprintf(msg, "%02d:%02d", a, b);
        } else {
            sprintf(msg, "%02d %02d", a, b);
        }

        text->text = msg;
    }
};

struct RecorderWidget : ModuleWidget {
    Recorder *recorder;
    RecordingTimer *recordingTimer;
    RecorderWidget(Recorder *module);
    void step() override;
};

RecorderWidget::RecorderWidget(Recorder *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/Recorder.svg")));
    recorder = module;

    // Mounting Screws
    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));

    // Recording ports
    addInput(Port::create<PJ301MPort>(Vec(10, 50), Port::INPUT, module, Recorder::LEFT_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(10, 90), Port::INPUT, module, Recorder::RIGHT_INPUT));

    recordingTimer = Widget::create<RecordingTimer>(Vec(5, 140));
    addChild(recordingTimer);

    addParam(ParamWidget::create<RecordButton>(Vec(7.5, 170), module, Recorder::RECORD_BUTTON, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<CKSS>(Vec(15, 260), module, Recorder::MONO_STEREO, 0.0f, 1.0f, 0.0f));
}

void RecorderWidget::step() {
    recordingTimer->setSeconds(recorder->getSeconds());
    recordingTimer->recording = recorder->recording;
}

Model *modelRecorder = Model::create<Recorder, RecorderWidget>("MicroTools", "Recorder", "Recorder", UTILITY_TAG);
