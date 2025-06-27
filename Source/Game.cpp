#include "Game.hpp"

void Game::Init()
{
	cubeTex = new Texture("Assets/container.jpg");
	cube1 = LoadMesh("Assets/Untitled.obj");
	cube2 = LoadMesh("Assets/Teapot/Teapot.obj");

	mainCam.BuildViewPlane();

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
	mainCam.BuildViewPlane();

	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsed = currentTime - previousTime;
	float deltaTime = elapsed.count(); // deltaTime in seconds
	previousTime = currentTime;

	angle += 25.f * deltaTime;

	if (input.moveForward)
		mainCam.eye.z -= 3.f * deltaTime;
	else if (input.moveBackward)
		mainCam.eye.z += 3.f * deltaTime;

	if (input.moveLeft)
		mainCam.eye.x -= 3.f * deltaTime;
	else if (input.moveRight)
		mainCam.eye.x += 3.f * deltaTime;
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
		//if (msg.message == WM_QUIT)
			// = false;

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
	if (pX >= 0 && pX < SCREEN_WIDTH && pY >= 0 && pY < SCREEN_HEIGHT)
	{
		framebuffer[SCREEN_WIDTH * pY + pX] = color;
	}
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
		if (!BackFacing(tri, viewVertices))
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
/*
void Game::PlotTriangle(const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
	Edge edges[3] = {
		Edge((int)v1.position.x, (int)v1.position.y, (int)v2.position.x, (int)v2.position.y,
			 v1.uv.x, v1.uv.y, v2.uv.x, v2.uv.y, v1.position.z, v2.position.z),

		Edge((int)v2.position.x, (int)v2.position.y, (int)v3.position.x, (int)v3.position.y,
			 v2.uv.x, v2.uv.y, v3.uv.x, v3.uv.y, v2.position.z, v3.position.z),

		Edge((int)v3.position.x, (int)v3.position.y, (int)v1.position.x, (int)v1.position.y,
			 v3.uv.x, v3.uv.y, v1.uv.x, v1.uv.y, v3.position.z, v1.position.z)
	};

	int maxLength = 0, longEdge = 0;

	// Find edge with the greatest length in the y axis.
	for (int i = 0; i < 3; i++)
	{
		int length = edges[i].Y2 - edges[i].Y1;
		if (length > maxLength) 
		{
			maxLength = length;
			longEdge = i;
		}
	}

	// Now that we now which is the long edge find the short ones
	int shortEdge1 = (longEdge + 1) % 3;
	int shortEdge2 = (longEdge + 2) % 3;

	// Fill from long edge to short edge 1 and then from long edge to short edge 2
	DrawSpansBetweenEdges(edges[longEdge], edges[shortEdge1]);
	DrawSpansBetweenEdges(edges[longEdge], edges[shortEdge2]);
}
*/

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
			int texX = int(u * cubeTex->GetWidth());
			int texY = int(v * cubeTex->GetHeight());
			int texIndex = (texY * cubeTex->GetWidth() + texX) * cubeTex->GetChannels();

			uint8_t r = cubeTex->GetTexel(texIndex + 0);
			uint8_t g = cubeTex->GetTexel(texIndex + 1);
			uint8_t b = cubeTex->GetTexel(texIndex + 2);

			// Write to framebuffer
			depthBuffer[index] = z;
			Plot(MakeColor(r, g, b, 255), x, y);
		}
	}
}

