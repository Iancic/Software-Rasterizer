#include "Game.hpp"

void Game::Init()
{
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
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsed = currentTime - previousTime;
	float deltaTime = elapsed.count(); // deltaTime in seconds
	previousTime = currentTime;

	angle += 1.f * deltaTime;

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
/*
void Game::TriangleFill(uint32_t color, Vertex v0, Vertex v1, Vertex v2)
{
	int xMin = std::min(std::min(v0.position.x, v1.position.x), v2.position.x);
	int yMin = std::min(std::min(v0.position.y, v1.position.y), v2.position.y);
	int xMax = std::max(std::max(v0.position.x, v1.position.x), v2.position.x);
	int yMax = std::max(std::max(v0.position.y, v1.position.y), v2.position.y);

	// Compute deltas for stepping horizontally and vertically
	int delta_w0_col = (v1.position.y - v2.position.y);
	int delta_w1_col = (v2.position.y - v0.position.y);
	int delta_w2_col = (v0.position.y - v1.position.y);
	int delta_w0_row = (v2.position.x - v1.position.x);
	int delta_w1_row = (v0.position.x - v2.position.x);
	int delta_w2_row = (v1.position.x - v0.position.x);

	int2 vertex0 = int2(v0.position.x, v0.position.y);
	int2 vertex1 = int2(v1.position.x, v1.position.y);
	int2 vertex2 = int2(v2.position.x, v2.position.y);

	float area = cross(vertex0, vertex1, vertex2);

	int bias0 = 0; // isTopLeft(vertex1, vertex2) ? 0 : -1;
	int bias1 = 0; //isTopLeft(vertex2, vertex0) ? 0 : -1;
	int bias2 = 0; //isTopLeft(vertex0, vertex1) ? 0 : -1;

	int2 p0 = { xMin, yMin };
	int w0_row = cross(vertex1, vertex2, p0) + bias0;
	int w1_row = cross(vertex2, vertex0, p0) + bias1;
	int w2_row = cross(vertex0, vertex1, p0) + bias2;

	for (int y = yMin; y <= yMax; y++)
	{
		float w0 = w0_row;
		float w1 = w1_row;
		float w2 = w2_row;
		for (int x = xMin; x <= xMax; x++)
		{
			bool isInside = w0 >= 0 && w1 >= 0 && w2 >= 0;
			if (isInside)
			{
				// Weights inside a triangle.
				float alpha = w0 / static_cast<float>(area);
				float beta = w1 / static_cast<float>(area);
				float gamma = w2 / static_cast<float>(area);
				float z = alpha * v0.position.z + beta * v1.position.z + gamma * v2.position.z;

				int index = y * SCREEN_WIDTH + x;
				if (z < depthBuffer[index]) // Z Buffer Test, otherwise don't compute for fill
				{
					uint32_t c0 = v0.color;
					uint32_t c1 = v1.color;
					uint32_t c2 = v2.color;

					int r = static_cast<int>(((c0 >> 24) & 0xFF) * alpha + ((c1 >> 24) & 0xFF) * beta + ((c2 >> 24) & 0xFF) * gamma);
					int g = static_cast<int>(((c0 >> 16) & 0xFF) * alpha + ((c1 >> 16) & 0xFF) * beta + ((c2 >> 16) & 0xFF) * gamma);
					int b = static_cast<int>(((c0 >> 8) & 0xFF) * alpha + ((c1 >> 8) & 0xFF) * beta + ((c2 >> 8) & 0xFF) * gamma);
					int a = static_cast<int>((c0 & 0xFF) * alpha + (c1 & 0xFF) * beta + (c2 & 0xFF) * gamma);
	
					uint32_t finalColor = (b) | (g << 8) | (r << 16) | (a << 24); // MAKE SURE IT'S BGRA
					depthBuffer[index] = z;
					Plot(finalColor, x, y);
				}
			}
			w0 += delta_w0_col;
			w1 += delta_w1_col;
			w2 += delta_w2_col;
		}

		w0_row += delta_w0_row;
		w1_row += delta_w1_row;
		w2_row += delta_w2_row;
	}
}
*/
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

/*
bool Game::BackFacing(const Triangle& triangle, std::vector<float3>& viewVerts)
{
	const float3& p0 = viewVerts[triangle.indices[0]];
	const float3& p1 = viewVerts[triangle.indices[1]];
	const float3& p2 = viewVerts[triangle.indices[2]];

	float3 edge1 = p1 - p0;
	float3 edge2 = p2 - p0;
	float3 normal = crossF3(edge1, edge2);

	// In view space, camera always looks down -Z axis → viewDir is (0, 0, -1)
	const float3 viewDir = { 0.f, 0.f, -1.f };

	return Dot(normal, viewDir) >= 0.f; // True if backface
}
*/

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
	return Dot(normal, toCamera) > 0.f;
}

