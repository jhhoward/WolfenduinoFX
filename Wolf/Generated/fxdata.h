#pragma once

/**** FX data header generated by fxdata-build.py tool version 1.15 ****/

using uint24_t = __uint24;

// Initialize FX hardware using  FX::begin(FX_DATA_PAGE); in the setup() function.

constexpr uint16_t FX_DATA_PAGE  = 0xfc60;
constexpr uint24_t FX_DATA_BYTES = 237517;

constexpr uint24_t MapData = 0x000000;
constexpr uint24_t Data_audio = 0x028000;
constexpr uint24_t Data_pistolSprite = 0x0297BA;
constexpr uint24_t Data_machinegunSprite = 0x02A0E3;
constexpr uint24_t Data_chaingunSprite = 0x02A9E2;
constexpr uint24_t Data_knifeSprite = 0x02BB61;
constexpr uint24_t Data_itemSprites = 0x02C068;
constexpr uint24_t Data_guardSprite = 0x02C6EE;
constexpr uint24_t Data_dogSprite = 0x02D8AD;
constexpr uint24_t Data_ssSprite = 0x02E851;
constexpr uint24_t Data_bossSprite = 0x02FE89;
constexpr uint24_t Data_decorations = 0x031BE4;
constexpr uint24_t Data_blockingDecorations = 0x03284A;
constexpr uint24_t Data_wallTextures = 0x03304D;
constexpr uint24_t Data_uiSprite = 0x03984D;
