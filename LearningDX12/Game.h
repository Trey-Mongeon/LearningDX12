// Trey Mongeon
// Summer 2026
// Learning DX12

#pragma once

#include "Events.h"
#include <memory> // For std::enable_shared_from_this
#include <string> // For std::wstring


// Forward Declarations
class Window;

// The Game class is the abstract base class for DirectX12 demos
class Game : public std::enable_shared_from_this<Game>
{
public:

	// Create the DirextX demo using the specified window dimensions.
	Game(const std::wstring& name, int width, int height, bool vSync);

	virtual ~Game();

	int GetClientWidth() const;

	int GetClientHeight() const;

	// Initialize the DirectX runtime
	virtual bool Initialize();

	// Load content required for the demo
	virtual bool LoadContent() = 0;

	// Unload demo specific content that was loaded in LoadContent()
	virtual bool UnloadContent() = 0;

	// Destroy and resources that are used by the game
	virtual void Destroy();

protected:
	friend class Window;

	// Update the game logic
	virtual void OnUpdate(UpdateEventArgs& e);

	// Render stuff
	virtual void OnRender(RenderEventArgs& e);

	// Invoked by registered window when key is pressed
	virtual void OnKeyPressed(KeyEventArgs& e);

	// Invoked by registered window when key is released
	virtual void OnKeyReleased(KeyEventArgs& e);

	// Invoked when mouse is moved over the window
	virtual void OnMouseMoved(MouseMotionEventArgs& e);

	// Invoked when a mouse button is pressed over the window
	virtual void OnMouseButtonPressed(MouseButtonEventArgs& e);

	// Invoked when a mouse button is released over the window
	virtual void OnMouseButtonReleased(MouseButtonEventArgs& e);

	// Invoked when the mouse wheel is moved
	virtual void OnMouseWheel(MouseWheelEventArgs& e);

	// Invoked when the registered window is resized
	virtual void OnResize(ResizeEventArgs& e);

	// Invoked when the registered window is destroyed
	virtual void OnWindowDestroy();

	std::shared_ptr<Window> m_pWindow;

private:

	std::wstring m_Name;
	int m_Width;
	int m_Height;
	bool m_vSync;

};