void Game::DrawSpansBetweenEdges(const Edge& e1, const Edge& e2)
{
	// calculate difference between the y coordinates
	// of the first edge and return if 0
	float e1ydiff = (float)(e1.Y2 - e1.Y1);
	if (e1ydiff == 0.0f)
		return;

	// calculate difference between the y coordinates
	// of the second edge and return if 0
	float e2ydiff = (float)(e2.Y2 - e2.Y1);
	if (e2ydiff == 0.0f)
		return;

	// calculate differences between the x coordinates
	// and colors of the points of the edges
	float e1xdiff = (float)(e1.X2 - e1.X1);
	float e2xdiff = (float)(e2.X2 - e2.X1);

	float eu1diff = e1.U2 - e1.U1;
	float ev1diff = e1.V2 - e1.V1;
	float eu2diff = e2.U2 - e2.U1;
	float ev2diff = e2.V2 - e2.V1;

	// calculate factors to use for interpolation
	// with the edges and the step values to increase
	// them by after drawing each span
	float factor1 = (float)(e2.Y1 - e1.Y1) / e1ydiff;
	float factorStep1 = 1.0f / e1ydiff;
	float factor2 = 0.0f;
	float factorStep2 = 1.0f / e2ydiff;

	// loop through the lines between the edges and draw spans
	for (int y = e2.Y1; y < e2.Y2; y++) 
	{
		float e1x = e1.X1 + e1xdiff * factor1;
		float e2x = e2.X1 + e2xdiff * factor2;

		float e1u = e1.U1 + eu1diff * factor1;
		float e1v = e1.V1 + ev1diff * factor1;
		float e2u = e2.U1 + eu2diff * factor2;
		float e2v = e2.V1 + ev2diff * factor2;
		float ez1diff = e1.Z2 - e1.Z1;
		float ez2diff = e2.Z2 - e2.Z1;

		// Interpolate Z values
		float e1z = e1.Z1 + ez1diff * factor1;
		float e2z = e2.Z1 + ez2diff * factor2;

		Span span((int)e1x, (int)e2x, e1u, e1v, e2u, e2v, e1z, e2z);
		DrawSpan(span, y);

		// increase factors
		factor1 += factorStep1;
		factor2 += factorStep2;
	}
}

void Game::DrawSpan(const Span& span, int y)
{
	if (y < 0 || y >= SCREEN_HEIGHT) return;

	int xdiff = span.X2 - span.X1;
	if (xdiff <= 0) return;

	float factor = 0.0f;
	float factorStep = 1.0f / static_cast<float>(xdiff);

	for (int x = span.X1; x < span.X2; x++)
	{
		if (x < 0 || x >= SCREEN_WIDTH) {
			factor += factorStep;
			continue;
		}

		// === Perspective-correct interpolation ===
		float uOverZ1 = span.U1 / span.Z1;
		float vOverZ1 = span.V1 / span.Z1;
		float uOverZ2 = span.U2 / span.Z2;
		float vOverZ2 = span.V2 / span.Z2;

		float invZ1 = 1.0f / span.Z1;
		float invZ2 = 1.0f / span.Z2;

		float uOverZ = Interpolate(uOverZ1, uOverZ2, factor);
		float vOverZ = Interpolate(vOverZ1, vOverZ2, factor);
		float invZ = Interpolate(invZ1, invZ2, factor);
		float z = 1.0f / invZ;

		float u = uOverZ / invZ;
		float v = vOverZ / invZ;

		// Clamp UVs slightly below 1.0 to avoid texel overflow
		u = std::clamp(u, 0.0f, 0.999f);
		v = std::clamp(v, 0.0f, 0.999f);

		int index = y * SCREEN_WIDTH + x;

		// === Z-buffer test with perspective-correct z ===
		if (z < depthBuffer[index])
		{
			depthBuffer[index] = z;

			int texX = int(u * cubeTex->GetWidth());
			int texY = int(v * cubeTex->GetHeight());
			int texIndex = (texY * cubeTex->GetWidth() + texX) * cubeTex->GetChannels();

			uint8_t r = cubeTex->GetTexel(texIndex + 0);
			uint8_t g = cubeTex->GetTexel(texIndex + 1);
			uint8_t b = cubeTex->GetTexel(texIndex + 2);

			Plot(MakeColor(r, g, b, 255), x, y);
		}

		factor += factorStep;
	}
}

