#include "Game.hpp"

void Game::Init()
{
	testBunny = new glTF_Mesh("Assets/Bunny/scene.gltf");

	testCharacter = new Model("Assets/Snake/Source/Old_Snake.obj");
	models.push_back(testCharacter);

	previousTime = std::chrono::high_resolution_clock::now();

	std::wstring title = L"Renderer";
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
	rotationIncrement += 0.1f;

	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> elapsed = currentTime - previousTime;
	float deltaTime = elapsed.count(); // deltaTime in seconds
	previousTime = currentTime;

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



void Game::PlotTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2, const Mesh& mesh, const int matIndex)
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
			float invZ = 1.0f / v0.position.z * w0 + 1.0f / v1.position.z * w1 + 1.0f / v2.position.z * w2;
			float z = 1.0f / invZ;

			int index = y * SCREEN_WIDTH + x;
			if (z >= depthBuffer[index]) continue;

			// We store some precaculated values for the affine inside vertices (invW and uvDivW)
			float invW = w0 * v0.invW + w1 * v1.invW + w2 * v2.invW;

			float u = (w0 * v0.uvDivW.x + w1 * v1.uvDivW.x + w2 * v2.uvDivW.x) / invW;
			float v = (w0 * v0.uvDivW.y + w1 * v1.uvDivW.y + w2 * v2.uvDivW.y) / invW;

			u = std::clamp(u, 0.0f, 1.f);
			v = std::clamp(v, 0.0f, 1.f);

			// Here we store the texture the triangle has
			Texture tex = mesh.textures[matIndex];

			// Sample texture
			int texX = int(u * tex.GetWidth());
			int texY = int(v * tex.GetHeight());
			
			// Clamp coordinates to valid range
			texX = std::clamp(texX, 0, tex.GetWidth() - 1);
			texY = std::clamp(texY, 0, tex.GetHeight() - 1);
			
			// Diffuse
			
			uint8_t r = tex.GetTexel(texX, texY, 0); // Red channel
			uint8_t g = tex.GetTexel(texX, texY, 1); // Green channel
			uint8_t b = tex.GetTexel(texX, texY, 2); // Blue channel
			

			// UV
			/*
			uint8_t r = uint8_t(u * 255.0f);
			uint8_t g = uint8_t(v * 255.0f);
			uint8_t b = 0; // You can also set b = 255 - g for gradient effect
			*/

			// Write to framebuffer
			depthBuffer[index] = z;
			Plot(MakeColor(r, g, b, 255), x, y);
		}
	}
}

void Game::RenderGLTF(std::vector<glTF_Mesh::Vertex*> vertices, std::vector<glTF_Mesh::Triangle*> triangles, const mat4& MV, const mat4& proj)
{
	// Convert glTF vertices to your rendering vertex format
	std::vector<Vertex> renderVertices;
	std::vector<Triangle> renderTriangles;

	// Convert glTF vertices to your Vertex format
	renderVertices.reserve(vertices.size());
	for (const auto* gltfVertex : vertices)
	{
		Vertex v;
		// Convert glm::vec3 to your float3 format
		v.position = { gltfVertex->m_position.x, gltfVertex->m_position.y, gltfVertex->m_position.z };
		v.uv = { gltfVertex->m_texcoord_0.x, gltfVertex->m_texcoord_0.y };
		// You can add normal, color, etc. if needed
		renderVertices.push_back(v);
	}

	// Convert glTF triangles to your Triangle format
	renderTriangles.reserve(triangles.size());
	for (const auto* gltfTriangle : triangles)
	{
		Triangle tri;
		tri.indices[0] = gltfTriangle->m_indices[0];
		tri.indices[1] = gltfTriangle->m_indices[1];
		tri.indices[2] = gltfTriangle->m_indices[2];
		tri.materialIndex = 0; // You might want to handle materials differently
		renderTriangles.push_back(tri);
	}

	// Now use the same rendering pipeline as OBJ
	auto interpolate = [](const Vertex& a, const Vertex& b, float t) -> Vertex
		{
			Vertex v;
			v.position = a.position + (b.position - a.position) * t;
			v.uv = a.uv + (b.uv - a.uv) * t;
			return v;
		};

	std::vector<Vertex> viewVerts;
	std::vector<Triangle> clippedTris;
	std::vector<Vertex> clippedVerts;

	viewVerts.reserve(renderVertices.size());
	for (const auto& v : renderVertices)
	{
		float4 viewPos = v.pos4() * MV;
		Vertex out = v;
		out.position = { viewPos.x, viewPos.y, viewPos.z };
		viewVerts.push_back(out);
	}

	for (auto& triangle : renderTriangles)
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
			t1.materialIndex = triangle.materialIndex;
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
			t1.materialIndex = triangle.materialIndex;
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
			t1.materialIndex = triangle.materialIndex;
			t2.materialIndex = triangle.materialIndex;
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

		Vertex v;
		v.position = { screenX, screenY, ndcZ }; // keep ndcZ for depth buffer
		v.invW = 1.0f / c.w;
		v.uvDivW = clippedVerts[i].uv * v.invW;

		projected[i] = v;
	}

	for (const auto& tri : culledTriangles)
	{
		const Vertex& v0 = projected[tri.indices[0]];
		const Vertex& v1 = projected[tri.indices[1]];
		const Vertex& v2 = projected[tri.indices[2]];
		int materialIndex = tri.materialIndex;

		// For glTF, I want to handle materials differently
		// For now I just pass the snake obj mesh and it will take it's texture
		PlotTriangle(v0, v1, v2, models[0]->mesh, materialIndex);
	}
}

