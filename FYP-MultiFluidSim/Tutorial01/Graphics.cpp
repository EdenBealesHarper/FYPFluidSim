#include "Graphics.h"

//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT Graphics::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"FYPMultiFluidSim";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    g_viewWidth = 1024;
    g_viewHeight = 768;
    
    g_hInst = hInstance;
    RECT rc = { 0, 0, g_viewWidth, g_viewHeight };


    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    g_hWnd = CreateWindow(L"FYPMultiFluidSim", L"FYP Multi-Fluid Simulation",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
        nullptr);
    if (!g_hWnd)
        return E_FAIL;

    ShowWindow(g_hWnd, nCmdShow);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
            pErrorBlob->Release();
        }
        return hr;
    }
    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT Graphics::InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(g_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

        if (hr == E_INVALIDARG)
        {
            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
            hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
        }

        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
        return hr;

    // Create swap chain
    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
    if (dxgiFactory2)
    {
        // DirectX 11.1 or later
        hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1));
        if (SUCCEEDED(hr))
        {
            (void)g_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1));
        }

        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.Width = width;
        sd.Height = height;
        sd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;//  DXGI_FORMAT_R16G16B16A16_FLOAT;////DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;

        hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1);
        if (SUCCEEDED(hr))
        {
            hr = g_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain));
        }

        dxgiFactory2->Release();
    }
    else
    {
        // DirectX 11.0 systems
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = g_hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
    }

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

    dxgiFactory->Release();

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
        return hr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &g_pDepthStencil);
    if (FAILED(hr))
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
    if (FAILED(hr))
        return hr;

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports(1, &vp);

    hr = InitMesh();
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to initialise mesh.", L"Error", MB_OK);
        return hr;
    }

    hr = InitWorld(width, height);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to initialise world.", L"Error", MB_OK);
        return hr;
    }

    float ypos = 10.0f;
    float zpos = 10.0f;

    for (int i = 0; i < NUMCUBES; i++)
    {
        if (zpos <= -10.0f)
        {
            zpos = 10.0f;
            ypos -= 0.5f;
        }
        DrawableGameObject* dgo = new DrawableGameObject();
        dgo->initMesh(g_pd3dDevice, g_pImmediateContext);
        dgo->setPosition(XMFLOAT3(0.0f, ypos, zpos));
        g_GameObjectArr.push_back(dgo);

        zpos -= 0.5f;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pImmediateContext);
    ImGui::StyleColorsDark();

    return S_OK;
}

// ***************************************************************************************
// InitMesh
// ***************************************************************************************
HRESULT	Graphics::InitMesh()
{
    LoadShader((WCHAR*)L"shader.fx");

    // Create the constant buffer
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;        // 36 vertices needed for 12 triangles in a triangle list
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = ((sizeof(ConstantBuffer) - 1)|15) + 1;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    HRESULT hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pConstantBuffer);
    if (FAILED(hr))
        return hr;

    // Create the constant buffer
    D3D11_BUFFER_DESC bdPP = {};
    bdPP.Usage = D3D11_USAGE_DEFAULT;
    bdPP.ByteWidth = sizeof(SCREEN_VERTEX) * 4;
    bdPP.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bdPP.CPUAccessFlags = 0;
    bdPP.Usage = D3D11_USAGE_DEFAULT;
    bdPP.ByteWidth = ((sizeof(ConstantBuffer) - 1) | 15) + 1;
    bdPP.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bdPP.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer(&bdPP, nullptr, &g_pConstantBufferPP);
    if (FAILED(hr))
        return hr;

    // Create the material constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(MaterialPropertiesConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pMaterialConstantBuffer);
    if (FAILED(hr))
        return hr;

    // Create the light constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(LightPropertiesConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pLightConstantBuffer);
    if (FAILED(hr))
        return hr;

    // load and setup textures
    hr = CreateDDSTextureFromFile(g_pd3dDevice, L"Resources\\Brick\\diffuse.dds", nullptr, &g_pTextureRV);
    if (FAILED(hr))
        return hr;

    buildFSQuad();

    sph = new SmoothParticle();

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);

    return hr;
}

// ***************************************************************************************
// InitWorld
// ***************************************************************************************
HRESULT	Graphics::InitWorld(int width, int height)
{
    // Initialize the world matrix
    XMMATRIX TempWorld1 = XMMatrixIdentity();

    UpdateViewMatrix();
    
    XMStoreFloat4x4(g_World1, TempWorld1);
    XMStoreFloat4x4(g_Projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f));

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void Graphics::CleanupDevice()
{
    if (g_pImmediateContext) g_pImmediateContext->ClearState();

    if (g_pConstantBuffer) g_pConstantBuffer->Release();
    if (g_pVertexBuffer) g_pVertexBuffer->Release();
    if (g_pIndexBuffer) g_pIndexBuffer->Release();
    if (g_pVertexLayout) g_pVertexLayout->Release();
    if (g_pVertexShader) g_pVertexShader->Release();
    if (g_pPixelShader) g_pPixelShader->Release();
    if (g_pDepthStencil) g_pDepthStencil->Release();
    if (g_pDepthStencilView) g_pDepthStencilView->Release();
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain1) g_pSwapChain1->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext1) g_pImmediateContext1->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice1) g_pd3dDevice1->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWind, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Graphics::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
      return true;

    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_LBUTTONDOWN:
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        break;
    }
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

        // Note that this tutorial does not handle resizing (WM_SIZE) requests,
        // so we created the window without the resize border.
    case WM_KEYDOWN:
        
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}


