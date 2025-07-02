#include "Game.hpp"

void Game::Init()
{
	woodTex = new Texture("Assets/container.jpg");
	teapot = LoadMesh("Assets/Teapot/Teapot.obj");

	previousTime = std::chrono::high_resolution_clock::now();

	std::wstring title = L"Doom 1993 Path Traced";
	createWindow(SCREEN_WIDTH, SCREEN_HEIGHT, title.c_str());

	// Init framebuffer and depth buffer
	depthBuffer = new float[SCREEN_WIDTH * SCREEN_HEIGHT];
	framebuffer = new uint32_t[SCREEN_WIDTH * SCREEN_HEIGHT];
	ZeroMemory(&bitmapInfo, sizeof(bitmapInfo));
	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = SCREEN_WIDTH;
	bitmapInfo.bmiHeader.biHeight = -SCREEN_HEIGHT; // top-down
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
}

void Game::Update()
{
	rotation += 0.01f;

	mainCam.BuildViewPlane();

	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsed = currentTime - previousTime;
	float deltaTime = elapsed.count(); // deltaTime in seconds
	previousTime = currentTime;

	if (input.moveForward)
		mainCam.eye.z += 3.f * deltaTime;
	else if (input.moveBackward)
		mainCam.eye.z -= 3.f * deltaTime;
	if (input.moveLeft)
		mainCam.eye.x -= 3.f * deltaTime;
	else if (input.moveRight)
		mainCam.eye.x += 3.f * deltaTime;

	HandleInput();
}

