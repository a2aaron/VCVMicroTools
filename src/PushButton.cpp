#include "MicroTools.hpp"
#include "dsp/digital.hpp"

#define NUM_CHANNELS 8

struct PushButton : Module {
    enum ParamIds {
        NUM_PARAMS = NUM_CHANNELS
    };
    enum OutputIds {
        NUM_OUTPUTS = NUM_CHANNELS
    };
    enum LightIds {
        NUM_LIGHTS = NUM_CHANNELS * 2
    };

    bool state[NUM_CHANNELS]; // tracks state of mute buttons
    SchmittTrigger toneTrigger[NUM_CHANNELS];

    PushButton() : Module(NUM_PARAMS, 0, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};

void PushButton::step() {
    for (int i = 0; i < NUM_PARAMS; i++) {
        float out = 0.0f;
        state[i] = params[i].value > 0 ? true : false;
        if (state[i]) {
            out = 1.0f;
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
    setPanel(SVG::load(assetPlugin(plugin, "res/8vert.svg")));

    addChild(Widget::create<ScrewSilver>(Vec(15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(15, 365)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 30, 365)));

    // Handles on click stuff
    addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 18.165)), module, 0, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 28.164)), module, 1, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 38.164)), module, 2, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 48.165)), module, 3, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 58.164)), module, 4, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 68.165)), module, 5, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 78.164)), module, 6, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 88.164)), module, 7, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 98.165)), module, 8, 0.0f, 1.0f, 0.0f));
    addParam(ParamWidget::create<LEDBezel>(mm2px(Vec(16.57, 108.166)), module, 9, 0.0f, 1.0f, 0.0f));

    // The green light that displays when you click it
    addChild(ModuleLightWidget::create<TriggerLight<GreenLight>>(mm2px(Vec(17.32, 18.915)), module, 0));
    addChild(ModuleLightWidget::create<TriggerLight<GreenLight>>(mm2px(Vec(17.32, 28.916)), module, 1));
    addChild(ModuleLightWidget::create<TriggerLight<GreenLight>>(mm2px(Vec(17.32, 38.915)), module, 2));
    addChild(ModuleLightWidget::create<TriggerLight<GreenLight>>(mm2px(Vec(17.32, 48.915)), module, 3));
    addChild(ModuleLightWidget::create<TriggerLight<GreenLight>>(mm2px(Vec(17.32, 58.916)), module, 4));
    addChild(ModuleLightWidget::create<TriggerLight<GreenLight>>(mm2px(Vec(17.32, 68.916)), module, 5));
    addChild(ModuleLightWidget::create<TriggerLight<GreenLight>>(mm2px(Vec(17.32, 78.915)), module, 6));
    addChild(ModuleLightWidget::create<TriggerLight<GreenLight>>(mm2px(Vec(17.32, 88.916)), module, 7));
    addChild(ModuleLightWidget::create<TriggerLight<GreenLight>>(mm2px(Vec(17.32, 98.915)), module, 8));
    addChild(ModuleLightWidget::create<TriggerLight<GreenLight>>(mm2px(Vec(17.32, 108.915)), module, 9));

    // Output ports
    addOutput(Port::create<PJ301MPort>(Vec(86.393, 50.397), Port::OUTPUT, module, 0));
    addOutput(Port::create<PJ301MPort>(Vec(86.393, 88.842), Port::OUTPUT, module, 1));
    addOutput(Port::create<PJ301MPort>(Vec(86.393, 127.283), Port::OUTPUT, module, 2));
    addOutput(Port::create<PJ301MPort>(Vec(86.393, 165.728), Port::OUTPUT, module, 3));
    addOutput(Port::create<PJ301MPort>(Vec(86.393, 204.173), Port::OUTPUT, module, 4));
    addOutput(Port::create<PJ301MPort>(Vec(86.393, 242.614), Port::OUTPUT, module, 5));
    addOutput(Port::create<PJ301MPort>(Vec(86.393, 281.059), Port::OUTPUT, module, 6));
    addOutput(Port::create<PJ301MPort>(Vec(86.393, 319.504), Port::OUTPUT, module, 7));
}


Model *modelPushButton = Model::create<PushButton, PushButtonWidget>("MicroTools", "Push Button", "Push Button", UTILITY_TAG);
