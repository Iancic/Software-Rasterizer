#pragma once
#include "Math.hpp"
#include "Program.hpp"
#include "Model.hpp"
#include "glTF_Mesh.hpp"
#include <algorithm>

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
		case VK_ESCAPE:
			PostQuitMessage(0);
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
	void PlotTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Mesh& mesh, const int matIndex);

	std::vector<Triangle> CullBackFaces(std::vector<float3>& viewVertices, std::vector<Triangle>& triangles);
	bool BackFacing(const Triangle& triangle, std::vector<float3>& viewVerts);

	void RenderOBJ(Model* targetModel, std::vector<Vertex>& vertices, std::vector<Triangle>& triangles, const mat4& MV, const mat4& proj);
	void RenderGLTF(std::vector<glTF_Mesh::Vertex*> vertices, std::vector<glTF_Mesh::Triangle*> triangles, const mat4& MV, const mat4& proj);

	class Camera
	{
		public:

		float3 eye = { 0.0f, 0.0f, -5.0f };
		float3 target = { 0.0f, 0.0f, -1.0f };
		float3 up = { 0.0f, 1.0f, 0.0f };
		float3 topLeft = float3(-aspect, 1, 0);
		float3 topRight = float3(aspect, 1, 0);
		float3 bottomLeft = float3(-aspect, -1, 0);
		float3 forward;

		float zNear = 0.1f;

		float aspect = float(SCREEN_WIDTH) / float(SCREEN_HEIGHT);
		float fovRad = 60 * (3.14159f / 180.0f);
	};

	float rotationIncrement = 0.f, scaleIncrement = 1.f;

	Camera mainCam;
	glTF_Mesh *testBunny = nullptr;
	Model* testCharacter = nullptr;
	Model* testFloor = nullptr;

	std::vector<Model*> models;
};