bool Game::createWindow(int widht, int height, const wchar_t* title)
{
	HINSTANCE instance = GetModuleHandleA(0);

	WNDCLASSW wc = {};
	wc.hInstance = instance;
	wc.hIcon = LoadIcon(instance, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = title;
	wc.lpfnWndProc = WindowProc;

	if (!RegisterClassW(&wc)) return false;

	int dwStyle = WS_OVERLAPPEDWINDOW;

	window = CreateWindowExW(0, title, title, dwStyle, 100, 100, widht, height, NULL, NULL, instance, NULL);

	if (!window) return false;

	ShowWindow(window, SW_SHOW);
	return true;
}

void Game::UpdateWindow()
{
	MSG msg;

	while (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			isRunning = false;

		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

void Game::Clear(uint32_t color)
{
	for (int x = 0; x < SCREEN_WIDTH; x++)
	{
		for (int y = 0; y < SCREEN_HEIGHT; y++)
		{
			int index = y * SCREEN_WIDTH + x;
			depthBuffer[index] = 1.f;
			framebuffer[index] = color;
		}
	}
}

void Game::Plot(uint32_t color, int pX, int pY)
{
	if (pX >= 0 && pX < SCREEN_WIDTH && pY >= 0 && pY < SCREEN_HEIGHT) framebuffer[SCREEN_WIDTH * pY + pX] = color;
}

// Bresenham's line algorithm
void Game::Line(uint32_t color, float x1, float y1, float x2, float y2)
{
	float dx = x2 - x1;
	float dy = y2 - y1;

	float steps = std::max(std::abs(dx), std::abs(dy));
	float xInc = dx / steps;
	float yInc = dy / steps;

	float x = x1;
	float y = y1;

	for (int i = 0; i <= steps; i++)
	{
		Plot(color, static_cast<int>(x), static_cast<int>(y));
		x += xInc;
		y += yInc;
	}
}

float Game::Interpolate(float a0, float a1, float w)
{
	return a0 * (1 - w) + a1 * w;
}

void Game::TriangleWireframe(uint32_t color, float x1, float y1, float x2, float y2, float x3, float y3)
{
	Line(color, x1, y1, x2, y2);
	Line(color, x2, y2, x3, y3);
	Line(color, x1, y1, x3, y3);
}

std::vector<Triangle> Game::CullBackFaces(std::vector<float3>& viewVertices, std::vector<Triangle>& triangles)
{
	std::vector<Triangle> result;
	for (const auto& tri : triangles)
	{
		if (BackFacing(tri, viewVertices))
			result.push_back(tri);
	}
	return result;
}

bool Game::BackFacing(const Triangle& triangle, std::vector<float3>& viewVerts)
{
	const float3& p0 = viewVerts[triangle.indices[0]];
	const float3& p1 = viewVerts[triangle.indices[1]];
	const float3& p2 = viewVerts[triangle.indices[2]];

	float3 edge1 = p1 - p0;
	float3 edge2 = p2 - p0;
	float3 normal = normalize(Cross(edge1, edge2));

	// Direction from camera (0,0,0 in view space) to triangle
	float3 toCamera = normalize(float3{ -p0.x, -p0.y, -p0.z }); // View space eye is at origin

	// Backface if facing away from camera
	return Dot(normal, toCamera) < 0.f;
}

void Game::ClipTriangle()
{
}

void Game::PlotTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
{
	// Bounding box
	int minX = std::max(0, (int)std::floor(std::min({ v0.position.x, v1.position.x, v2.position.x })));
	int maxX = std::min(SCREEN_WIDTH - 1, (int)std::ceil(std::max({ v0.position.x, v1.position.x, v2.position.x })));
	int minY = std::max(0, (int)std::floor(std::min({ v0.position.y, v1.position.y, v2.position.y })));
	int maxY = std::min(SCREEN_HEIGHT - 1, (int)std::ceil(std::max({ v0.position.y, v1.position.y, v2.position.y })));

	float2 p0 = { v0.position.x, v0.position.y };
	float2 p1 = { v1.position.x, v1.position.y };
	float2 p2 = { v2.position.x, v2.position.y };

	// Triangle area (for barycentric)
	float area = (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);
	if (area == 0.0f) return; // Degenerate

	for (int y = minY; y <= maxY; ++y)
	{
		for (int x = minX; x <= maxX; ++x)
		{
			float2 p = { x + 0.5f, y + 0.5f };

			// Compute barycentric weights
			float w0 = ((p1.x - p.x) * (p2.y - p.y) - (p2.x - p.x) * (p1.y - p.y)) / area;
			float w1 = ((p2.x - p.x) * (p0.y - p.y) - (p0.x - p.x) * (p2.y - p.y)) / area;
			float w2 = 1.0f - w0 - w1;

			if (w0 < 0 || w1 < 0 || w2 < 0) continue; // Outside

			// Interpolate depth (perspective correct)
			float invZ = 1.0f / v0.position.z * w0 +
				1.0f / v1.position.z * w1 +
				1.0f / v2.position.z * w2;

			float z = 1.0f / invZ;

			int index = y * SCREEN_WIDTH + x;
			if (z >= depthBuffer[index]) continue;

			// Interpolate UVs (perspective correct)
			float u = (v0.uv.x / v0.position.z) * w0 +
				(v1.uv.x / v1.position.z) * w1 +
				(v2.uv.x / v2.position.z) * w2;

			float v = (v0.uv.y / v0.position.z) * w0 +
				(v1.uv.y / v1.position.z) * w1 +
				(v2.uv.y / v2.position.z) * w2;

			u *= z;
			v *= z;

			u = std::clamp(u, 0.0f, 0.999f);
			v = std::clamp(v, 0.0f, 0.999f);

			// Sample texture
			int texX = int(u * woodTex->GetWidth());
			int texY = int(v * woodTex->GetHeight());
			int texIndex = (texY * woodTex->GetWidth() + texX) * woodTex->GetChannels();

			uint8_t r = woodTex->GetTexel(texIndex + 0);
			uint8_t g = woodTex->GetTexel(texIndex + 1);
			uint8_t b = woodTex->GetTexel(texIndex + 2);

			// Write to framebuffer
			depthBuffer[index] = z;
			Plot(MakeColor(r, g, b, 255), x, y);
		}
	}
}

/*
void Game::RenderObject(uint32_t color, std::vector<Vertex>& vertices, std::vector<Triangle>& triangles, const mat4& MVP, const mat4& MV)
{
	// HERE WE GO TO MODEL AND VIEW SPACE
	std::vector<float3> viewVertices;
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		float4 viewPos = vertices[i].pos4() * MV;

		// Now we have the objects in view space and we check for near clipping
		// This means if viewPos.z < nearClipPlane, pop back triangle


		viewVertices.push_back({ viewPos.x, viewPos.y, viewPos.z });  // here we store the new triangle
	}

	// HERE WE CLIP AGAINST CAMERA NEAR

	std::vector<Vertex> projected(vertices.size());
	std::vector<float4> clip(vertices.size());

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		float4 c = vertices[i].pos4() * MVP;
		if (std::abs(c.w) < 1e-5f) continue; // avoid divide-by-zero
		float ndcX = c.x / c.w;
		float ndcY = c.y / c.w;
		float ndcZ = c.z / c.w;

		float screenX = (ndcX + 1.0f) * 0.5f * SCREEN_WIDTH;
		float screenY = (1.0f - (ndcY + 1.0f) * 0.5f) * SCREEN_HEIGHT;
		float zDepth = (ndcZ + 1.0f) * 0.5f;

		projected[i] = Vertex{{screenX, screenY, zDepth}, vertices[i].uv};
	}

	auto culledTriangles = CullBackFaces(viewVertices, triangles);

	for (size_t i = 0; i < culledTriangles.size(); ++i)
	{
		auto& triangle = culledTriangles[i];

		const Vertex& v0 = projected[triangle.indices[0]];
		const Vertex& v1 = projected[triangle.indices[1]];
		const Vertex& v2 = projected[triangle.indices[2]];

		PlotTriangle(v0, v1, v2);
		//TriangleWireframe(0xFFFFFFFF, v0.position.x, v0.position.y, v1.position.x, v1.position.y, v2.position.x, v2.position.y);
	}
}
*/
void Game::RenderObject(uint32_t color, std::vector<Vertex>& vertices, std::vector<Triangle>& triangles, const mat4& MV, const mat4& proj)
{
	auto interpolate = [](const Vertex& a, const Vertex& b, float t) -> Vertex {
		Vertex v;
		v.position = a.position + (b.position - a.position) * t;
		v.uv = a.uv + (b.uv - a.uv) * t;
		return v;
		};

	std::vector<Vertex> viewVerts;
	viewVerts.reserve(vertices.size());
	for (const auto& v : vertices)
	{
		float4 viewPos = v.pos4() * MV;
		Vertex out = v;
		out.position = { viewPos.x, viewPos.y, viewPos.z };
		viewVerts.push_back(out);
	}

	std::vector<Triangle> clippedTris;
	std::vector<Vertex> clippedVerts;
	for (auto& triangle : triangles)
	{
		const Vertex& v0 = viewVerts[triangle.indices[0]];
		const Vertex& v1 = viewVerts[triangle.indices[1]];
		const Vertex& v2 = viewVerts[triangle.indices[2]];

		float z0 = v0.position.z;
		float z1 = v1.position.z;
		float z2 = v2.position.z;

		std::vector<std::pair<Vertex, float>> inside;
		std::vector<std::pair<Vertex, float>> outside;

		if (z0 >= mainCam.zNear) inside.push_back({ v0, z0 }); else outside.push_back({ v0, z0 });
		if (z1 >= mainCam.zNear) inside.push_back({ v1, z1 }); else outside.push_back({ v1, z1 });
		if (z2 >= mainCam.zNear) inside.push_back({ v2, z2 }); else outside.push_back({ v2, z2 });

		if (inside.empty()) continue;

		if (outside.empty())
		{
			size_t base = clippedVerts.size();
			clippedVerts.push_back(v0);
			clippedVerts.push_back(v1);
			clippedVerts.push_back(v2);
			Triangle t1;
			t1.indices[0] = base + 0;
			t1.indices[2] = base + 1;
			t1.indices[1] = base + 2;
			clippedTris.push_back(t1);
		}
		else if (inside.size() == 1)
		{
			Vertex A = inside[0].first;
			Vertex B = interpolate(A, outside[0].first, (mainCam.zNear - inside[0].second) / (outside[0].second - inside[0].second));
			Vertex C = interpolate(A, outside[1].first, (mainCam.zNear - inside[0].second) / (outside[1].second - inside[0].second));
			size_t base = clippedVerts.size();
			clippedVerts.push_back(A);
			clippedVerts.push_back(B);
			clippedVerts.push_back(C);
			Triangle t1;
			t1.indices[0] = base + 0;
			t1.indices[2] = base + 1;
			t1.indices[1] = base + 2;
			clippedTris.push_back(t1);
		}
		else if (inside.size() == 2)
		{
			Vertex A = inside[0].first;
			Vertex B = inside[1].first;
			Vertex C = interpolate(A, outside[0].first, (mainCam.zNear - inside[0].second) / (outside[0].second - inside[0].second));
			Vertex D = interpolate(B, outside[0].first, (mainCam.zNear - inside[1].second) / (outside[0].second - inside[1].second));
			size_t base = clippedVerts.size();
			clippedVerts.push_back(A);
			clippedVerts.push_back(B);
			clippedVerts.push_back(C);
			clippedVerts.push_back(D);
			Triangle t1;
			Triangle t2;
			t1.indices[0] = base + 0;
			t1.indices[2] = base + 1;
			t1.indices[1] = base + 2;
			clippedTris.push_back(t1);
			t2.indices[0] = base + 0;
			t2.indices[2] = base + 3;
			t2.indices[1] = base + 2;
			clippedTris.push_back(t2);
		}
	}

	std::vector<float3> viewPositions;
	viewPositions.reserve(clippedVerts.size());
	for (const auto& v : clippedVerts)
		viewPositions.push_back(v.position);

	auto culledTriangles = CullBackFaces(viewPositions, clippedTris);

	std::vector<Vertex> projected(clippedVerts.size());
	for (size_t i = 0; i < clippedVerts.size(); ++i)
	{
		float4 c = clippedVerts[i].pos4() * proj;
		if (std::abs(c.w) < 1e-5f) continue;
		float ndcX = c.x / c.w;
		float ndcY = c.y / c.w;
		float ndcZ = c.z / c.w;
		float screenX = (ndcX + 1.0f) * 0.5f * SCREEN_WIDTH;
		float screenY = (1.0f - (ndcY + 1.0f) * 0.5f) * SCREEN_HEIGHT;
		float zDepth = (ndcZ + 1.0f) * 0.5f;
		projected[i] = { {screenX, screenY, zDepth}, clippedVerts[i].uv };
	}

	for (const auto& tri : culledTriangles)
	{
		const Vertex& v0 = projected[tri.indices[0]];
		const Vertex& v1 = projected[tri.indices[1]];
		const Vertex& v2 = projected[tri.indices[2]];
		PlotTriangle(v0, v1, v2);
	}
}

float3 Game::Trace(Ray& ray, const mat4& modelMat)
{



	return (ray.T == 1e30f) ? float3{ 0.f, 255.f, 0.f } : float3{ 255.f, 0.f, 0.f };
}

void Game::IntersectTri(Ray& ray, const Tri& tri)
{
	const float3 edge1 = tri.vertex1 - tri.vertex0;
	const float3 edge2 = tri.vertex2 - tri.vertex0;
	const float3 h = Cross(ray.D, edge2);
	const float a = Dot(edge1, h);
	if (a > -0.0001f && a < 0.0001f) return; // ray parallel to triangle
	const float f = 1 / a;
	const float3 s = ray.O - tri.vertex0;
	const float u = f * Dot(s, h);
	if (u < 0 || u > 1) return;
	const float3 q = Cross(s, edge1);
	const float v = f * Dot(ray.D, q);
	if (v < 0 || u + v > 1) return;
	const float t = f * Dot(edge2, q);
	if (t > 0.0001f) ray.T = std::min(ray.T, t);
}

void Game::Render()
{
	UpdateWindow();
	
	Clear(0x00000000);

	mat4 model = (mat::Translate(0.f, -4.5f, 16.f) + mat::Scale(1.f, 1.f, 1.f)) * mat::Rotate(0.1f, 1.f, 0.1f, rotation);
	mat4 view = mat::LookAt(mainCam.eye, mainCam.eye + mainCam.target, mainCam.up);
	mat4 proj = mat::Perspective(mainCam.fovRad, mainCam.aspect, 1.0f, 500.0f);

	mat4 MV = view * model;
	mat4 MVP = proj * view * model;

	RenderObject(0xFFFFFFFF, teapot.vertices, teapot.triangle, MV, proj);

	/*
	if (gameState.rasterized == true) 
	{
		
	}
	else if (gameState.raytraced == true)
	{
		for (int y = 0; y < SCREEN_HEIGHT; y++)
		{
			for (int x = 0; x < SCREEN_WIDTH; x++)
			{
				Ray tracedRay = mainCam.GetPrimaryRay(x, y);
				float3 trace = Trace(tracedRay, model);
				uint32_t pixel = MakeColor(int(trace.x), int(trace.y), int(trace.z), 255);
				Plot(pixel, x, y);
			}
		}
	}
	*/

	InvalidateRect(window, nullptr, FALSE);
}

void Game::HandleEvents()
{

}

void Game::HandleInput()
{

}

void Game::Shutdown()
{

}