#include <Renderers/Renderer.hpp>

const std::array<float, 9> Renderer::TriangleVerts = {
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	0.0f,  0.5f, 0.0f
};  



std::unique_ptr<Renderer> CreateRenderer(SDL_Window* aWindow, RendererType aType, const char* aRenderBackend)
{
	switch (aType)
	{
		#ifdef WIN32
			case RendererType::Dx11Renderer: return CreateDx11Renderer(aWindow);
			case RendererType::Dx12Renderer: return CreateDx12Renderer(aWindow);
		#endif // WIN32

		case RendererType::OpenGL3_3Renderer: return CreateOpenGL3_3Renderer(aWindow);
		#ifdef HAVE_VULKAN
			case RendererType::VkRenderer: return CreateVkRenderer(aWindow);
		#endif // HAVE_VULKAN
            case RendererType::SdlRenderRenderer: return CreateSdlRenderRenderer(aWindow, aRenderBackend);
		default: printf("No renderer of type %d", (int)aType);  return nullptr;
	}
}
