#pragma once

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
//#include <Metal/shared_ptr.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "Renderers/Renderer.hpp"



class MetalRenderer : public Renderer
{
public:
	MetalRenderer(SDL_Window* aWindow);

	void Initialize() override;
	void Update() override;
	void Resize(unsigned int aWidth, unsigned int aHeight) override;

private:
	SDL_Renderer* mRenderer;
    CA::MetalLayer* mSwapChain;
};