//--------------------------------------------------------------------------------------
// Update Variables
//--------------------------------------------------------------------------------------
void Graphics::Update()
{
    // Update our time
    float totalTime = 0.0f;
    if (g_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        totalTime += (float)XM_PI * 0.0125f;
    }
    else
    {
        if (curTime == 0.0f)
        {
            curTime = GetTickCount64();
        }

        prevTime = curTime;
        curTime = GetTickCount64();

        deltaTime = (curTime - prevTime) / 1000.0f;

        /*static ULONGLONG timeStart = 0;
        ULONGLONG timeCur = GetTickCount64();
        if (timeStart == 0)
            timeStart = timeCur;
        totalTime = (timeCur - timeStart) / 1000.0f;

        deltaTime = (timeCur - prevTime)/1000.0f;
        prevTime = timeCur;*/
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Info");

    float fps = 1.0f / deltaTime;

    ImGui::Text("FPS: %f", fps);

    ImGui::Text("Eye Pos = %f, %f, %f", g_EyePosition.x, g_EyePosition.y, g_EyePosition.z);
    ImGui::Text("Eye Looking At = %f, %f, %f", g_At.x, g_At.y, g_At.z);


    ImGui::Checkbox("Simulate?", &doSim);
    for (int i = 0; i < NUMCUBES; i++)
    {
        g_GameObjectArr[i]->setStatic(!doSim);
    }

    sph->CalcNavStok(g_GameObjectArr, 1.0 / 60.0, doSim);

    doSim = true;
    
    for (int i = 0; i < g_GameObjectArr.size(); i++)
    {
        g_GameObjectArr[i]->update(deltaTime);
    }

    ReadKeyboard(deltaTime);

    UpdateCamera();
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Graphics::Render()
{
    RenderDefault();

    ImGui::End();

    ImGui::Render();

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Present our back buffer to our front buffer
    g_pSwapChain->Present(0, 0);
}


void Graphics::UpdateCamera()
{
    XMVECTOR vecUp = XMLoadFloat4(&g_Up);
    XMVECTOR vecAt = XMLoadFloat4(&g_At);
    XMVECTOR vecRight = XMLoadFloat4(&g_Right);
    XMVECTOR vecEye = XMLoadFloat4(&g_EyePosition);
    XMMATRIX view = XMLoadFloat4x4(g_View);

    XMFLOAT4 fUp = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR up = XMLoadFloat4(&fUp);

    XMFLOAT4 fRight = XMFLOAT4(1.0f, 0.0f, 1.0f, 0.0f);
    XMVECTOR right = XMLoadFloat4(&fRight);

    //XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(0.0f, camYaw, camPitch);
    XMMATRIX camRotationMatrix = XMMatrixRotationAxis(vecRight, camPitch) * XMMatrixRotationAxis(up, camYaw);

    vecRight = XMVector3Normalize(XMVector3TransformCoord(vecRight, camRotationMatrix));
    vecAt = XMVector3Normalize(XMVector3TransformCoord(vecAt, camRotationMatrix));

    vecRight = XMVector3Normalize(vecRight);
    vecAt = XMVector3Normalize(vecAt);

    vecUp = XMVector3Cross(vecRight, vecAt);

    XMStoreFloat4(&g_Up, vecUp);
    XMStoreFloat4(&g_At, vecAt);
    XMStoreFloat4(&g_Right, vecRight);

    vecEye += vecRight * right * moveLeftRight;
    vecEye += vecAt * moveForwardBack;
    vecEye += up * moveUpDown;

    XMStoreFloat4(&g_EyePosition, vecEye);

    moveLeftRight = 0.0f;
    moveForwardBack = 0.0f;
    moveUpDown = 0.0f;
    camPitch = 0.0f;
    camYaw = 0.0f;

    view = XMMatrixLookToLH(vecEye, vecAt, vecUp);

    XMStoreFloat4x4(g_View, view);
}


void Graphics::UpdateViewMatrix()
{
    XMVECTOR At = XMLoadFloat4(&g_At);
    XMVECTOR Up = XMLoadFloat4(&g_Up);
    XMVECTOR Eye = XMLoadFloat4(&g_EyePosition);

    // Initialize the view matrix
    XMMATRIX TempView = XMMatrixLookToLH(Eye, At, Up);

    XMStoreFloat4x4(g_View, TempView);
}


void Graphics::ReadKeyboard(float deltaT)
{
    float delta = deltaT * 1000;

    if (GetKeyState(WKEY) & 0x8000)
    {
        moveForwardBack += camSpeed * delta;
    }

    if (GetKeyState(AKEY) & 0x8000)
    {
        moveLeftRight += camSpeed * delta;
    }

    if (GetKeyState(SKEY) & 0x8000)
    {
        moveForwardBack -= camSpeed * delta;
    }

    if (GetKeyState(DKEY) & 0x8000)
    {
        moveLeftRight -= camSpeed * delta;
    }

    if (GetKeyState(VK_SHIFT) & 0x8000)
    {
        moveUpDown -= camSpeed * delta;
    }

    if (GetKeyState(VK_SPACE) & 0x8000)
    {
        moveUpDown += camSpeed * delta;
    }

    
    if (GetKeyState(QKEY) & 0x8000)
    {
        camYaw -= camRotateSpeed * delta;
    }

    if (GetKeyState(EKEY) & 0x8000)
    {
        camYaw += camRotateSpeed * delta;
    }
    
    if (GetKeyState(RKEY) & 0x8000)
    {
        camPitch -= camRotateSpeed * delta;
    }

    if (GetKeyState(FKEY) & 0x8000)
    {
        camPitch += camRotateSpeed * delta;
    }

}


HRESULT Graphics::LoadShader(WCHAR* shaderName)
{
    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;
    HRESULT hr = CompileShaderFromFile(shaderName, "VS", "vs_4_0", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled - Vertex Shader [Basic].  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), &g_pVertexLayout);
    pVSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(shaderName, "PS", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled - Pixel Shader [Basic].  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;


    // Compile the SOLID pixel shader
    pPSBlob = nullptr;
    hr = CompileShaderFromFile(shaderName, "PSSolid", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled - Pixel Shader [Solid].  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the SOLID pixel shader
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShaderSolid);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;
}


void Graphics::buildFSQuad()
{
    svQuad[0].pos = XMFLOAT3(-1.0f, 1.0f, 0.0f);
    svQuad[0].tex = XMFLOAT2(0.0f, 0.0f);

    svQuad[1].pos = XMFLOAT3(1.0f, 1.0f, 0.0f);
    svQuad[1].tex = XMFLOAT2(1.0f, 0.0f);

    svQuad[2].pos = XMFLOAT3(-1.0f, -1.0f, 0.0f);
    svQuad[2].tex = XMFLOAT2(0.0f, 1.0f);

    svQuad[3].pos = XMFLOAT3(1.0f, -1.0f, 0.0f);
    svQuad[3].tex = XMFLOAT2(1.0f, 1.0f);

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SCREEN_VERTEX) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = svQuad;
    g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pScreenQuadVB);
}


