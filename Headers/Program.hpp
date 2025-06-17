#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <Windows.h>

#include <map>
#include <unordered_map>
#include <thread>
#include <iostream>
#include <algorithm>
#include <chrono>

#include "Common.hpp"
#include "Logger.hpp"

class Program
{
public:
	Program(const char* title);
	virtual ~Program();

	virtual void Init() {};
	virtual void Shutdown() {};
	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual void HandleEvents() = 0;
	virtual void HandleInput() = 0;
	virtual void Quit();

	bool isRunning = true;

protected:

	// Time
	int milisecondsPreviousFrame;
	double deltaTime;
	float timeElapsed;
};