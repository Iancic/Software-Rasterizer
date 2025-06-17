#include "Program.hpp"

Program::Program(const char* title)
{
	Init();
}

Program::~Program()
{
	Shutdown();
}

void Program::Quit()
{

}