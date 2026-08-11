#include "Tutorial2.h"

#include "Application.h"
#include "CommandQueue.h"
#include "Helpers.h"
#include "Window.h"
#include "Game.h"

#include "wrl.h"
using namespace Microsoft::WRL;

#include <d3dx12.h>
#include <d3dcompiler.h>

#include <algorithm>

#pragma comment(lib, "dxguid.lib") // THIS IS REALLY COOL I DIDNT KNOW YOU COULD BASICALLY JUST SAY INCLUDE THIS .lib

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

using namespace DirectX;

struct VertexPosColor
{
	XMFLOAT3 Position;
	XMFLOAT3 Color;
};

static VertexPosColor g_Vertices[8] = {
	{XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f)},	// 0
	{XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f)},		// 1
	{XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 0.0f)},		// 2
	{XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)},		// 3
	{XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f)},		// 4
	{XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f)},		// 5
	{XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f)},		// 6
	{XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 1.0f)}		// 7
};

static WORD g_Indices[36] = {
	0, 1, 2,	0, 2, 3,
	4, 6, 5,	4, 7, 6,
	4, 5, 1,	4, 1, 0,
	3, 2, 6,	3, 6, 7,
	1, 5, 6,	1, 6, 2,
	4, 0, 3,	4, 3, 7
};

Tutorial2::Tutorial2(const std::wstring& name, int width, int height, bool vSync)
	: super(name, width, height, vSync), 
	m_ScissorRect(CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX)),
	m_Viewport(CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height))),
	m_FOV(45.0),
	m_ContentLoaded(false),
	m_IndexBufferView(), m_IndexBuffer(), m_DepthBuffer(), m_DSVHeap(), m_RootSignature(), m_VertexBufferView(),
	m_ProjectionMatrix(), m_ViewMatrix(), m_ModelMatrix()
{
}

void Tutorial2::UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
	ID3D12Resource** pDestinationResource,
	ID3D12Resource** pIntermediateResource,
	size_t numElements, size_t elementSize, const void* bufferData,
	D3D12_RESOURCE_FLAGS flags)
{
	auto device = Application::Get().GetDevice();

	size_t bufferSize = numElements * elementSize;

	// Create a committed resource for the GPU resource in a default heap
	CD3DX12_HEAP_PROPERTIES destHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC destBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

	ThrowIfFailed(device->CreateCommittedResource(
		&destHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&destBufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(pDestinationResource)));

	if (bufferData)
	{
		CD3DX12_HEAP_PROPERTIES intermediateHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC intermediateBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);
		
		ThrowIfFailed(device->CreateCommittedResource(
			&intermediateHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&intermediateBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(pIntermediateResource)));

		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = bufferData;
		subresourceData.RowPitch = bufferSize;
		subresourceData.SlicePitch = subresourceData.RowPitch;

		UpdateSubresources(commandList.Get(),
			*pDestinationResource, *pIntermediateResource,
			0, 0, 1, &subresourceData);
	}

}
/*
	1. Upload the Vertex buffer data for a cube mesh
		Create a vertex buffer view
	2. Upload the index buffer data for a cube mesh
		Create an index buffer view
	3. Create a descriptor heap for the depth-stencil view
	4. Load the vertex shader
	5. Load the pixel shader
	6. Create the input layout for the vertex shader
	7. Create a root signature
	8. Create a graphics pipeline state object (PSO)
	9. Create a depth buffer
*/
bool Tutorial2::LoadContent()
{
	auto device = Application::Get().GetDevice();
	auto copyCommandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
	auto commandList = copyCommandQueue->GetCommandList();

	// Upload vertex buffer data
	ComPtr<ID3D12Resource> intermediateVertexBuffer;
	UpdateBufferResource(commandList.Get(),
		&m_VertexBuffer, &intermediateVertexBuffer,
		_countof(g_Vertices), sizeof(VertexPosColor), g_Vertices);

	// Create the vertex buffer view
	m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.SizeInBytes = sizeof(g_Vertices);
	m_VertexBufferView.StrideInBytes = sizeof(VertexPosColor);

	// Upload index buffer data
	ComPtr<ID3D12Resource> intermediateIndexBuffer;
	UpdateBufferResource(commandList.Get(),
		&m_IndexBuffer, &intermediateIndexBuffer,
		_countof(g_Indices), sizeof(WORD), g_Indices);

	// Create index buffer view
	m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
	m_IndexBufferView.SizeInBytes = sizeof(g_Indices);



	// Create the descriptor heap for the depth stencil view
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DSVHeap)));

	// Load vertex shader, it was compiled by visual studio during the build process?
	ComPtr<ID3DBlob> vertexShaderBlob;
	ThrowIfFailed(D3DReadFileToBlob(L"VertexShader.cso", &vertexShaderBlob));

	// Load the pixel shader, it was compiled by visual studio i guess
	ComPtr<ID3DBlob> pixelShaderBlob;
	ThrowIfFailed(D3DReadFileToBlob(L"PixelShader.cso", &pixelShaderBlob));

	// Create the vertex input layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	// Create the root signature
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	// Allow input layout and deny unnecessary access to certain pipeline stages
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	// A single 32 bit constant root parameter that is used by the vertex shader
	CD3DX12_ROOT_PARAMETER1 rootParameters[1];
	rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
	rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

	// Serialize the root signature
	ComPtr<ID3DBlob> rootSignatureBlob;
	ComPtr<ID3DBlob> errorBlob;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
		featureData.HighestVersion, &rootSignatureBlob, &errorBlob));

	// CREATE THE ROOT SIGNATURE
	ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));

	// Pipeline state stream for creating the PSO (pipeline state object)
	struct PipelineStateStream
	{
		CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
		CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
		CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
		CD3DX12_PIPELINE_STATE_STREAM_VS VS;
		CD3DX12_PIPELINE_STATE_STREAM_PS PS;
		CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT DSVFormat;
		CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
	} pipelineStateStream;

	// Need to define the number of render targets and the render target formats before creating the PSO
	D3D12_RT_FORMAT_ARRAY rtvFormats = {};
	rtvFormats.NumRenderTargets = 1;
	rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

	// The PSO can now be described
	pipelineStateStream.pRootSignature = m_RootSignature.Get();
	pipelineStateStream.InputLayout = { inputLayout, _countof(inputLayout) };
	pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
	pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
	pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	pipelineStateStream.RTVFormats = rtvFormats;

	D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
	sizeof(PipelineStateStream), &pipelineStateStream
	};
	ThrowIfFailed(device->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_PipelineState)));

	auto fenceValue = copyCommandQueue->ExecuteCommandList(commandList);
	copyCommandQueue->WaitForFenceValue(fenceValue);

	m_ContentLoaded = true;

	// Resize/Create the depth buffer
	ResizeDepthBuffer(GetClientWidth(), GetClientHeight());

	return true;
}

