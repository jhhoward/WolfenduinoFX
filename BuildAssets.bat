set IMAGE_ENCODER=Debug\ImageEncoder.exe

%IMAGE_ENCODER% Assets\pistol.png DataHeaders\Data_Pistol.h Data_pistolSprite sprite
%IMAGE_ENCODER% Assets\machinegun.png DataHeaders\Data_Machinegun.h Data_machinegunSprite sprite
%IMAGE_ENCODER% Assets\knife.png DataHeaders\Data_Knife.h Data_knifeSprite sprite
%IMAGE_ENCODER% Assets\items.png DataHeaders\Data_Items.h Data_itemSprites sprite
%IMAGE_ENCODER% Assets\guard.png DataHeaders\Data_Guard.h Data_guardSprite sprite
%IMAGE_ENCODER% Assets\walls.png DataHeaders\Data_Walls.h Data_wallTextures texture
%IMAGE_ENCODER% Assets\decorations.png DataHeaders\Data_Decorations.h Data_decorations sprite
%IMAGE_ENCODER% Assets\blockingDecorations.png DataHeaders\Data_BlockingDecorations.h Data_blockingDecorations sprite
%IMAGE_ENCODER% Assets\font.png DataHeaders\Data_Font.h Data_font font

set AUDIO_ENCODER=Debug\AudioEncoder.exe
set AUD=Assets\RawAudio\audio

%AUDIO_ENCODER% DataHeaders\Data_Audio.h %AUD%18.raw %AUD%19.raw %AUD%24.raw %AUD%58.raw %AUD%31.raw 
rem %AUDIO_ENCODER% DataHeaders\Data_Audio.h %AUD%