void Graphics::RenderDefault() 
{
    //UpdateViewMatrix();
    XMMATRIX TempView = XMLoadFloat4x4(g_View);

    XMMATRIX TempProjection = XMLoadFloat4x4(g_Projection);

    MaterialPropertiesConstantBuffer redPlasticMaterial;
    redPlasticMaterial.Material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    redPlasticMaterial.Material.Specular = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
    redPlasticMaterial.Material.SpecularPower = 32.0f;
    redPlasticMaterial.Material.UseTexture = true;
    g_pImmediateContext->UpdateSubresource(g_pMaterialConstantBuffer, 0, nullptr, &redPlasticMaterial, 0, 0);

    Light light;
    light.Enabled = static_cast<int>(true);
    light.LightType = PointLight;
    light.Color = XMFLOAT4(Colors::White);
    light.SpotAngle = XMConvertToRadians(45.0f);
    light.ConstantAttenuation = 1.0f;
    light.LinearAttenuation = 1;
    light.QuadraticAttenuation = 1;


    // set up the light
    light.Position = g_LightPosition;
    XMVECTOR LightDirection = XMVectorSet(-g_LightPosition.x, -g_LightPosition.y, -g_LightPosition.z, 0.0f);
    LightDirection = XMVector3Normalize(LightDirection);
    XMStoreFloat4(&light.Direction, LightDirection);

    LightPropertiesConstantBuffer lightProperties;
    lightProperties.EyePosition = g_EyePosition;
    lightProperties.Lights[0] = light;
    g_pImmediateContext->UpdateSubresource(g_pLightConstantBuffer, 0, nullptr, &lightProperties, 0, 0);

    //Swap to screen render
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

    // Set the input layout
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    // Clear the back buffer
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);

    // Clear the depth buffer to 1.0 (max depth)
    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Render the cube
    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);

    g_pImmediateContext->PSSetConstantBuffers(1, 1, &g_pMaterialConstantBuffer);
    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);

    g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);

    for (int i = 0; i < g_GameObjectArr.size(); i++)
    {
        // Update variables for the cube
        XMMATRIX mGO = XMLoadFloat4x4(g_GameObjectArr[i]->getTransform());

        ConstantBuffer cb1;
        cb1.mWorld = XMMatrixTranspose(mGO);
        cb1.mView = XMMatrixTranspose(TempView);
        cb1.mProjection = XMMatrixTranspose(TempProjection);
        cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
        cb1.fParalBias = 1.0f;
        cb1.fParalScale = 0.1f;
        g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb1, 0, 0);
        g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
        g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

        g_GameObjectArr[i]->draw(g_pImmediateContext);
    }
}