void Tutorial2::ResizeDepthBuffer(int width, int height)
{
	if (m_ContentLoaded)
	{
		// Flush any GPU commands that might be referencing the depth buffer
		Application::Get().Flush();

		width = std::max(1, width);
		height = std::max(1, height);

		auto device = Application::Get().GetDevice();

		// Resize screen dependent resources
		D3D12_CLEAR_VALUE optimizedClearValue = {};
		optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		optimizedClearValue.DepthStencil = { 1.0f, 0 };

		// Descriptors for the depth buffer basically
		CD3DX12_HEAP_PROPERTIES depthBufferHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC depthBufferResourceDescription = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height,
			1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

		// Create a depth buffer, it is a 2D texture
		ThrowIfFailed(device->CreateCommittedResource(
			&depthBufferHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthBufferResourceDescription,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&optimizedClearValue,
			IID_PPV_ARGS(&m_DepthBuffer)
		));

		// Update the depth stencil view
		D3D12_DEPTH_STENCIL_VIEW_DESC dsv = {};
		dsv.Format = DXGI_FORMAT_D32_FLOAT;
		dsv.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsv.Texture2D.MipSlice = 0;
		dsv.Flags = D3D12_DSV_FLAG_NONE;

		device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsv,
			m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
	}
}

void Tutorial2::OnResize(ResizeEventArgs& e)
{
	if (e.Width != GetClientWidth() || e.Height != GetClientHeight())
	{
		super::OnResize(e);

		m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(e.Width), static_cast<float>(e.Height));

		ResizeDepthBuffer(e.Width, e.Height);
	}
}

