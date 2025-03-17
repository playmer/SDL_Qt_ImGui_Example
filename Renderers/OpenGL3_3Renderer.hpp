#pragma once

#include "Renderers/Renderer.hpp"



class OpenGL3_3Renderer : public Renderer
{
public:
	OpenGL3_3Renderer(SDL_Window* aWindow);

	void Initialize() override;
	void Update() override;
	void Resize(unsigned int aWidth, unsigned int aHeight) override;
    virtual const char* Name() override { return "OpenGL3_3Renderer"; };

private:
    unsigned int vertexShader;
    unsigned int fragmentShader;
    unsigned int shaderProgram;
    unsigned int VAO;
    unsigned int VBO;
    SDL_GLContext mGlContext;
};
