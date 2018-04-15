#include "MicroTools.hpp"
#include "dsp/digital.hpp"
#include "wavwriter.hpp"

#include <vector>
#include <iostream>

std::vector<uint8_t> to_bytes(SampleFmt format, std::vector<float> buffer) {
    std::vector<uint8_t> byte_buffer;
    for (float x : buffer) {
        // Vector storing the sample unpacked into some number of bytes.
        std::vector<uint8_t> bytes;

        // Note: Audio output from Eurorack devices is +-12V
        if (format == SampleFmt::PCM_U8) {
            // Range will be from 0 to 255
            uint8_t sample = 127 * (x / 24 + 1);
            bytes.push_back(sample);
        } else if (format == SampleFmt::PCM_S16) {
            // Range will be from -32766 to 32767
            int16_t sample = 32767 * (x / 12);
            // Pretend the 16bit integer is actually just two 8 bit bytes
            uint8_t const* casted = reinterpret_cast<uint8_t const *>(&sample);
            bytes.push_back(casted[0]);
            bytes.push_back(casted[1]);
        } else if (format == SampleFmt::FLOAT_32) {
            // Wav files are expecting floats between -1 and 1.
            float sample = x / 12;
            // Pretend the 32bit float is actually just four 8 bit bytes
            uint8_t const* casted = reinterpret_cast<uint8_t const *>(&sample);
            bytes.push_back(casted[0]);
            bytes.push_back(casted[1]);
            bytes.push_back(casted[2]);
            bytes.push_back(casted[3]);
        } else {
            assert(not "Unknown format");
        }
        // Append the bytes vector to byte_buffer
        byte_buffer.insert(byte_buffer.end(), bytes.begin(), bytes.end());
    }
    return byte_buffer;
}

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

    size_t num_samples = 0;
    std::vector<float> buffer; // keeps track of the currently written samples
    bool recording = false;
    SampleFmt format = SampleFmt::FLOAT_32;
    int num_channels = 1;

    Recorder() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    float getSeconds() {
        return num_samples / engineGetSampleRate();
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
        printf("Recording %d channels %f, %s\n", num_channels, engineGetSampleRate(), toString(format));
        buffer.clear();
        num_samples = 0;
    }

    if (recording and not button_on) {
        int i = 0;
        std::string filename = "recording0.wav";

        do {
            i++;
            filename = "recording" + std::to_string(i) + ".wav";
        }
        while (file_exists(filename));
        std::vector<uint8_t> byte_buffer = to_bytes(format, buffer);
        writewav(&byte_buffer[0], format, num_channels, num_samples, engineGetSampleRate(), filename.c_str());
        printf("Wrote %s\n", filename.c_str());
    }

    if (recording) {
        // Note: While this buffer always has floats, the actual conversion is done at write time
        if (num_channels == 1) {
            buffer.push_back(inputs[Recorder::LEFT_INPUT].value);
        } else {
            // When stereo, the samples are interleaved.
            buffer.push_back(inputs[Recorder::LEFT_INPUT].value);
            buffer.push_back(inputs[Recorder::RIGHT_INPUT].value);
        }
        num_samples += 1;
    }

    recording = button_on;
}

struct RecordButton : SVGSwitch, ToggleSwitch {
    RecordButton() {
        addFrame(SVG::load(assetPlugin(plugin, "res/DarkButton.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/LightButton.svg")));
    }
};

struct FormatItem : MenuItem {
    SampleFmt format;
    Recorder *recorder;
    FormatItem(SampleFmt format, Recorder *recorder) {
        this->format = format;
        this->text = toString(format);
        this->recorder = recorder;
        this->rightText = CHECKMARK(format == recorder->format);
    }

    // on click, set the Recorder to use the selected format.
    void onAction(EventAction &e) override {
        recorder->format = this->format;
    }
};

struct RecordingDisplay : LedDisplay {
    char msg[8] = {0};
    bool recording = false;

    LedDisplayChoice *timerText = nullptr;
    LedDisplaySeparator *separator = nullptr;
    LedDisplayChoice *formatChoice = nullptr;
    Recorder *recorder;

    RecordingDisplay() {
        box.size = Vec(35, 44);
        Vec pos = Vec(0, 0);
        timerText = Widget::create<LedDisplayChoice>(pos);
        timerText->textOffset = Vec(3, 14);
        timerText->box.size = Vec(35, 22);
        pos = timerText->box.getBottomLeft();
        setSeconds(0);
        addChild(timerText);

        separator = Widget::create<LedDisplaySeparator>(pos);
        separator->box.size.x = box.size.x;
        addChild(separator);

        formatChoice = Widget::create<LedDisplayChoice>(pos);
        formatChoice->textOffset = Vec(3, 14);
        formatChoice->box.size = Vec(35, 22);
        pos = formatChoice->box.getBottomLeft();
        addChild(formatChoice);
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

        timerText->text = msg;
    }

    void setDisplay(SampleFmt format) {
        switch(format) {
        case SampleFmt::PCM_U8:
            formatChoice->text = "8 USI";
            break;
        case SampleFmt::PCM_S16:
            formatChoice->text = "16 SI";
            break;
        case SampleFmt::FLOAT_32:
            formatChoice->text = "32 FL";
            break;
        default:
            formatChoice->text = "ERROR";
            break;
        }
    }

    void onMouseDown(EventMouseDown &e) override {
        Menu *menu = gScene->createMenu();
        menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Format"));
        if (recorder->recording) {
            menu->addChild(MenuItem::create("Can't change formats while recording!"));
        } else {
            menu->addChild(new FormatItem(SampleFmt::PCM_U8, recorder));
            menu->addChild(new FormatItem(SampleFmt::PCM_S16, recorder));
            menu->addChild(new FormatItem(SampleFmt::FLOAT_32, recorder));
        }
    }
};

struct RecorderWidget : ModuleWidget {
    Recorder *recorder;
    RecordingDisplay *display;
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

    display = Widget::create<RecordingDisplay>(Vec(5, 140));
    display->recorder = module;
    addChild(display);

    addParam(ParamWidget::create<RecordButton>(Vec(7.5, 200), module, Recorder::RECORD_BUTTON, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<CKSS>(Vec(15, 260), module, Recorder::MONO_STEREO, 0.0f, 1.0f, 0.0f));
}

void RecorderWidget::step() {
    display->recording = recorder->recording;
    display->setSeconds(recorder->getSeconds());
    display->setDisplay(recorder->format);
}

Model *modelRecorder = Model::create<Recorder, RecorderWidget>("MicroTools", "Recorder", "Recorder", RECORDING_TAG);
