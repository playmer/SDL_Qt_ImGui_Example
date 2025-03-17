#include <string>
#include <vector>

#include "SDL3/SDL.h"

#include "Renderers/Renderer.hpp"

class SDLRenderRenderer : public Renderer
{
public:
    SDLRenderRenderer(SDL_Window* aWindow, const char* aRendererBackend);
    virtual void Initialize() override;
    virtual void Update() override;
    virtual void Resize(unsigned int aWidth, unsigned int aHeight) override;
    virtual const char* Name() override;

protected:
    const char* mRendererBackend;
    SDL_Renderer* mRenderer = nullptr;
    std::string mName;
};
