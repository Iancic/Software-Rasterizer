#pragma once
#include <cstdint>
#include <algorithm>
#include <math.h>
#include "Common.hpp"

struct int2
{
	int x;
	int y;
};

static inline int cross(int2& a, int2& b, int2& p)
{
	int2 ab = { b.x - a.x, b.y - a.y };
	int2 ap = { p.x - a.x, p.y - a.y };
	return ab.x * ap.y - ab.y * ap.x;
}

struct int3
{
	int x;
	int y;
	int z;
};

struct float2
{
	float x;
	float y;
};

struct float3
{
	float x;
	float y;
	float z;
};

inline float3 operator-(const float3 A, const float3& B)
{
	return
	{
		A.x - B.x, A.y - B.y, A.z - B.z
	};
};

inline float3 operator+(const float3 A, const float3& B)
{
	return
	{
		A.x + B.x, A.y + B.y, A.z + B.z
	};
};

inline float3 operator/(const float3 A, const float3& B)
{
	return
	{
		A.x / B.x, A.y / B.y, A.z / B.z
	};
};

inline float3 operator/(const float3 A, const float& B)
{
	return
	{
		A.x / B, A.y / B, A.z / B
	};
};

static inline float Dot(float3 a, float3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float3 Normalize(float3 v)
{
	float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return { v.x / len, v.y / len, v.z / len };
}

static inline float3 normalize(float3 a)
{
	float len = sqrtf(Dot(a, a));
	if (len == 0.0f) return { 0, 0, 0 }; // avoid divide by zero
	return a / len;
}

static inline float3 Cross(float3 a, float3 b)
{
	return {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

struct float4
{
	float x;
	float y;
	float z;
	float w;
};

struct mat4
{
	float m[4][4];
	static mat4 Identity()
	{
		mat4 mat{};
		mat.m[0][0] = mat.m[1][1] = mat.m[2][2] = mat.m[3][3] = 1.f;
		return mat;
	}
};

inline float4 operator*(const float4 V, const mat4& M)
{
	return {
		V.x * M.m[0][0] + V.y * M.m[1][0] + V.z * M.m[2][0] + V.w * M.m[3][0],
		V.x * M.m[0][1] + V.y * M.m[1][1] + V.z * M.m[2][1] + V.w * M.m[3][1],
		V.x * M.m[0][2] + V.y * M.m[1][2] + V.z * M.m[2][2] + V.w * M.m[3][2],
		V.x * M.m[0][3] + V.y * M.m[1][3] + V.z * M.m[2][3] + V.w * M.m[3][3]
	};
}

inline mat4 operator+(const mat4& A, const mat4& B)
{
	return {
		A.m[0][0] + B.m[0][0], A.m[1][0] + B.m[1][0], A.m[2][0] + B.m[2][0], A.m[3][0] + B.m[3][0],
		A.m[0][1] + B.m[0][1], A.m[1][1] + B.m[1][1], A.m[2][1] + B.m[2][1], A.m[3][1] + B.m[3][1],
		A.m[0][2] + B.m[0][2], A.m[1][2] + B.m[1][2], A.m[2][2] + B.m[2][2], A.m[3][2] + B.m[3][2],
		A.m[0][3] + B.m[0][3], A.m[1][3] + B.m[1][3], A.m[2][3] + B.m[2][3], A.m[3][3] + B.m[3][3]
	};
}

inline mat4 operator*(const mat4& A, const mat4& B)
{
	mat4 R{};
	for (int r = 0; r < 4; ++r)
		for (int c = 0; c < 4; ++c)
			for (int k = 0; k < 4; ++k)
				R.m[c][r] += A.m[k][r] * B.m[c][k];
	return R;
};

struct Vertex
{
	float3 position;
	uint32_t color;
	float4 pos4() const { return { position.x, position.y, position.z, 1.f }; };
};

struct Triangle
{
	uint32_t color;
	int indices[3];
};

namespace mat
{
	static inline mat4 Translate(const float pX, const float pY, const float pZ)
	{
		return {
			1, 0, 0, pX,
			0, 1, 0, pY,
			0, 0, 1, pZ,
			0, 0, 0, 1
		};
	}

	static inline mat4 Rotate(const float rX, const float rY, const float rZ, const float theta)
	{
		float c = cos(theta);
		float s = sin(theta);
		float t = 1.f - c;

		return {
			t * rX * rX + c,      t * rX * rY - s * rZ,  t * rX * rZ + s * rY,  0.f,
			t * rX * rY + s * rZ,   t * rY * rY + c,     t * rY * rZ - s * rX,  0.f,
			t * rX * rZ - s * rY,   t * rY * rZ + s * rX,  t * rZ * rZ + c,     0.f,
			0.f,              0.f,             0.f,             1.f
		};
	}

	static inline mat4 Scale(const float sX, const float sY, const float sZ)
	{
		return {
			sX, 0, 0, 0,
			0, sY, 0, 0,
			0, 0, sZ, 0,
			0, 0, 0, 1
		};
	}

	// TODO: These are blackboxed now learn about them
	static inline mat4 LookAt(float3 eye, float3 target, float3 up)
	{
		float3 zaxis = Normalize(eye - target);               
		float3 xaxis = Normalize(Cross(up, zaxis));
		float3 yaxis = Cross(zaxis, xaxis);

		mat4 view = mat4::Identity();

		view.m[0][0] = xaxis.x;  view.m[0][1] = xaxis.y;  view.m[0][2] = xaxis.z;
		view.m[1][0] = yaxis.x;  view.m[1][1] = yaxis.y;  view.m[1][2] = yaxis.z;
		view.m[2][0] = zaxis.x;  view.m[2][1] = zaxis.y;  view.m[2][2] = zaxis.z;

		view.m[3][0] = -Dot(xaxis, eye);
		view.m[3][1] = -Dot(yaxis, eye);
		view.m[3][2] = -Dot(zaxis, eye);

		return view;
	}

	// TODO: These are blackboxed now learn about them
	static inline mat4 Perspective(float fovY, float aspect, float zn, float zf)
	{
		float yScale = 1.0f / tanf(fovY / 2.0f);
		float xScale = yScale / aspect;

		return mat4{
			xScale, 0,      0,                          0,
			0,      yScale, 0,                          0,
			0,      0,      zf / (zf - zn),             1,
			0,      0,      -zn * zf / (zf - zn),       0
		};
	};
}

// Classes For Triangle fill
// Article by https://joshbeam.com/articles/triangle_rasterization/
class Edge
{
public:
	uint32_t Color1, Color2;
	int X1, Y1, X2, Y2;

	// this makes sure first point of the edge is the one with lower y values
	Edge(const uint32_t& color1, int x1, int y1, const uint32_t& color2, int x2, int y2)
	{
		if (y1 < y2) 
		{
			Color1 = color1;
			X1 = x1;
			Y1 = y1;
			Color2 = color2;
			X2 = x2;
			Y2 = y2;
		}
		else 
		{
			Color1 = color2;
			X1 = x2;
			Y1 = y2;
			Color2 = color1;
			X2 = x1;
			Y2 = y1;
		}
	};
};

// a span is an edge but without the y cause it's parralel to the screen we just go down
// it's the span on horizontal
// when filling we go down with this one
class Span
{
public:
	uint32_t Color1, Color2;
	int X1, X2;

	// same as the edge we have to make sure the x1 is the one that's the lowest.
	Span(const uint32_t& color1, int x1, const uint32_t& color2, int x2)
	{
		if (x1 < x2)
		{
			Color1 = color1;
			X1 = x1;
			Color2 = color2;
			X2 = x2;
		}
		else
		{
			Color1 = color2;
			X1 = x2;
			Color2 = color1;
			X2 = x1;
		}
	};
};