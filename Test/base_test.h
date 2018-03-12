#pragma once

class BaseTest : public InputEventHandler {
protected:
	DX12 dx12;
	InitParams params = {};
public:
	virtual void init(HINSTANCE hInstance, int cmdShow) {
		dx12.init(hInstance, params, this);
		setup();
		dx12.showWindow(cmdShow);
	}
	virtual void shutdown() {};
	virtual void setup() = 0;
	void run() { dx12.run(); shutdown();  }
	virtual void key(int vkCode, bool pressed) {}
	virtual void mouseMove(POINT pos, KeyMod mod) {}
	virtual void mouseButton(int button, POINT pos, KeyMod mod, MouseClick click) {}
	virtual void mouseWheel(int delta, KeyMod mod) {}
};
