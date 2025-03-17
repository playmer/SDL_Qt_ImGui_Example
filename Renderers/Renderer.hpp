#pragma once

#include <array>
#include <memory>
#include "SDL3/SDL.h"

class Renderer;

class Dx11Renderer;
std::unique_ptr<Renderer> CreateDx11Renderer(SDL_Window*);
class Dx12Renderer;
std::unique_ptr<Renderer> CreateDx12Renderer(SDL_Window*);
class OpenGL3_3Renderer;
std::unique_ptr<Renderer> CreateOpenGL3_3Renderer(SDL_Window*);
class VkRenderer;
std::unique_ptr<Renderer> CreateVkRenderer(SDL_Window*);
class SdlRenderRenderer;
std::unique_ptr<Renderer> CreateSdlRenderRenderer(SDL_Window*, const char* aRenderBackend);
class SdlGpuRenderer;
std::unique_ptr<Renderer> CreateSdlGpuRenderer(SDL_Window*, const char* aRenderBackend);

enum class RendererType
{
	Dx11Renderer,
	Dx12Renderer,
	OpenGL3_3Renderer,
	VkRenderer,
    SdlRenderRenderer,
    SdlGpuRenderer
};

std::unique_ptr<Renderer> CreateRenderer(SDL_Window* aWindow, RendererType aType, const char* aRenderBackend);


struct color
{
    Uint8 r, g, b, a;
};

class Renderer
{
public:
	Renderer(SDL_Window* aWindow)
		: mWindow{ aWindow }
	{

	}

	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Resize(unsigned int aWidth, unsigned int aHeight) = 0;
    virtual const char* Name() = 0;
	
    color mClearColor = {0x00, 0x00, 0xFF, 0xFF};
    color mTriangleColor = {0xFF, 0x00, 0x00, 0xFF};

	static const std::array<float, 9> TriangleVerts;
	static constexpr unsigned int cVertexStride = 3 * sizeof(float);
	static constexpr unsigned int cVertexOffset = 0;
	static constexpr unsigned int cVertexCount = 3;

protected:
	SDL_Window* mWindow = nullptr;
};
