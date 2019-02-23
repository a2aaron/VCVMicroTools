#include "MicroTools.hpp"


Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);
	
	p->addModel(modelPushButton);
    p->addModel(modelNoiseGenerator);
    p->addModel(modelRecorder);
}
