#include "Chip8.hpp"

#include <cstdio>
#include <cstring>

Chip8::Chip8(const char gameFile[])
{
	reset();

	FILE* file = fopen(gameFile, "r");

	fread(memory + 0x200, 1, sizeof(memory) - 0x200, file);

	fclose(file);

	//for (int i = 0; i < sizeof(memory); i += 2)
	//	printf("(%d) %04X ", i, memory[i] << 8 | memory[i+1]);
}

void Chip8::reset()
{
	memset(memory, 0, sizeof(memory));
	memset(registers, 0, sizeof(registers));
	memset(stack, 0, sizeof(stack));
	memset(keyStates, 0, sizeof(keyStates));

	stackPointer = 0;
	address = 0;
	programCounter = 0x200;

	delayTimer = 0;
	soundTimer = 0;

	memset(pixels, 0, 64*32);

	pixels[35][22] = true;
}

void Chip8::tick()
{

}

bool Chip8::getPixel(int x, int y)
{
	return pixels[x][y];
}

void Chip8::setKey(int key, bool isPressed)
{
	keyStates[key] = isPressed;
}
