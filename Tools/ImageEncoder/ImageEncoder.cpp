#include <math.h>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include "lodepng.h"

using namespace std;

enum EncodeMode
{
	Encode_Invalid = -1,
	Encode_Texture,
	Encode_Sprite
};

uint8_t texturePalette[] = 
{
	191, 191, 191,		// light grey
	255, 255, 255,		// white
	0, 0, 0,			// black
	127, 127, 127		// grey
};

uint8_t texturePaletteSprite[] = 
{
	0, 255, 255,		// cyan
	255, 255, 255,		// white
	0, 0, 0,			// black
	127, 127, 127		// grey
};

EncodeMode encodeMode = Encode_Invalid;

int GetPaletteIndexFromColour(uint8_t* palette, uint8_t r, uint8_t g, uint8_t b)
{
	int bestPalette = -1;
	int bestDistance = -1;
	
	for(int n = 0; n < 4; n++)
	{
		int rdiff = (int) palette[n * 3] - (int) r;
		int gdiff = (int) palette[n * 3 + 1] - (int) g;
		int bdiff = (int) palette[n * 3 + 2] - (int) b;
		int distance = (int) sqrt(rdiff * rdiff + gdiff * gdiff + bdiff * bdiff);
		
		if(bestPalette == -1 || distance < bestDistance)
		{
			bestPalette = n;
			bestDistance = distance;
		}
	}
	
	return bestPalette;
}

vector<uint8_t> EncodeImage(vector<uint8_t> data, int width, int height)
{
	uint8_t buffer = 0;
	int bufferPos = 0;
	vector<uint8_t> output;
	
	for(int x = 0; x < width; x++)
	{
		for(int y = 0; y < height; y++)
		{
			int position = (y * width + x) * 4;
			uint8_t* palette = encodeMode == Encode_Texture ? texturePalette : texturePaletteSprite;
			int index = GetPaletteIndexFromColour(palette, data[position], data[position + 1], data[position + 2]);
			buffer |= ((index & 0x3) << bufferPos);
			bufferPos += 2;
			if(bufferPos >= 8)
			{
				output.push_back(buffer);
				buffer = 0;
				bufferPos = 0;
			}
		}
	}
	
	if(bufferPos > 0)
	{
		output.push_back(buffer);
	}
	
	return output;
}

void OutputFile(char* filename, char* varName, vector<uint8_t> data, int width, int height, bool outputDimensions)
{
	FILE* fs = NULL;
	
	fopen_s(&fs, filename, "w");
	
	if(fs)
	{
		fprintf(fs, "const uint8_t %s[] PROGMEM = {\n\t", varName);
		for(int n = 0; n < data.size(); n++)
		{
			fprintf(fs, "0x%02x", data[n]);
			
			if(n != data.size() - 1)
			{
				fprintf(fs, ",");
				
				if(n > 0 && (n % 20) == 0)
				{
					fprintf(fs, "\n\t");
				}
			}
		}
		fprintf(fs, "};\n");
		fclose(fs);
	}
	else
	{
		printf("Unable to open %s for write\n", filename);
	}
}

void PrintUsage(char* processName)
{
	printf("Usage:\n"
			"%s [input.png] [output.inc.h] [varName] [texture|sprite]\n", processName);
}

int main(int argc, char* argv[])
{
	if(argc != 5)
	{
		PrintUsage(argv[0]);
		return 0;
	}

	if(!strcmp(argv[4], "texture"))
	{
		encodeMode = Encode_Texture;
	}
	else if(!strcmp(argv[4], "sprite"))
	{
		encodeMode = Encode_Sprite;
	}
	
	if(encodeMode == Encode_Invalid)
	{
		PrintUsage(argv[0]);
		return 0;
	}
	
	char* filename = argv[1];
	char* outputFilename = argv[2];
	char* varName = argv[3];
	vector<uint8_t> image; 
	unsigned width, height;

	unsigned error = lodepng::decode(image, width, height, filename);
	
	if(!error)
	{
		vector<uint8_t> encoded = EncodeImage(image, width, height);
		OutputFile(outputFilename, varName, encoded, width, height, false);
	}
	else
	{
		printf("Unable to open %s for read\n", filename);
	}

	return 0;
}
