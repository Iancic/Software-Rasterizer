#pragma once
#include "Math.hpp"
#include "Program.hpp"

static uint32_t* framebuffer = nullptr;
static BITMAPINFO bitmapInfo;

struct InputState 
{
	bool moveForward = false;
	bool moveBackward = false;
	bool moveLeft = false;
	bool moveRight = false;
};

static InputState input;

static inline LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		if (framebuffer) {
			StretchDIBits(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
				0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
				framebuffer, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
		}

		EndPaint(hwnd, &ps);
		return 0;
	}

	case WM_KEYDOWN:
		switch (wParam) 
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case 'W': case VK_UP:
			input.moveForward = true;
			break;
		case 'S': case VK_DOWN:
			input.moveBackward = true;
			break;
		case 'A': case VK_LEFT:
			input.moveLeft = true;
			break;
		case 'D': case VK_RIGHT:
			input.moveRight = true;
			break;
		}
		return 0;

	case WM_KEYUP:
		switch (wParam) 
		{
		case 'W': case VK_UP:
			input.moveForward = false;
			break;
		case 'S': case VK_DOWN:
			input.moveBackward = false;
			break;
		case 'A': case VK_LEFT:
			input.moveLeft = false;
			break;
		case 'D': case VK_RIGHT:
			input.moveRight = false;
			break;
		}
		return 0;
	}

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
};

class Game : public Program
{
public:
	Game(const char* title) : Program(title) {}
	~Game() {}

	void Init() override;
	void Shutdown() override;
	void Render() override;
	void HandleEvents() override;
	void HandleInput() override;
	void Update() override;

	// Windowing:
    HWND window;
	HDC hdc;
	bool createWindow(int widht, int height, const wchar_t* title);
    void UpdateWindow();
    
private: 

	// Rendering:
	float* depthBuffer = nullptr;

	// Time
	std::chrono::high_resolution_clock::time_point previousTime;

	float Interpolate(float a0, float a1, float w);

	// Rendering Utilities:
	void Clear(uint32_t color);
	void Plot(uint32_t color, int pX, int pY);
	void Line(uint32_t color, float x1, float y1, float x2, float y2);
	void TriangleWireframe(uint32_t color, float x1, float y1, float x2, float y2, float x3, float y3);
	void PlotTriangle(const uint32_t& color1, float x1, float y1, const uint32_t& color2, float x2, float y2, const uint32_t& color3, float x3, float y3);
	
	void DrawSpansBetweenEdges(const Edge& e1, const Edge& e2);
	void DrawSpan(const Span& span, int y);

	std::vector<Triangle> CullBackFaces(std::vector<float3>& viewVertices, std::vector<Triangle>& triangles);
	bool BackFacing(const Triangle& triangle, std::vector<float3>& viewVerts);
	
	void RenderObject(uint32_t color, std::vector<Vertex>& vertices, std::vector<Triangle>& triangles, const mat4& MVP, const mat4& MV);
	
	// Scene:
	struct Camera
	{
		float3 eye = { 0.0f, 5.0f, 10.0f };
		float3 target = { 0.0f, 0.0f, 0.0f };
		float3 up = { 0.0f, 1.0f, 0.0f };
	};

	Camera mainCam;

	struct Cube
	{
		const float size = 1.f;

		std::vector<Vertex> cubeVer =
		{
			{{-size, -size,  size}, 0xFFFFFFFF}, // 0 - Front Bottom Left
			{{ size, -size,  size}, 0xFFFFFFFF}, // 1 - Front Bottom Right
			{{ size,  size,  size}, 0xFFFFFFFF}, // 2 - Front Top Right
			{{-size,  size,  size}, 0xFFFFFFFF}, // 3 - Front Top Left

			{{-size, -size, -size}, 0xFFFFFFFF}, // 4 - Back Bottom Left
			{{ size, -size, -size}, 0xFFFFFFFF}, // 5 - Back Bottom Right
			{{ size,  size, -size}, 0xFFFFFFFF}, // 6 - Back Top Right
			{{-size,  size, -size}, 0xFFFFFFFF}  // 7 - Back Top Left
		};

		std::vector<Triangle> cubeTri =
		{
			// Front (+Z)
			{ 0xFFFFFFFF, { 0, 2, 1 } },
			{ 0xFFFFFFFF, { 0, 3, 2 } },

			// Right (+X)
			{ 0xFFFFFFFF, { 1, 2, 6 } },
			{ 0xFFFFFFFF, { 1, 6, 5 } },

			// Back (-Z)
			{ 0xFFFFFFFF, { 5, 6, 7 } },
			{ 0xFFFFFFFF, { 5, 7, 4 } },

			// Left (-X)
			{ 0xFFFFFFFF, { 4, 7, 3 } },
			{ 0xFFFFFFFF, { 4, 3, 0 } },

			// Top (+Y)
			{ 0xFFFFFFFF, { 3, 7, 6 } },
			{ 0xFFFFFFFF, { 3, 6, 2 } },

			// Bottom (-Y)
			{ 0xFFFFFFFF, { 4, 0, 1 } },
			{ 0xFFFFFFFF, { 4, 1, 5 } }
		};
	};

	// Project all to screen
	Vertex projected[8];

	float angle = 0.f;
	Cube cube1;
};