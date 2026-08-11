// Trey Mongeon
// Summer 2026
// Learning DX12

// This class is for creating a window for the application and probably manage other stuff later.
// Uses windows API specific stuff

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <memory>
#include <string>


// Forward declarations

class CommandQueue;
class Window;
class Game;


class Application
{
public:

	// Create the singleton with the application instance handle
	static void Create(HINSTANCE hInst);

	// Destroy the application and all windows created by it
	static void Destroy();

	// Getter
	static Application& Get();

	// Check if VSync-off is supported
	bool IsTearingSupported() const;

	/**
	* Create a new DirectX11 render window instance.
	* @param windowName The name of the window. This name will appear in the title bar of the window. This name should be unique.
	* @param clientWidth The width (in pixels) of the window's client area.
	* @param clientHeight The height (in pixels) of the window's client area.
	* @param vSync Should the rendering be synchronized with the vertical refresh rate of the screen.
	* @param windowed If true, the window will be created in windowed mode. If false, the window will be created full-screen.
	* @returns The created window instance. If an error occurred while creating the window an invalid
	* window instance is returned. If a window with the given name already exists, that window will be
	* returned.
	*/
	std::shared_ptr<Window> CreateRenderWindow(const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync = true);

	// Destroy a window given the name of it
	void DestroyWindow(const std::wstring& winsowName);

	// Destroy a window given the window reference
	void DestroyWindow(std::shared_ptr<Window> window);

	// Find a window by the name
	std::shared_ptr<Window> GetWindowByName(const std::wstring& windowName);

	// Run the application loop and message pump
	// Returns error codes if an error occurred
	int Run(std::shared_ptr<Game> pGame);

	// Request to quit the application and close all windows
	void Quit(int exitCode = 0);

	// Get the Direct3D 12 device
	Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice() const;

	/**
	 * Get a command queue. Valid types are:
	 * - D3D12_COMMAND_LIST_TYPE_DIRECT : Can be used for draw, dispatch, or copy commands.
	 * - D3D12_COMMAND_LIST_TYPE_COMPUTE: Can be used for dispatch or copy commands.
	 * - D3D12_COMMAND_LIST_TYPE_COPY   : Can be used for copy commands.
	 */
	std::shared_ptr<CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) const;

	// Flush all command queues
	void Flush();

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);
	UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

protected:

	// Ctor is protected, creates an application instance
	Application(HINSTANCE hInst);

	// Destroy the application instance and all windows associated with it
	virtual ~Application();

	Microsoft::WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool bUseWarp);
	Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice(Microsoft::WRL::ComPtr <IDXGIAdapter4> adapter);

	bool CheckTearingSupport();

private:

	Application(const Application& rhs) = delete;
	Application& operator=(const Application& other) = delete;

	// Application instance handle that the application was created with
	HINSTANCE m_hInstance;

	Microsoft::WRL::ComPtr<IDXGIAdapter4> m_dxgiAdapter;
	Microsoft::WRL::ComPtr<ID3D12Device2> m_d3d12Device;

	std::shared_ptr<CommandQueue> m_DirectCommandQueue;
	std::shared_ptr<CommandQueue> m_ComputeCommandQueue;
	std::shared_ptr<CommandQueue> m_CopyCommandQueue;

	bool m_TearingSupported;

};