void Tutorial2::OnUpdate(UpdateEventArgs& e)
{
	static uint64_t frameCount = 0;
	static double totalTime = 0.0;

	super::OnUpdate(e);

	totalTime += e.ElapsedTime;
	frameCount++;

	if (totalTime > 1.0)
	{
		double fps = frameCount / totalTime;

		// Output the fps to the visual studio output
		char buffer[512];
		sprintf_s(buffer, "FPS: %f\n", fps);
		OutputDebugStringA(buffer);

		frameCount = 0;
		totalTime = 0.0;
	}

	// Update model matrix
	float angle = static_cast<float>(e.TotalTime * 90.0);
	const XMVECTOR rotAxis = XMVectorSet(0.0f, 1.0f, 1.0f, 0.0f);
	m_ModelMatrix = XMMatrixRotationAxis(rotAxis, XMConvertToRadians(angle));

	// Update view matrix
	const XMVECTOR eyePos = XMVectorSet(0, 0, -10, 1);
	const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
	const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
	m_ViewMatrix = XMMatrixLookAtLH(eyePos, focusPoint, upDirection);

	// Update the projection matrix
	float aspectRatio = GetClientWidth() / static_cast<float>(GetClientHeight());
	m_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(m_FOV), aspectRatio, 0.1f, 100.0f);
}

// Transition a resource
void Tutorial2::TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
	Microsoft::WRL::ComPtr<ID3D12Resource> resource,
	D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		resource.Get(),
		beforeState, afterState);

	commandList->ResourceBarrier(1, &barrier);
}

void Tutorial2::ClearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
	D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor)
{
	commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void Tutorial2::ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
	D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth)
{
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
}

void Tutorial2::OnRender(RenderEventArgs& e)
{
	super::OnRender(e);

	auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	auto commandList = commandQueue->GetCommandList();

	UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
	auto backBuffer = m_pWindow->GetCurrentBackBuffer();
	auto rtv = m_pWindow->GetCurrentRenderTargetView();
	auto dsv = m_DSVHeap->GetCPUDescriptorHandleForHeapStart();

	// Clear the render targets
	{
		TransitionResource(commandList, backBuffer,
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };

		ClearRTV(commandList, rtv, clearColor);
		ClearDepth(commandList, dsv);
	}

	/*
	* The ID3D12GraphicsCommandList::SetPipelineState method is used to bind the
	PSO to the rendering pipeline. This single function call is all that is required
	to set the various programmable shader stages, input layout, depth-stencil state,
	and blend state (to name a few things that are defined in the PSO).
	*/
	commandList->SetPipelineState(m_PipelineState.Get());
	/*
	the root signature must be explicitly set on the command list using the
	ID3D12GraphicsCommandList::SetGraphicsRootSignature method before binding
	any resources (CBV, UAV, SRV, or Sampler descriptors)
	*/
	commandList->SetGraphicsRootSignature(m_RootSignature.Get());

	commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	commandList->IASetIndexBuffer(&m_IndexBufferView);

	commandList->RSSetViewports(1, &m_Viewport);
	commandList->RSSetScissorRects(1, &m_ScissorRect);

	commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

	// Update Model, View, Projection matrix
	XMMATRIX mvpMatrix = XMMatrixMultiply(m_ModelMatrix, m_ViewMatrix);
	mvpMatrix = XMMatrixMultiply(mvpMatrix, m_ProjectionMatrix);
	commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &mvpMatrix, 0);

	// RENDER THE SHITTT
	commandList->DrawIndexedInstanced(_countof(g_Indices), 1, 0, 0, 0);


	// Present
	{
		TransitionResource(commandList, backBuffer,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		m_FenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);

		currentBackBufferIndex = m_pWindow->Present();

		commandQueue->WaitForFenceValue(m_FenceValues[currentBackBufferIndex]);
	}

}

void Tutorial2::OnKeyPressed(KeyEventArgs& e)
{
	super::OnKeyPressed(e);

	switch (e.Key)
	{
	case KeyCode::Escape:
		Application::Get().Quit(0);
		break;

	case KeyCode::Enter:
		if (e.Alt)
		{
	case KeyCode::F11:
		m_pWindow->ToggleFullscreen();
		break;
		}

	case KeyCode::V:
		m_pWindow->ToggleVSync();
		break;
	}
}

void Tutorial2::OnMouseWheel(MouseWheelEventArgs& e)
{
	m_FOV -= e.WheelDelta;
	m_FOV = std::max(m_FOV, 12.0f);
	m_FOV = std::min(m_FOV, 90.0f);

	char buffer[256];
	sprintf_s(buffer, "FOV: %f\n", m_FOV);
	OutputDebugStringA(buffer);
}

bool Tutorial2::UnloadContent()
{
	return false;
}









