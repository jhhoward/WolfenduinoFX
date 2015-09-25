/*

Takes a raw output of the wolf3d map (64x64) and puts into a format that the game can read

*/

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <algorithm>
#include <cassert>

#define MAP_SIZE 64
#define MAP_CHUNK_WIDTH 16
#define NUM_MAP_CHUNKS 16
#define MAP_CHUNKS_PER_MAP_WIDTH 4
#define RUN_LENGTH_BIT (1 << 7)

#define AREATILE 107
#define RLE_TOKEN 0xff
#define USE_RLE_TOKEN 0

// input 
uint8_t layer1[MAP_SIZE * MAP_SIZE];
uint8_t layer2[MAP_SIZE * MAP_SIZE];

// output
uint8_t outlayer[MAP_SIZE * MAP_SIZE];

int outChunkOffsets[NUM_MAP_CHUNKS];
uint8_t outData[MAP_SIZE * MAP_SIZE];
int outDataLength = 0;

using namespace std;

struct HuffmanNode
{
	HuffmanNode* left;
	HuffmanNode* right;
	bool isLeaf;
	int value;
};

struct HuffmanEntry
{
	int value;
	int frequency;
	HuffmanNode* node;
};

HuffmanNode* huffmanTree = NULL;
vector<uint8_t> huffmanCodes[256];
int huffmanFrequency[256];

bool CompareHuffmanEntries(HuffmanEntry& a, HuffmanEntry& b)
{
	return a.frequency > b.frequency;
}

void TraverseHuffmanTree(HuffmanNode* node, vector<uint8_t>& code)
{
	if(node->left)
	{
		code.push_back(0);
		TraverseHuffmanTree(node->left, code);
		code.pop_back();
	}
	if(node->right)
	{
		code.push_back(1);
		TraverseHuffmanTree(node->right, code);
		code.pop_back();
	}
	if(node->isLeaf)
	{
		huffmanCodes[node->value] = code;
	}
}

void AddHuffmanFrequencies(vector<uint8_t> sampleData)
{
	for(int n = 0; n < sampleData.size(); n++)
	{
		huffmanFrequency[sampleData[n]]++;
	}
}

void GenerateHuffmanTree()
{
	vector<HuffmanEntry> entries;

/*	for(int n = 0; n < 256; n++)
	{
		frequency[n] = 0;
	}
	for(int n = 0; n < MAP_SIZE * MAP_SIZE; n++)
	{
		frequency[outlayer[n]] ++;
	}
	*/
	for(int n = 0; n < 256; n++)
	{
		if(huffmanFrequency[n] > 0)
		{
			HuffmanEntry entry;
			entry.value = n;
			entry.frequency = huffmanFrequency[n];
			entry.node = NULL;
			entries.push_back(entry);
		}
	}

	while(entries.size() > 1)
	{
		sort(entries.begin(), entries.end(), CompareHuffmanEntries);

		HuffmanEntry a = entries[entries.size() - 1];
		entries.pop_back();
		HuffmanNode* left = a.node;
		if(!left)
		{
			left = new HuffmanNode();
			left->left = NULL;
			left->right = NULL;
			left->isLeaf = true;
			left->value = a.value;
		}

		HuffmanEntry b = entries[entries.size() - 1];
		entries.pop_back();
		HuffmanNode* right = b.node;
		if(!right)
		{
			right = new HuffmanNode();
			right->left = NULL;
			right->right = NULL;
			right->isLeaf = true;
			right->value = b.value;
		}

		HuffmanNode* parent = new HuffmanNode();
		parent->isLeaf = false;
		parent->left = left;
		parent->right = right;

		HuffmanEntry newEntry;
		newEntry.frequency = a.frequency + b.frequency;
		newEntry.node = parent;
		newEntry.value = 0;

		entries.push_back(newEntry);
	}

	HuffmanEntry top = entries[0];
	huffmanTree = top.node;

	vector<uint8_t> code;
	TraverseHuffmanTree(huffmanTree, code);

	for(int n = 0; n < 256; n++)
	{
		if(huffmanCodes[n].size() > 0)
		{
			printf("%d: ", n);
			for(int i = 0; i < huffmanCodes[n].size(); i++)
			{
				printf("%d", huffmanCodes[n][i]);
			}
			printf("\n");
		}
	}
}

vector<uint8_t> CompressBlock(vector<uint8_t>& inputData)
{
	vector<uint8_t> compressed;

	for(int n = 0; n < inputData.size(); n++)
	{
		vector<uint8_t>& code = huffmanCodes[inputData[n]];
		assert(code.size() > 0);
		compressed.insert(compressed.end(), code.begin(), code.end());
	}

	while((compressed.size() % 8) > 0)
	{
		compressed.push_back(0);
	}

	return compressed;
}

