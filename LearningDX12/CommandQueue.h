// Trey Mongeon
// Summer 2026
// Learning DX12

#pragma once

#include <d3d12.h>  // For ID3D12CommandQueue, ID3D12Device2, and ID3D12Fence
#include <wrl.h>    // For Microsoft::WRL::ComPtr

#include <cstdint>  // For uint64_t
#include <queue>    // For std::queue

//Wrapper for ID3D12CommandQueue
class CommandQueue
{
public:

	CommandQueue(Microsoft::WRL::ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);

	virtual ~CommandQueue();

	// Get an available command list from the command queue
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> GetCommandList();

	uint64_t ExecuteCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList);

	uint64_t Signal();
	bool IsFenceComplete(uint64_t fenceValue);
	void WaitForFenceValue(uint64_t fenceValue);
	void Flush();

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetD3D12CommandQueue() const;

protected:

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator);

private:

	// Used to keep track of command allocators that are "in flight"
	struct CommandAllocatorEntry
	{
		uint64_t fenceValue;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	};

	D3D12_COMMAND_LIST_TYPE m_CommandListType;
	Microsoft::WRL::ComPtr<ID3D12Device2> m_d3d12Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_d3d12CommandQueue;

	Microsoft::WRL::ComPtr<ID3D12Fence> m_d3d12Fence;
	HANDLE m_FenceEvent;
	uint64_t m_FenceValue;

	std::queue<CommandAllocatorEntry> m_CommandAllocatorQueue;
	std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>> m_CommandListQueue;
};