#include "MicroTools.hpp"
#include "dsp/digital.hpp"

#define NUM_CHANNELS 8
// Voltages for the knobs
#define DEFAULT_VOLTAGE 1.0f
#define MAX_VOLTAGE 5.0f
#define MIN_VOLTAGE -5.0f

struct PushButton : Module {
    enum ParamIds {
        LIGHT_PARAM = 0,
        KNOB_PARAM = LIGHT_PARAM + NUM_CHANNELS,
        NUM_PARAMS = NUM_CHANNELS * 2,
    };
    enum OutputIds {
        NUM_OUTPUTS = NUM_CHANNELS
    };
    enum LightIds {
        NUM_LIGHTS = NUM_CHANNELS
    };

    bool state[NUM_CHANNELS]; // tracks state of mute buttons
    float knob[NUM_CHANNELS]; // tracks value of knobs

    PushButton() : Module(NUM_PARAMS, 0, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void PushButton::step() {
    for (int i = 0; i < NUM_CHANNELS; i++) {
        float out = 0.0f;
        state[i] = params[i + LIGHT_PARAM].value > 0 ? true : false;
        knob[i] = params[i + KNOB_PARAM].value;
        if (state[i]) {
            out = knob[i];
        }
        outputs[i].value = out;
        lights[i].setBrightness(state[i] ? 0.9f : 0.0f);
    }
}

template <typename BASE>
struct TriggerLight : BASE {
    TriggerLight() {
        this->box.size = mm2px(Vec(6.0f, 6.0f));
    }
};

struct PushButtonWidget : ModuleWidget {
    PushButtonWidget(PushButton *module);
};

PushButtonWidget::PushButtonWidget(PushButton *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/PushButton.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

    for (int i = 0; i < NUM_CHANNELS; i++) {
        // Handles on click stuff
        const float Y_DIST = 38.5;
        const float KNOB_HEIGHT = 14.5;
        const float BEZEL_HEIGHT = 11;
        const float LIGHT_HEIGHT = 9;
        const float PORT_HEIGHT = 12.5;
        addParam(ParamWidget::create<RoundBlackKnob>(Vec(9.507, 64 - KNOB_HEIGHT + Y_DIST * i), module, PushButton::KNOB_PARAM + i, MIN_VOLTAGE, MAX_VOLTAGE, DEFAULT_VOLTAGE));
        addParam(ParamWidget::create<LEDBezel>(Vec(50, 64 - BEZEL_HEIGHT + Y_DIST * i), module, PushButton::LIGHT_PARAM + i, 0.0f, 1.0f, 0.0f));
        addChild(ModuleLightWidget::create<TriggerLight<GreenLight>>(Vec(52, 64 - LIGHT_HEIGHT + Y_DIST * i), module, i));
        addOutput(Port::create<PJ301MPort>(Vec(84, 64 - PORT_HEIGHT + Y_DIST * i), Port::OUTPUT, module, i));
    }
}


Model *modelPushButton = Model::create<PushButton, PushButtonWidget>("MicroTools", "Push Button", "Push Button", UTILITY_TAG);