void CompressMap()
{
	vector< vector<uint8_t> > rleCompressedChunks;
	vector<uint8_t> compressedMap;

	for(int chunkY = 0; chunkY < MAP_CHUNKS_PER_MAP_WIDTH; chunkY++)
	{
		for(int chunkX = 0; chunkX < MAP_CHUNKS_PER_MAP_WIDTH; chunkX++)
		{
			vector<uint8_t> chunkData;
			int lastData = 0;
			int runLength = 0;
			//outChunkOffsets[chunkY * MAP_CHUNKS_PER_MAP_WIDTH + chunkX] = outPtr - outData;

			for(int j = 0; j < MAP_CHUNK_WIDTH; j++)
			{
				for(int i = 0; i < MAP_CHUNK_WIDTH; i++)
				{
					int x = chunkX * MAP_CHUNKS_PER_MAP_WIDTH + i;
					int y = chunkY * MAP_CHUNKS_PER_MAP_WIDTH + j;
					int data = outlayer[y * MAP_SIZE + x];
					if(runLength > 0 && lastData == data)
					{
						runLength ++;
					}
					else
					{
						if(runLength > 1)
						{
#if USE_RLE_TOKEN
							chunkData.push_back(RLE_TOKEN);
							chunkData.push_back(lastData);
#else
							chunkData.push_back(lastData | RUN_LENGTH_BIT);
#endif
							chunkData.push_back(runLength);
						}
						else
						{
							chunkData.push_back(lastData);
						}
					
						lastData = data;
						runLength = 1;
					}
				}
			}

			// End of chunk
			if(runLength > 1)
			{
#if USE_RLE_TOKEN
				chunkData.push_back(RLE_TOKEN);
				chunkData.push_back(lastData);
#else
				chunkData.push_back(lastData | RUN_LENGTH_BIT);
#endif
				chunkData.push_back(runLength);
			}
			else
			{
				chunkData.push_back(lastData);
			}
			
			rleCompressedChunks.push_back(chunkData);
		}
	}

	for(int n = 0; n < rleCompressedChunks.size(); n++)
	{
		AddHuffmanFrequencies(rleCompressedChunks[n]);
	}

	GenerateHuffmanTree();

	for(int n = 0; n < rleCompressedChunks.size(); n++)
	{
		vector<uint8_t> compressed = CompressBlock(rleCompressedChunks[n]);

		compressedMap.insert(compressedMap.end(), compressed.begin(), compressed.end());
	}

	printf("Total data size: %d\n", compressedMap.size() / 8);
}

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		printf("Usage: %s [input] [output]\n\n", argv[0]);
		return 0;
	}

	FILE* fs = fopen(argv[1], "rb");
	if(!fs)
	{
		printf("Error opening %s\n", argv[1]);
		return 0;
	}

	fread(layer1, 1, MAP_SIZE * MAP_SIZE, fs);
	fread(layer2, 1, MAP_SIZE * MAP_SIZE, fs);

	fclose(fs);

	fs = fopen(argv[2], "w");
	if(!fs)
	{
		printf("Error opening %s\n", argv[2]);
		return 0;
	}

	fprintf(fs, "const uint8_t mapData[] PROGMEM = {\n\t");

	for(int y = 0; y < MAP_SIZE; y++)
	{
		for(int x = 0; x < MAP_SIZE; x++)
		{
			uint8_t l1 = layer1[y * MAP_SIZE + x];
			uint8_t l2 = layer2[y * MAP_SIZE + x];
			if(l1 < AREATILE)
			{
				outlayer[y * MAP_SIZE + x] = l1;
			}
			else
			{
				outlayer[y * MAP_SIZE + x] = 0;
			}
		}
	}

	for(int y = 0; y < MAP_SIZE; y++)
	{
		for(int x = 0; x < MAP_SIZE; x++)
		{
			fprintf(fs, "0x%02x", outlayer[y * MAP_SIZE + x]);
			if(x < MAP_SIZE - 1 || y < MAP_SIZE - 1)
			{
				fprintf(fs, ",");
			}
		}
		if(y < MAP_SIZE - 1)
		{
			fprintf(fs, "\n\t");
		}
	}

	for(int chunkY = 0; chunkY < MAP_CHUNKS_PER_MAP_WIDTH; chunkY++)
	{
	}

	fprintf(fs, "\n};\n\n");

	fclose(fs);

	CompressMap();

	return 0;
}
