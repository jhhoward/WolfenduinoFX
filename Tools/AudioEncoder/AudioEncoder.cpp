#include <stdio.h>
#include <stdint.h>
#include <vector>

using namespace std;

#define NUM_NOTES 59
#define MUTE_NOTE 63

#define INSTRUMENT_COMMAND(x) (((x) << 6) | (1 << 2) | 1)
#define NOTE_COMMAND(note, duration) (((note) << 2) | ((duration) << 8))

float noteFrequencies[NUM_NOTES] = 
{
	116.5f,	// A#2
	123.5f,	// B
	130.8f,	// C3
	138.6f,	// C#
	146.8f,	// D
	155.6f,	// D#
	164.8f,	// E .
	174.6f,	// F
	185.0f,	// F#
	196.0f,	// G
	207.7f,	// G#
	220.0f,	// A
	233.1f,	// A#
	246.9f,	// B
	261.6f,	// C4 .
	277.2f,	
	293.7f,	
	311.1f,	
	329.6f,	
	349.2f,	
	370.0f,	
	392.0f,	
	415.3f,	
	440.0f,	
	466.2f,	
	493.9f,
	523.3f,	
	554.4f,	
	587.3f,	
	622.3f,	
	659.3f,	
	698.5f,	
	740.0f,	
	784.0f,	
	830.6f,	
	880.0f,	
	932.3f,	
	987.8f,
	1047,	
	1109,	
	1175,	
	1245,	
	1319,	
	1397,	
	1480,	
	1568,	
	1661,	
	1760,	
	1865,	
	1976,
	2093,	
	2217,	
	2349,	
	2489,	// D#7
	2637,	
	2794,	
	2960,	
	3136,	
	3322
};

struct AudioNote
{
	uint8_t note;
	bool noisy;
};

struct AudioPattern
{
	vector<uint16_t> data;
};

uint8_t getBestNote(float frequency)
{
	if(frequency == 0.0f)
		return MUTE_NOTE;

	uint8_t closest = MUTE_NOTE;
	float closestDistance = 0.0f;

	for(int n = 0; n < NUM_NOTES; n++)
	{
		float distance = frequency - noteFrequencies[n];
		if(distance < 0) 
			distance = -distance;

		if(closest == MUTE_NOTE || distance < closestDistance)
		{
			closest = n;
			closestDistance = distance;
		}
	}

	return closest;
}

