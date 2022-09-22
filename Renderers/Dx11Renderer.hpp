#pragma once

#define NOMINMAX
#include <d3d11.h>

#include <wrl.h>

#include "Renderers/Renderer.hpp"

class DX11Renderer : public Renderer
{
public:
	DX11Renderer(SDL_Window* aWindow);

	void Initialize() override;
	void Update() override;
	void Resize(unsigned int aWidth, unsigned int aHeight) override;

private:
	void CleanupRenderTarget();
	void CreateRenderTarget();

	Microsoft::WRL::ComPtr<ID3D11Device> mD3DDevice = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> mD3DDeviceContext = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout = nullptr;
	
	Microsoft::WRL::ComPtr<ID3DBlob> mVertexShaderBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> mPixelShaderBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer = nullptr;
	
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> mRasterState = nullptr;
	

	ID3D11RenderTargetView* mMainRenderTargetView = nullptr;
};