void Game::RenderObject(uint32_t color, std::vector<Vertex>& vertices, std::vector<Triangle>& triangles, const mat4& MVP, const mat4& MV)
{
	std::vector<float3> viewVertices(vertices.size());
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		float4 viewPos = vertices[i].pos4() * MV;
		viewVertices[i] = { viewPos.x, viewPos.y, viewPos.z };
	}

	std::vector<Vertex> projected(vertices.size());
	std::vector<float4> clip(vertices.size());

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		float4 c = vertices[i].pos4() * MVP;
		clip[i] = c; // Store clip vert before division.
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

float3 Game::Trace(Ray& ray)
{
	ray.T = 1e30f;
	for (int i = 0; i < TRI_N; i++) IntersectTri(ray, tri[i]);

	if (ray.T == 1e30f ) return float3{ 0.f, 255.f, 0.f };
	else return float3{ 255.f, 0.f, 0.f };
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

	if (gameState.rasterized == true)
	{
		mat4 model_c1 = (mat::Translate(0.f, 1.f, -16.f) + mat::Scale(0.1f, 0.1f, 0.1f)); //* mat::Rotate(0.f, 0.1f, 0.f, 180 * M_PI / 180.f);
		mat4 model_c2 = (mat::Translate(0.f, -5.f, -15.f) + mat::Scale(0.1f, 0.1f, 0.1f)); //* mat::Rotate(0.f, 0.1f, 0.f, 180 * M_PI / 180.f);

		mat4 view = mat::LookAt(mainCam.eye, mainCam.target, mainCam.up);
		float aspect = float(SCREEN_WIDTH) / float(SCREEN_HEIGHT);
		mat4 proj = mat::Perspective(3.14159f / 3.0f, aspect, 1.0f, 50.0f);

		mat4 MV_c1 = view * model_c1;
		mat4 MVP_c1 = proj * view * model_c1;

		mat4 MV_c2 = view * model_c2;
		mat4 MVP_c2 = proj * view * model_c2;
	
		RenderObject(0xFFFFFFFF, cube2.vertices, cube2.triangle, MVP_c2, MV_c2);
		RenderObject(0xFFFFFFFF, cube1.vertices, cube1.triangle, MVP_c1, MV_c1);
	}
	else if (gameState.raytraced == true)
	{
		for (int y = 0; y < SCREEN_HEIGHT; y++)
		{
			for (int x = 0; x < SCREEN_WIDTH; x++)
			{
				Ray tracedRay = mainCam.GetPrimaryRay(x, y);
				float3 trace = Trace(tracedRay);
				uint32_t pixel = MakeColor(int(trace.x), int(trace.y), int(trace.z), 255);
				Plot(pixel, x, y);
			}
		}
	}

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

Ray Game::Camera::GetPrimaryRay(const float pX, const float pY)
{
	float u = static_cast<float>(pX) / static_cast<float>(SCREEN_WIDTH);
	float v = static_cast<float>(pY) / static_cast<float>(SCREEN_HEIGHT);

	// Interpolate across the screen plane
	float3 screenPoint = topLeft + (topRight - topLeft) * u + (bottomLeft - topLeft) * v;
	return Ray(eye, normalize(screenPoint - eye));
}

void Game::Camera::BuildViewPlane(float fovY, float focalLength)
{
	float aspect = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);

	// Basis vectors
	float3 forward = normalize(target - eye);
	float3 right = normalize(Cross(forward, up));
	float3 upVec = normalize(Cross(right, forward));

	// Convert FOV to view plane height
	float halfHeight = tanf(fovY * 0.5f * 3.14159f / 180.0f);
	float halfWidth = halfHeight * aspect;

	// View plane center
	float3 center = eye + forward * focalLength;

	// Corner positions in world space
	topLeft = center + upVec * halfHeight - right * halfWidth;
	topRight = center + upVec * halfHeight + right * halfWidth;
	bottomLeft = center - upVec * halfHeight - right * halfWidth;
}
