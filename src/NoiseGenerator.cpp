#include "MicroTools.hpp"
#include "dsp/digital.hpp"
#include <map>
#include <string>

enum NoiseType {
  WHITE_NOISE,
  BROWNIAN,
  TRIGGER,
};

const std::map<NoiseType, std::pair<const char *, const char *>> noiseToStr = {
    {WHITE_NOISE, {"White Noise", "WHITE"}},
    {BROWNIAN, {"Brownian Noise", "BROWN"}},
    {TRIGGER, {"Random Trigger", "TRIGG"}},
};

const std::pair<const char *, const char *> toString(NoiseType format) {
  return noiseToStr.at(format);
}

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
  NoiseType noiseType = NoiseType::WHITE_NOISE;

  json_t *toJson() override {
    json_t *rootJ = json_object();
    json_t *jsonNoiseType = json_integer(noiseType);
    json_object_set_new(rootJ, "noiseType", jsonNoiseType);
    return rootJ;
  }

  void fromJson(json_t *rootJ) override {
    printf("fromJson\n");
    json_t *noiseJ = json_object_get(rootJ, "noiseType");
    noiseType = static_cast<NoiseType>(json_integer_value(noiseJ));
  }
};

void NoiseGenerator::step() {
  // The knob and CV inputs are from 0V to 10V. (note that negative CV inputs
  // will still work) We then multiply the knob and CV inputs by 10V to obtain a
  // nicer distribution of values. This way, clock length can range from 1 to
  // 1000 instead of just 1 to 10. PERIOD_MULTIPLIER will increase the range
  // from 1 to 100,000
  float f_clock_length =
      (params[PERIOD_KNOB].value + inputs[PERIOD_CV].value) * 10.0f;
  f_clock_length *= params[PERIOD_MULTIPLIER].value ? 100.0f : 1.0f;

  int clock_length = max(static_cast<int>(f_clock_length), 1);

  float volume = params[VOLUME_KNOB].value + inputs[VOLUME_CV].value;
  // Every clock_length frames, set the current output to a new sample.
  // TODO: don't use the set frame rate, actually do this via time.
  if (timer % clock_length == 0) {
    switch (noiseType) {
    case NoiseType::WHITE_NOISE: {
      float sample = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      last_out = sample * volume;
      break;
    }
    case NoiseType::BROWNIAN: {
      float sample = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      sample = 2 * (sample - 0.5);
      last_out += sample * volume / 10;

      last_out = min(max(0.0f, last_out), volume);
      break;
    }
    case NoiseType::TRIGGER: {
      float sample = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      last_out = sample > volume / 10.0f ? 1.0f : 0.0f;
      break;
    }
    }
    // printf("%s\n", toString(noiseType).first);
  }
  timer = (timer + 1) % clock_length;

  outputs[0].value = last_out;
}

struct ModeItem : MenuItem {
  NoiseType noiseType;
  NoiseType *noiseDest;
  ModeItem(NoiseType noiseType, NoiseType *noiseDest) {
    this->noiseType = noiseType;
    this->text = toString(noiseType).first;
    this->noiseDest = noiseDest;
    this->rightText = CHECKMARK(noiseType == *noiseDest);
  }

  // on click, set the NoiseGenerator to use the selected format.
  void onAction(EventAction &e) override { *noiseDest = this->noiseType; }
};

struct NoiseTypeDisplay : LedDisplay {
  char msg[8] = {0};

  LedDisplayChoice *displayChoice = nullptr;
  NoiseType *noiseDest; // Where the NoiseType value should get written to upon
                        // clicking a menu item

  NoiseTypeDisplay() {
    box.size = Vec(35, 22);
    Vec pos = Vec(0, 0);
    displayChoice = Widget::create<LedDisplayChoice>(pos);
    displayChoice->textOffset = Vec(3, 14);
    displayChoice->box.size = Vec(35, 22);
    addChild(displayChoice);
  }

  void setDisplay(NoiseType noiseType) {
    displayChoice->text = toString(noiseType).second;
  }

  void onMouseDown(EventMouseDown &e) override {
    Menu *menu = gScene->createMenu();
    menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Noise Type"));
    menu->addChild(new ModeItem(NoiseType::WHITE_NOISE, noiseDest));
    menu->addChild(new ModeItem(NoiseType::BROWNIAN, noiseDest));
    menu->addChild(new ModeItem(NoiseType::TRIGGER, noiseDest));
  }
};

struct NoiseGeneratorWidget : ModuleWidget {
  NoiseTypeDisplay *display;
  NoiseGenerator *noiseGenerator;
  NoiseGeneratorWidget(NoiseGenerator *module);

  void step() override {
    ModuleWidget::step();
    display->setDisplay(noiseGenerator->noiseType);
  }
};

NoiseGeneratorWidget::NoiseGeneratorWidget(NoiseGenerator *module)
    : ModuleWidget(module) {
  setPanel(SVG::load(assetPlugin(plugin, "res/NoiseGenerator.svg")));
  noiseGenerator = module;
  // Mounting Screws
  addChild(Widget::create<ScrewSilver>(Vec(5, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(5, 365)));
  addChild(Widget::create<ScrewSilver>(Vec(70, 0)));
  addChild(Widget::create<ScrewSilver>(Vec(50, 365)));

  const int KNOB_X = 15;
  const int VOLUME_Y = 55;
  const int PERIOD_Y = 105;
  const int CV_X = 58;
  // Volume
  addParam(ParamWidget::create<RoundBlackKnob>(Vec(KNOB_X, VOLUME_Y), module,
                                               NoiseGenerator::VOLUME_KNOB,
                                               0.0f, 12.0f, 1.0f));
  addInput(Port::create<PJ301MPort>(Vec(CV_X, VOLUME_Y), Port::INPUT, module,
                                    NoiseGenerator::VOLUME_CV));
  // Period
  addParam(ParamWidget::create<RoundBlackKnob>(Vec(KNOB_X, PERIOD_Y), module,
                                               NoiseGenerator::PERIOD_KNOB,
                                               0.0f, 10.0f, 0.0f));
  addInput(Port::create<PJ301MPort>(Vec(CV_X, PERIOD_Y), Port::INPUT, module,
                                    NoiseGenerator::PERIOD_CV));
  addParam(ParamWidget::create<CKSS>(Vec(30, 160), module,
                                     NoiseGenerator::PERIOD_MULTIPLIER, 0.0f,
                                     1.0f, 0.0f));
  // Noise Type Display
  display = Widget::create<NoiseTypeDisplay>(Vec(5, 260));
  display->noiseDest = &module->noiseType;
  addChild(display);

  // Output
  addOutput(Port::create<PJ301MPort>(Vec(50, 260), Port::OUTPUT, module, 0));
}

Model *modelNoiseGenerator =
    Model::create<NoiseGenerator, NoiseGeneratorWidget>(
        "MicroTools", "Noise Generator", "Noise Generator", NOISE_TAG);