// TODO: arguments on this functions are not needed since I pass model pointer
void Game::RenderOBJ(Model* targetModel, std::vector<Vertex>& vertices, std::vector<Triangle>& triangles, const mat4& MV, const mat4& proj)
{
	// ChatGPT generated
	auto interpolate = [](const Vertex& a, const Vertex& b, float t) -> Vertex 
		{
		Vertex v;
		v.position = a.position + (b.position - a.position) * t;
		v.uv = a.uv + (b.uv - a.uv) * t;
		return v;
		};

	std::vector<Vertex> viewVerts;
	std::vector<Triangle> clippedTris;
	std::vector<Vertex> clippedVerts;

	viewVerts.reserve(vertices.size());
	for (const auto& v : vertices)
	{
		float4 viewPos = v.pos4() * MV;
		Vertex out = v;
		out.position = { viewPos.x, viewPos.y, viewPos.z };
		viewVerts.push_back(out);
	}

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
			t1.materialIndex = triangle.materialIndex;
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
			t1.materialIndex = triangle.materialIndex;
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
			t1.materialIndex = triangle.materialIndex;
			t2.materialIndex = triangle.materialIndex;
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

		Vertex v;
		v.position = { screenX, screenY, ndcZ }; // keep ndcZ for depth buffer
		v.invW = 1.0f / c.w;
		v.uvDivW = clippedVerts[i].uv * v.invW;

		projected[i] = v;
	}

	for (const auto& tri : culledTriangles)
	{
		const Vertex& v0 = projected[tri.indices[0]];
		const Vertex& v1 = projected[tri.indices[1]];
		const Vertex& v2 = projected[tri.indices[2]];
		int materialIndex = tri.materialIndex;
		PlotTriangle(v0, v1, v2, targetModel->mesh, materialIndex);
	}
}

void Game::Render()
{
	UpdateWindow();
	
	Clear(0x00000000);

	mat4 model = (mat::Translate(0.f, -4.5f, 3.5f) + mat::Scale(40, 40, 40)) * mat::Rotate(0.0f, 1.f, 0.0f, rotationIncrement);
	mat4 view = mat::LookAt(mainCam.eye, mainCam.eye + mainCam.target, mainCam.up);
	mat4 proj = mat::Perspective(mainCam.fovRad, mainCam.aspect, 1.0f, 500.0f);

	mat4 MV = view * model;
	mat4 MVP = proj * view * model;

	// SNAKE: .obj file 
	//RenderObject(models[0], models[0]->mesh.vertices, models[0]->mesh.triangle, MV, proj);

	// BUNNY: .gltf file
	RenderGLTF(testBunny->m_vertices, testBunny->m_triangles, MV, proj);

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