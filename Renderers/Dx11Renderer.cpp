#include <d3dcompiler.h>

#include "Renderers/Dx11Renderer.hpp"

static const char* shader = R"(
/* vertex attributes go here to input to the vertex shader */
struct vs_in {
    float3 position_local : POS;
};

/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct vs_out {
    float4 position_clip : SV_POSITION; // required output of VS
};

vs_out vs_main(vs_in input) {
  vs_out output = (vs_out)0; // zero the memory first
  output.position_clip = float4(input.position_local, 1.0);
  return output;
}

float4 ps_main(vs_out input) : SV_TARGET {
  return float4(1.0, 0.5, 0.2, 1.0); // must return an RGBA colour
}
)";

std::unique_ptr<Renderer> CreateDx11Renderer(SDL_Window* aWindow)
{
    return std::unique_ptr<Renderer>(new DX11Renderer(aWindow));
}

DX11Renderer::DX11Renderer(SDL_Window* aWindow)
	: Renderer{ aWindow }
{
    mWindow = aWindow;
    HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(aWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;

    // If the project is in a debug build, enable the debug layer.
#if !defined(NDEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &mSwapChain, &mD3DDevice, &featureLevel, &mD3DDeviceContext) != S_OK)
    {
        printf("Bad Device/Swapchain");
        
        CleanupRenderTarget();

        mSwapChain = nullptr;
        mD3DDeviceContext = nullptr;
        mD3DDevice = nullptr;
        return;
    }
    
    CreateRenderTarget();


    // Shaders
    ID3DBlob *error_blob = nullptr;

    auto result = D3DCompile(
      shader,
      strlen(shader), 
      "shader.hlsl", 
      nullptr,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      "vs_main",
      "vs_5_0",
      D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
      0,
      mVertexShaderBlob.GetAddressOf(),
      &error_blob);

    if (FAILED(result)) 
    {
        if (error_blob)
        {
            printf( "Pixel Shdaer Error: %s", (char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }

        mVertexShader.Reset();
    }

    result = mD3DDevice->CreateVertexShader(
      mVertexShaderBlob->GetBufferPointer(),
      mVertexShaderBlob->GetBufferSize(),
      nullptr,
      mVertexShader.GetAddressOf() );

    
    if (FAILED(result))
    {
        printf("Couldn't create Vertex Shader\n");
        return;
    }

    result = D3DCompile(
      shader,
      strlen(shader), 
      "shader.hlsl", 
      nullptr,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      "ps_main",
      "ps_5_0",
      D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
      0,
      mPixelShaderBlob.GetAddressOf(),
      &error_blob);

    if (FAILED(result)) 
    {
        if (error_blob)
        {
            printf( "Pixel Shader Error: %s", (char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }

        mPixelShader.Reset();
    }

    result = mD3DDevice->CreatePixelShader(
      mPixelShaderBlob->GetBufferPointer(),
      mPixelShaderBlob->GetBufferSize(),
      nullptr,
      mPixelShader.GetAddressOf());
    
    if (FAILED(result))
    {
        printf("Couldn't create Pixel Shader\n");
        return;
    }

    // Input Layout
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
      { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      /*
      { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      */
    };
    result = mD3DDevice->CreateInputLayout(
      inputElementDesc,
      ARRAYSIZE( inputElementDesc ),
      mVertexShaderBlob->GetBufferPointer(),
      mVertexShaderBlob->GetBufferSize(),
      mInputLayout.GetAddressOf());

    if (FAILED(result))
    {
        printf("oh noooo");
        return;
    }

    // Vertex Buffer
    {
        D3D11_BUFFER_DESC vertex_buff_descr     = {};
        vertex_buff_descr.ByteWidth             = sizeof(float) * TriangleVerts.size();
        vertex_buff_descr.Usage                 = D3D11_USAGE_DEFAULT;
        vertex_buff_descr.BindFlags             = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA sr_data          = { 0 };
        sr_data.pSysMem                         = TriangleVerts.data();
        HRESULT hr = mD3DDevice->CreateBuffer(
            &vertex_buff_descr,
            &sr_data,
            mVertexBuffer.GetAddressOf() );
        
        if (FAILED(result))
        {
            printf("Vertex Buffer failed to be created");
            return;
        }
    }

    // Raster State
    D3D11_RASTERIZER_DESC rasterizerDesc;
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.FrontCounterClockwise = TRUE;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.ScissorEnable = FALSE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;

    mD3DDevice->CreateRasterizerState(&rasterizerDesc, mRasterState.GetAddressOf());
}

void DX11Renderer::Initialize()
{
}

void DX11Renderer::Update()
{
    std::array<float, 4> color = {
        mClearColor.r / 255.f,
        mClearColor.g / 255.f,
        mClearColor.b / 255.f,
        mClearColor.a / 255.f,
    };

    mD3DDeviceContext->RSSetState(mRasterState.Get());

    int width, height;
    SDL_GetWindowSize(mWindow, &width, &height);
    D3D11_VIEWPORT viewport = {
      0.0f,
      0.0f,
      (FLOAT)width,
      (FLOAT)height,
      0.0f,
      1.0f };

    mD3DDeviceContext->RSSetViewports( 1, &viewport );
    mD3DDeviceContext->OMSetRenderTargets(1, &mMainRenderTargetView, nullptr);
    mD3DDeviceContext->ClearRenderTargetView(mMainRenderTargetView, color.data());
    mD3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    mD3DDeviceContext->IASetInputLayout(mInputLayout.Get());
    mD3DDeviceContext->IASetVertexBuffers(
      0,
      1,
      mVertexBuffer.GetAddressOf(),
      &cVertexStride,
      &cVertexOffset);

    mD3DDeviceContext->VSSetShader( mVertexShader.Get(), nullptr, 0);
    mD3DDeviceContext->PSSetShader( mPixelShader.Get(), nullptr, 0 );
    mD3DDeviceContext->Draw( cVertexCount, 0 );

    mSwapChain->Present(1, 0); // Present with vsync
}

void DX11Renderer::Resize(unsigned int aWidth, unsigned int aHeight)
{
    CleanupRenderTarget();
    mSwapChain->ResizeBuffers(0, (UINT)aWidth, (UINT)aHeight, DXGI_FORMAT_UNKNOWN, 0);
    CreateRenderTarget();
}

void DX11Renderer::CleanupRenderTarget()
{
    if (mMainRenderTargetView) 
    { 
        mMainRenderTargetView->Release(); 
        mMainRenderTargetView = nullptr; 
    }
}

void DX11Renderer::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    mSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

    // Should handle the error case more gracefully, but this lets us close the window.
    if (pBackBuffer)
    {
        mD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mMainRenderTargetView);
        pBackBuffer->Release();
    }
}
