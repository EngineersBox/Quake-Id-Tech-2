#include "g_main.hpp"

#define GL_SILENCE_DEPRECATION

int main(int, char**) {
    Core::Logger::init();

    Core::Engine* engine = new Core::Engine("Test Game");
    engine->init();
    engine->tick();

	return 0;
}
