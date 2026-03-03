#pragma once


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int32_t s32;


const int TILE_SIZE = 48;
//const int MARGIN = TILE_SIZE/4;
const s32 CHUNK_SIZE = 8;
const s32 chunkMarginX = 3;
const s32 chunkMarginY = 2;
const s32 worldMapX = 1+2*chunkMarginX;
const s32 worldMapY = 1+2*chunkMarginY;
const s32 worldMapMSX = ((u8)-1)/CHUNK_SIZE-worldMapX+1;
const s32 worldMapMSY = ((u8)-1)/CHUNK_SIZE-worldMapY+1;
const int screenX = 1200;
const int screenY = 800;
const int screenXhalf = screenX/2;
const int screenYhalf = screenY/2;


const float speed = TILE_SIZE/6; // blocks per second
const float jump = TILE_SIZE/2;
const float gravity = TILE_SIZE;
