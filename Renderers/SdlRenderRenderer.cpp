#include "Renderers/SdlRenderRenderer.hpp"

SDLRenderRenderer::SDLRenderRenderer(SDL_Window* aWindow, const char* aRendererBackend)
    : Renderer{ aWindow }
    , mRendererBackend{ aRendererBackend }
{
    mName = "SDLRenderer { ";
    mName += aRendererBackend;
    mName += " }";


    mRenderer = SDL_CreateRenderer(mWindow, mRendererBackend);

    if (nullptr == mRenderer) {
        printf("SDL Error: %s\n", SDL_GetError());
        __debugbreak();
    }
}

void SDLRenderRenderer::Initialize()
{
}

void SDLRenderRenderer::Update()
{
    SDL_SetRenderDrawColor(mRenderer, mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a);
    SDL_RenderClear(mRenderer);

    int x = 0, y = 0;
    SDL_GetWindowSize(mWindow, &x, &y);
    int width_center = x / 2;
    int height_center = y / 2;

    SDL_SetRenderDrawColor(mRenderer, mTriangleColor .r, mTriangleColor.g, mTriangleColor.b, mTriangleColor .a);
    SDL_FRect rect{ width_center - (width_center / 2), height_center - (height_center / 2), width_center, height_center };
    SDL_RenderFillRect(mRenderer, &rect);
    if (!SDL_RenderPresent(mRenderer)) {
        printf("SDL Error: %s\n", SDL_GetError());
    }
}

void SDLRenderRenderer::Resize(unsigned int aWidth, unsigned int aHeight)
{

}

const char* SDLRenderRenderer::Name()
{
    return mName.c_str();
}

std::unique_ptr<Renderer> CreateSdlRenderRenderer(SDL_Window* aWindow, const char* aRenderBackend)
{
    return std::unique_ptr<Renderer>(new SDLRenderRenderer(aWindow, aRenderBackend));
}
