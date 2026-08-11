// Trey Mongeon
// Summer 2026
// Learning DX12

#pragma once

#include "Game.h"
#include "Window.h"
#include <string>

#include <DirectXMath.h>

class Game;

class Tutorial2 : public Game
{
public:
	using super = Game; //wtf is the point of this bro

	Tutorial2(const std::wstring& name, int width, int height, bool vSync = false);

	// Load shi specific to this demo
	virtual bool LoadContent() override;

	// Unload shi specific to this demo
	virtual bool UnloadContent() override;

protected:

	// Update game logic
	virtual void OnUpdate(UpdateEventArgs& e) override;

	// Render stuff
	virtual void OnRender(RenderEventArgs& e) override;

	// Invoked when key is pressed when window is focused
	virtual void OnKeyPressed(KeyEventArgs& e) override;

	// Invoked when mouse wheel is moved over window
	virtual void OnMouseWheel(MouseWheelEventArgs& e) override;

	virtual void OnResize(ResizeEventArgs& e) override;

private:

	//Helper functions

	// Transition a resource
	void TransitionResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		Microsoft::WRL::ComPtr<ID3D12Resource> resource,
		D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

	// Clear a render target view
	void ClearRTV(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE rtv, FLOAT* clearColor);

	// Clear the depth of a depth stencil view
	void ClearDepth(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		D3D12_CPU_DESCRIPTOR_HANDLE dsv, FLOAT depth = 1.0f);

	// Create as GPU buffer
	void UpdateBufferResource(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList,
		ID3D12Resource** pDestinationResource, ID3D12Resource** pIntermediateResource,
		size_t numElements, size_t elementSize, const void* bufferData,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

	// Resize the depth buffer to match the size of the client area
	void ResizeDepthBuffer(int width, int height);

	uint64_t m_FenceValues[Window::BufferCount] = {};

	// Vertex buffer for the cube in this demo
	Microsoft::WRL::ComPtr<ID3D12Resource> m_VertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	// Index buffer for the cube in this demo
	Microsoft::WRL::ComPtr<ID3D12Resource> m_IndexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

	// Depth buffer
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthBuffer;

	// Descriptor heap for depth buffer
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap;

	// Root signature
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;

	// Pipeline state object
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;

	D3D12_VIEWPORT m_Viewport;
	D3D12_RECT m_ScissorRect;

	float m_FOV;

	DirectX::XMMATRIX m_ModelMatrix;
	DirectX::XMMATRIX m_ViewMatrix;
	DirectX::XMMATRIX m_ProjectionMatrix;

	bool m_ContentLoaded;

};