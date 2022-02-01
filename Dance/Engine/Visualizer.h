#pragma once

#include "framework.h"

class Engine;

class Visualizer {
public:
	virtual ~Visualizer() {}

	virtual void Render(Engine const& engine) = 0;
	virtual void Update(StepTimer const& timer) = 0;
};

class FourierVisualizer : public Visualizer {
public:
	static std::unique_ptr<FourierVisualizer> Load() {
		return std::make_unique<FourierVisualizer>();
	}

	virtual void Render(Engine const& engine) override {}
	virtual void Update(StepTimer const& timer) override {}
};
