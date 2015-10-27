#include <SPI.h>
#include <Gamebuino.h>

#define NOTE(pitch, duration) (pitch << 2) | (duration << 8)
#define COMMAND(c, x, y) (0x1 | ((c) << 2) | ((x) << 6) | ((y) << 11))
/*
uint16_t pattern[] PROGMEM = {
	NOTE(1, 1),
	NOTE(2, 1),
	NOTE(3, 1),
	NOTE(4, 1),
	NOTE(5, 1),
	NOTE(6, 1),
	NOTE(7, 1),
	NOTE(8, 1),
	NOTE(9, 1),
	NOTE(10, 1),
	NOTE(11, 1),
	NOTE(12, 1),
	NOTE(13, 1),
	NOTE(14, 1),
	NOTE(15, 1),
	NOTE(16, 1),
	NOTE(17, 1),
	NOTE(18, 1),
	NOTE(19, 1),
	NOTE(20, 1),
	NOTE(21, 1),
	NOTE(22, 1),
	NOTE(23, 1),
	NOTE(24, 1),
	NOTE(25, 1),
	NOTE(26, 1),
	NOTE(27, 1),
	NOTE(28, 1),
	NOTE(29, 1),
	NOTE(30, 1),
	NOTE(31, 1),
	NOTE(32, 1),
	NOTE(33, 1),
	NOTE(34, 1),
	NOTE(35, 1),
	NOTE(36, 1),
	NOTE(37, 1),
	NOTE(38, 1),
	NOTE(39, 1),
	NOTE(40, 1),
	NOTE(41, 1),
	NOTE(42, 1),
	NOTE(43, 1),
	NOTE(44, 1),
	NOTE(45, 1),
	NOTE(46, 1),
	NOTE(47, 1),
	NOTE(48, 1),
	NOTE(49, 1),
	NOTE(50, 1),
	NOTE(51, 1),
	NOTE(52, 1),
	NOTE(53, 1),
	NOTE(54, 1),
	NOTE(55, 1),
	NOTE(56, 1),
	NOTE(57, 1),
	NOTE(58, 1),
	0
}; */

#include "Data_Audio.h"

const char* patternNames[] = {
	"HITWALL",       
	"SELECTWPN",     
	"SELECTITEM",    
	"HEARTBEAT",     
	"MOVEGUN2",      
	"MOVEGUN1",      
	"NOWAY",         
	"NAZIHITPLAYER", 
	"SCHABBSTHROW",  
	"PLAYERDEATH",   
	"DOGDEATH",      
	"ATKGATLING",    
	"GETKEY",        
	"NOITEM",        
	"WALK1",         
	"WALK2",         
	"TAKEDAMAGE",    
	"GAMEOVER",      
	"OPENDOOR",      
	"CLOSEDOOR",     
	"DONOTHING",     
	"HALT",          
	"DEATHSCREAM2",  
	"ATKKNIFE",      
	"ATKPISTOL",     
	"DEATHSCREAM3",  
	"ATKMACHINEGUN", 
	"HITENEMY",      
	"SHOOTDOOR",     
	"DEATHSCREAM1",  
	"GETMACHINE",    
	"GETAMMO",       
	"SHOOT",         
	"HEALTH1",       
	"HEALTH2",       
	"BONUS1",        
	"BONUS2",        
	"BONUS3",        
	"GETGATLING",    
	"ESCPRESSED",    
	"LEVELDONE",     
	"DOGBARK",       
	"ENDBONUS1",     
	"ENDBONUS2",     
	"BONUS1UP",      
	"BONUS4",        
	"PUSHWALL",      
	"NOBONUS",       
	"PERCENT100",    
	"BOSSACTIVE",    
	"MUTTI",         
	"SCHUTZAD",      
	"AHHHG",         
	"DIE",           
	"EVA",           
	"GUTENTAG",      
	"LEBEN",         
	"SCHEIST",       
	"NAZIFIRE",      
	"BOSSFIRE",      
	"SSFIRE",        
	"SLURPIE",       
	"TOT_HUND",      
	"MEINGOTT",      
	"SCHABBSHA",     
	"HITLERHA",      
	"SPION",         
	"NEINSOVAS",     
	"DOGATTACK",     
	"FLAMETHROWER",  
	"MECHSTEP",      
	"GOOBS",         
	"YEAH",          
	"DEATHSCREAM4",  
	"DEATHSCREAM5",  
	"DEATHSCREAM6",  
	"DEATHSCREAM7",  
	"DEATHSCREAM8",  
	"DEATHSCREAM9",  
	"DONNER",        
	"EINE",          
	"ERLAUBEN",      
	"KEIN",          
	"MEIN",          
	"ROSE",          
	"MISSILEFIRE",   
	"MISSILEHIT"
};

Gamebuino gb;

void setup(void) 
{
	gb.begin();
	//gb.titleScreen(F("WOLF3D DEMO\n\nControls:\n \25 strafe\n \26 run "));
	gb.display.persistence = false;
	gb.battery.show = false;
	gb.setFrameRate(28);

}

int currentPattern = 0;
int volume = 1;

void loop()
{
	if(gb.update())
	{
		gb.display.cursorX = 0;
		gb.display.cursorY = 0;
		gb.display.print(F("Pattern:"));
		gb.display.println(currentPattern);
		gb.display.print(patternNames[currentPattern]);
		gb.display.println(F("SND"));
		gb.display.print(F("Volume:"));
		gb.display.println(volume);
		if(gb.buttons.pressed(BTN_LEFT))
		{
			currentPattern--;
			if(currentPattern < 0)
				currentPattern = NUM_AUDIO_PATTERNS - 1;
		}
		if(gb.buttons.pressed(BTN_RIGHT))
		{
			currentPattern++;
			if(currentPattern >= NUM_AUDIO_PATTERNS)
				currentPattern = 0;
		}
		if(gb.buttons.pressed(BTN_A))
		{
			//gb.sound.command(CMD_INSTRUMENT, 0, 0, 0);
			//gb.sound.playPattern(pattern, 0);
			gb.sound.playPattern((const uint16_t*)pgm_read_word(&Data_AudioPatterns[currentPattern]), 0);
			gb.sound.command(CMD_VOLUME, volume, 0, 0);
		//	gb.sound.playNote(12, 1, 0);
		}
		if(gb.buttons.pressed(BTN_UP))
		{
			volume ++;
			if(volume == 11)
				volume = 0;
			gb.sound.playPattern(Data_Audio01, 0);
			gb.sound.command(CMD_VOLUME, volume, 0, 0);
		}
		if(gb.buttons.pressed(BTN_DOWN))
		{
			volume --;
			if(volume == -1)
				volume = 10;
			gb.sound.playPattern(Data_Audio01, 0);
			gb.sound.command(CMD_VOLUME, volume, 0, 0);
		}
	}
}
