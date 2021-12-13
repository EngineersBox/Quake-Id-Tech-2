#pragma once

#ifndef ENGINE_H
#define ENGINE_H

#include "window.hpp"
#include "../view/camera.hpp"
#include "timing/highResTimer.hpp"

class Engine : public Window {
protected:
	HighResTimer* timer;

	virtual void cycle(float deltaTime);
	virtual void onInit() {}
	virtual Camera* getCamera() { return nullptr;  }
	//virtual World* getWorld() { return nullptr;  }
	virtual void tickInput(float deltaTime);
	void swapBuffers() { SDL_GL_SwapWindow(this->window); }
public:
	Engine(): timer(new HighResTimer) {}
	Engine(const char* name, bool fscreen, int width, int height) :
		Window(name, fscreen, width, height),
		timer(new HighResTimer) {}
	~Engine() {}
	int eventLoop();
};

#endif // ENGINE_H