void Game::PlotTriangle(const uint32_t& color1, float x1, float y1, const uint32_t& color2, float x2, float y2, const uint32_t& color3, float x3, float y3)
{
	Edge edges[3] = {
		Edge(color1, (int)x1, (int)y1, color2, (int)x2, (int)y2),
		Edge(color2, (int)x2, (int)y2, color3, (int)x3, (int)y3),
		Edge(color3, (int)x3, (int)y3, color1, (int)x1, (int)y1) };

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
	uint32_t e1colordiff = (e1.Color2 - e1.Color1);
	uint32_t e2colordiff = (e2.Color2 - e2.Color1);

	// calculate factors to use for interpolation
	// with the edges and the step values to increase
	// them by after drawing each span
	float factor1 = (float)(e2.Y1 - e1.Y1) / e1ydiff;
	float factorStep1 = 1.0f / e1ydiff;
	float factor2 = 0.0f;
	float factorStep2 = 1.0f / e2ydiff;

	// loop through the lines between the edges and draw spans
	for (int y = e2.Y1; y < e2.Y2; y++) {
		// create and draw span
		Span span(e1.Color1 + (e1colordiff * factor1),
			e1.X1 + (int)(e1xdiff * factor1),
			e2.Color1 + (e2colordiff * factor2),
			e2.X1 + (int)(e2xdiff * factor2));
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
	float factorStep = 1.0f / (float)xdiff;

	// Unpack colors
	uint8_t r1 = (span.Color1 >> 16) & 0xFF;
	uint8_t g1 = (span.Color1 >> 8) & 0xFF;
	uint8_t b1 = (span.Color1 >> 0) & 0xFF;

	uint8_t r2 = (span.Color2 >> 16) & 0xFF;
	uint8_t g2 = (span.Color2 >> 8) & 0xFF;
	uint8_t b2 = (span.Color2 >> 0) & 0xFF;

	for (int x = span.X1; x < span.X2; x++)
	{
		uint8_t r = static_cast<uint8_t>(r1 + (r2 - r1) * factor);
		uint8_t g = static_cast<uint8_t>(g1 + (g2 - g1) * factor);
		uint8_t b = static_cast<uint8_t>(b1 + (b2 - b1) * factor);

		uint32_t color = (b) | (g << 8) | (r << 16) | (255 << 24); // BGRA

		Plot(color, x, y);
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

		float ndcX = c.x / c.w;
		float ndcY = c.y / c.w;
		float ndcZ = c.z / c.w;

		float screenX = (ndcX + 1.0f) * 0.5f * SCREEN_WIDTH;
		float screenY = (1.0f - (ndcY + 1.0f) * 0.5f) * SCREEN_HEIGHT;
		float zDepth = (ndcZ + 1.0f) * 0.5f;

		projected[i] = Vertex{ {screenX, screenY, zDepth}, vertices[i].color };
	}

	auto culledTriangles = CullBackFaces(viewVertices, triangles);

	for (size_t i = 0; i < culledTriangles.size(); ++i)
	{
		auto& triangle = culledTriangles[i];
		bool fullyOutside = false;

		const float4& c0 = clip[triangle.indices[0]];
		const float4& c1 = clip[triangle.indices[1]];
		const float4& c2 = clip[triangle.indices[2]];

		// Clipping:
		/*
		// Following 3 if's discard triangles outside of the view frustrum 
		// x
		{
			int outsidePositive = 0, outsideNegative = 0;

			if (c0.x > c0.w) outsidePositive++;
			if (c1.x > c1.w) outsidePositive++;
			if (c2.x > c2.w) outsidePositive++;

			if (c0.x < -c0.w) outsideNegative++;
			if (c1.x < -c1.w) outsideNegative++;
			if (c2.x < -c2.w) outsideNegative++;

			if (outsidePositive == 3 || outsideNegative == 3)
			{
				fullyOutside = true;
				break;
			}
		}
		if (!fullyOutside) // y
		{
			int outsidePositive = 0, outsideNegative = 0;

			if (c0.y > c0.w) outsidePositive++;
			if (c1.y > c1.w) outsidePositive++;
			if (c2.y > c2.w) outsidePositive++;

			if (c0.y < -c0.w) outsideNegative++;
			if (c1.y < -c1.w) outsideNegative++;
			if (c2.y < -c2.w) outsideNegative++;

			if (outsidePositive == 3 || outsideNegative == 3)
			{
				fullyOutside = true;
				break;
			}
		}
		// z
		if (!fullyOutside) {
			int outsidePositive = 0, outsideNegative = 0;

			if (c0.z > c0.w) outsidePositive++;
			if (c1.z > c1.w) outsidePositive++;
			if (c2.z > c2.w) outsidePositive++;

			if (c0.z < -c0.w) outsideNegative++;
			if (c1.z < -c1.w) outsideNegative++;
			if (c2.z < -c2.w) outsideNegative++;

			if (outsidePositive == 3 || outsideNegative == 3)
			{
				fullyOutside = true;
				break;
			}
		}

		if (fullyOutside) continue;
		*/

		const Vertex& v0 = projected[triangle.indices[0]];
		const Vertex& v1 = projected[triangle.indices[1]];
		const Vertex& v2 = projected[triangle.indices[2]];
		
		uint32_t faceColor = 0xFF000000 | ((i * 1234567 + 0x303030) % 0xFFFFFF);

		PlotTriangle(faceColor, v0.position.x, v0.position.y, faceColor, v1.position.x, v1.position.y, faceColor, v2.position.x, v2.position.y);
		TriangleWireframe(triangle.color, v0.position.x, v0.position.y, v1.position.x, v1.position.y, v2.position.x, v2.position.y);

	}
}

void Game::Render()
{
	UpdateWindow();
	
	Clear(0x00000000);

	mat4 model = (mat4::Identity() + mat::Translate(1.f, 1.f, 1.f)) * mat::Rotate(0.f, 1.f, 0.f, angle);

	mat4 view = mat::LookAt(mainCam.eye, mainCam.target, mainCam.up);
	float aspect = float(SCREEN_WIDTH) / float(SCREEN_HEIGHT);
	mat4 proj = mat::Perspective(3.14159f / 4.0f, aspect, 1.f, 10.0f);

	mat4 MV = view * model;
	mat4 MVP = proj * view * model;
	
	/*
	std::vector<float3> viewVertices(cube1.cubeVer.size());
	for (size_t i = 0; i < cube1.cubeVer.size(); ++i) 
	{
		float4 viewPos = cube1.cubeVer[i].pos4() * MV;
		viewVertices[i] = { viewPos.x, viewPos.y, viewPos.z }; // Drop w
	}
	*/

	RenderObject(0xFFFFFFFF, cube1.cubeVer, cube1.cubeTri, MVP, MV);

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