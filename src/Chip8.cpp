#include "Chip8.hpp"

#include <cstdlib>
#include <cstdio>
#include <cstring>

Chip8::Chip8(const char gameFile[])
{
	reset();

	FILE* file = fopen(gameFile, "r");

	fread(memory + 0x200, 1, sizeof(memory) - 0x200, file);

	fclose(file);

	for (int i = 0; i < sizeof(memory); i += 2)
		printf("(%d) %04X ", i, memory[i] << 8 | memory[i+1]);
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

	buzzer = false;

	memset(pixels, 0, 64*32);

	pixels[35][22] = true;
}

void Chip8::tick()
{
	printf("[%03X] ", programCounter);

	unsigned short opcode = memory[programCounter] << 8;
	opcode |= memory[programCounter + 1];

	// This offset (x2 bytes) will be applied to the program counter after the opcode is processed
	int pcDelta = 1;

	switch (opcode >> 12) {
		case 0x1: { // 1NNN: Jump to address
			programCounter = opcode & 0x0FFF;
			pcDelta = 0;

			printf("%04X (1NNN): jumping to address %03X\n", opcode, programCounter);
		} break;
		case 0x2: { // 2NNN
			stack[stackPointer] = programCounter;
			stackPointer++;

			programCounter = opcode & 0x0FFF;
			pcDelta = 0;

			printf("%04X (2NNN): subroutine call to address %03X\n", opcode, programCounter);
		} break;
		case 0x3: { // 3XNN
			int reg = (opcode & 0x0F00) >> 8;
			unsigned char number = opcode & 0x00FF;
			if (registers[reg] == number)
				 pcDelta = 2;

			printf("%04X (3XNN): Comparison \"==\"\n", opcode);
		} break;
		case 0x4: { // 4XNN
			int reg = (opcode & 0x0F00) >> 8;
			unsigned char number = opcode & 0x00FF;
			if (registers[reg] != number)
				pcDelta = 2;

			printf("%04X (4XNN): Comparison \"!=\"\n", opcode);
		} break;
		case 0x5: { // 5XY0
			int reg1 = (opcode & 0x0F00) >> 8;
			int reg2 = (opcode & 0x00F0) >> 4;
			if (registers[reg1] == registers[reg2])
				pcDelta = 2;

			printf("%04X (5XY0): Comparison\n", opcode);
		} break;
		case 0x6: { // 6XNN
			int reg = (opcode & 0x0F00) >> 8;
			registers[reg] = opcode & 0x00FF;

			printf("%04X (6XNN): Placed value %d in register %X\n", opcode, opcode & 0xFF, reg);
		} break;
		case 0xA: { // ANNN
			address = opcode & 0x0FFF;

			printf("%04X (ANNN): \n", opcode);
		} break;
		case 0xB: { // BNNN
			unsigned short nnn = opcode & 0x0FFF;
			programCounter = registers[0] + nnn;
			pcDelta = 0;

			printf("%04X (BNNN): \n", opcode);
		} break;
		case 0xC: { // CXNN
			int random = rand();
			int x = (opcode & 0x0F00) >> 16;
			int n = opcode & 0x00FF;
			registers[x] = random & n;

			printf("%04X (CXNN): Placing random value masked with %02X in register %X\n", opcode, n, x);
		} break;
		case 0xD: { // DXYN
			int x = (opcode & 0x0F00) >> 8;
			int y = (opcode & 0x00F0) >> 4;
			int xPos = registers[x];
			int yPos = registers[y];
			int n = opcode & 0x000F;
			int memoryPos = address;
			bool changed = false;

			for(int row = yPos; row < yPos + n; row++) {
				int column = xPos;
				for(int j = 7; j >= 0; j--) {
					bool set = (memory[memoryPos] & (1 << j)) > 0;
					if(pixels[column][row] && !set)
						changed = true;
					pixels[column][row] = set;
			  		column++;
				}
				memoryPos++;
			}
			registers[15] = changed ? 1 : 0;

			printf("%04X (DXYN): drawing sprite\n", opcode);
		} break;
		default: {
			printf("Unsupported opcode %04X, skipping!\n", opcode);
		}
	}

	programCounter += 2 * pcDelta;

}

bool Chip8::getPixel(int x, int y)
{
	return pixels[x][y];
}

void Chip8::setKey(int key, bool isPressed)
{
	keyStates[key] = isPressed;
}

bool Chip8::isBuzzer()
{
	return buzzer;
}