AudioPattern processData(uint8_t* data, long length)
{
	const int stepSize = 5;
	int averageCount = 0;
	float averageFrequency = 0;
	float frequencyMin, frequencyMax;
	int stepCount = 0;
	vector<AudioNote> notes;

	for(int n = 0; n < length; n++)
	{
		float frequency = 1193181.0f / (data[n] * 60.0f);
		if(data[n] == 0) 
		{
			frequency = 0.0f;
		}
		else
		{
			if(averageCount == 0)
			{
				frequencyMin = frequencyMax = frequency;
			}
			else
			{
				if(frequency < frequencyMin)
				{
					frequencyMin = frequency;
				}
				if(frequency > frequencyMax)
				{
					frequencyMax = frequency;
				}
			}
			averageFrequency += frequency;
			averageCount++;
		}
		printf("%d[%d], ", (int)frequency, getBestNote(frequency));

		stepCount ++;
		if(stepCount == stepSize || n == length - 1)
		{
			AudioNote note;
			
			// If more than half of the frequencies are zero, then force to zero
			if(notes.size() > 0 && n < length - 1)
			{
				if(averageCount <= stepSize / 2)
				{
					averageCount = 0;
				}
			}

			if(averageCount == 0)
			{
				note.note = MUTE_NOTE;
				note.noisy = false;
			}
			else
			{
				averageFrequency /= averageCount;
				float range = frequencyMax - frequencyMin;
				note.note = getBestNote(averageFrequency);

				int noteRange = getBestNote(frequencyMax) - getBestNote(frequencyMin);

				note.noisy = noteRange > 4;
			}
			notes.push_back(note);

			stepCount = 0;
			averageCount = 0;
			averageFrequency = 0.0f;
			frequencyMax = 0.0f;
			frequencyMin = 0.0f;
		}
	}
	
	while(notes.size() > 0 && notes.back().note == MUTE_NOTE)
		notes.pop_back();

	vector<uint16_t> commandBuffer;
	bool isNoisy = notes[0].noisy;
	int currentDuration = 0;
	uint8_t currentNote = MUTE_NOTE;

	if(isNoisy)
	{
		commandBuffer.push_back(INSTRUMENT_COMMAND(1));
	}
	else
	{
		commandBuffer.push_back(INSTRUMENT_COMMAND(0));
	}

	printf("\nCommand instrument: %d\n", isNoisy ? 1 : 0);

	for(int n = 0; n < notes.size(); n++)
	{
		if((isNoisy != notes[n].noisy && notes[n].note != MUTE_NOTE) || currentNote != notes[n].note)
		{
			if(currentDuration > 0)
			{
				commandBuffer.push_back(NOTE_COMMAND(currentNote, currentDuration));
				printf("Command note: %d, %d\n", currentNote, currentDuration);

				currentDuration = 0;
			}
			if(isNoisy != notes[n].noisy && notes[n].note != MUTE_NOTE)
			{
				isNoisy = notes[n].noisy;
				if(isNoisy)
				{
					commandBuffer.push_back(INSTRUMENT_COMMAND(1));
				}
				else
				{
					commandBuffer.push_back(INSTRUMENT_COMMAND(0));
				}
				printf("Command instrument: %d\n", isNoisy ? 1 : 0);
			}
			currentNote = notes[n].note;
			currentDuration = 1;
		}
		else
		{
			currentDuration ++;
		}
	}

	if(currentDuration > 0)
	{
		commandBuffer.push_back(NOTE_COMMAND(currentNote, currentDuration));
		printf("Command note: %d, %d\n", currentNote, currentDuration);
	}
	
	commandBuffer.push_back(0);

	printf("Command buffer size: %d bytes\n", commandBuffer.size() * 2);
	
	AudioPattern pattern;
	pattern.data = commandBuffer;
	return pattern;
}

AudioPattern loadData(char* filename)
{
	FILE* fs = NULL;
	AudioPattern pattern;
	
	fopen_s(&fs, filename, "rb");

	if(!fs)
	{
		printf("Error opening %s\n", filename);
		return pattern;
	}

	long fileSize;

	fseek (fs , 0 , SEEK_END);
	fileSize = ftell (fs);
	rewind (fs);

	uint8_t* buffer = new uint8_t[fileSize];
	fread(buffer, 1, fileSize, fs);

	pattern = processData(buffer, fileSize);

	delete[] buffer;

	fclose(fs);

	return pattern;
}

FILE* fs = NULL;

void writeData(char* filename, vector<AudioPattern>& patterns)
{
	fopen_s(&fs, filename, "w");
	
	if(fs)
	{
		for(int p = 0; p < patterns.size(); p++)
		{
			fprintf(fs, "const uint16_t Data_Audio%02d[] PROGMEM = {\n\t", p);
			for(int n = 0; n < patterns[p].data.size(); n++)
			{
				fprintf(fs, "0x%04x", patterns[p].data[n]);
				
				if(n != patterns[p].data.size() - 1)
				{
					fprintf(fs, ",");
					
					if(n > 0 && (n % 20) == 0)
					{
						fprintf(fs, "\n\t");
					}
				}
			}
			fprintf(fs, "\n};\n");
		}
		fclose(fs);
	}
	else
	{
		printf("Unable to open %s for write\n", filename);
	}
}
	
int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		printf("Usage:\n%s [output file] [input files]\n", argv[0]);
		return 0;
	}

	int numInputFiles = argc - 2;
	vector<AudioPattern> patterns;
	
	for(int n = 0; n < numInputFiles; n++)
	{
		AudioPattern pattern = loadData(argv[n + 2]);
		patterns.push_back(pattern);
	}
	
	writeData(argv[1], patterns);

	return 0;
}
