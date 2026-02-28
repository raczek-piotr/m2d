#include "headers.h"


using namespace std;



void drawVoxel0(sf::RenderWindow &window, s32 x, s32 y) { // air
	//sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
	//rect.setPosition(x, y);
	//rect.setFillColor(sf::Color(150, 150, 255));
	//window.draw(rect);
}
void drawVoxel1(sf::RenderWindow &window, s32 x, s32 y) { // grass
	sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
	rect.setPosition(x, y);
	rect.setFillColor(sf::Color(100, 250, 100));
	window.draw(rect);
}
void drawVoxel2(sf::RenderWindow &window, s32 x, s32 y) { // dirt
	sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
	rect.setPosition(x, y);
	rect.setFillColor(sf::Color(140, 70, 20));
	window.draw(rect);
}
void drawVoxel3(sf::RenderWindow &window, s32 x, s32 y) { // stone
	sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
	rect.setPosition(x, y);
	rect.setFillColor(sf::Color(128, 128, 128));
	window.draw(rect);
}
void drawVoxel4(sf::RenderWindow &window, s32 x, s32 y) { // sand
	sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
	rect.setPosition(x, y);
	rect.setFillColor(sf::Color(0, 100, 255));
	window.draw(rect);
}
void drawVoxel5(sf::RenderWindow &window, s32 x, s32 y) { // ?
	drawVoxel0(window, x, y);
	{
		sf::RectangleShape rect(sf::Vector2f(TILE_SIZE/5, TILE_SIZE));
		rect.setPosition(x+TILE_SIZE/5, y);
		rect.setFillColor(sf::Color(200, 120, 60));
		window.draw(rect);
	}{
		sf::RectangleShape rect(sf::Vector2f(TILE_SIZE/5, TILE_SIZE));
		rect.setPosition(x+3*TILE_SIZE/5, y);
		rect.setFillColor(sf::Color(200, 120, 60));
		window.draw(rect);
	}{
		sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE/6));
		rect.setPosition(x, y+TILE_SIZE/6);
		rect.setFillColor(sf::Color(200, 120, 60));
		window.draw(rect);
	}{
		sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE/6));
		rect.setPosition(x, y+4*TILE_SIZE/6);
		rect.setFillColor(sf::Color(200, 120, 60));
		window.draw(rect);
	}
}
void drawVoxel6(sf::RenderWindow &window, s32 x, s32 y) {
	sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
	rect.setPosition(x, y);
	rect.setFillColor(sf::Color(255, 80, 0));
	window.draw(rect);
}


void (*descriptor[])(sf::RenderWindow &window, s32 x, s32 y) = { drawVoxel0, drawVoxel1, drawVoxel2, drawVoxel3, drawVoxel4, drawVoxel5, drawVoxel6 };








void BG_air(sf::RenderWindow &window, s32 x, s32 y) { // BG 0
	sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
	rect.setPosition(x, y);
	rect.setFillColor(sf::Color(180, 180, 255));
	window.draw(rect);
}
void BG_grass(sf::RenderWindow &window, s32 x, s32 y) { // BG 0
	sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
	rect.setPosition(x, y);
	rect.setFillColor(sf::Color(50, 125, 50));
	window.draw(rect);
}
void BG_dirt(sf::RenderWindow &window, s32 x, s32 y) { // BG 0
	sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
	rect.setPosition(x, y);
	rect.setFillColor(sf::Color(70, 35, 10));
	window.draw(rect);
}
void BG_stone(sf::RenderWindow &window, s32 x, s32 y) { // BG 0
	sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
	rect.setPosition(x, y);
	rect.setFillColor(sf::Color(64, 64, 64));
	window.draw(rect);
}
void BG_sand(sf::RenderWindow &window, s32 x, s32 y) { // BG 0
	sf::RectangleShape rect(sf::Vector2f(TILE_SIZE, TILE_SIZE));
	rect.setPosition(x, y);
	rect.setFillColor(sf::Color(200, 180, 0));
	window.draw(rect);
}

void (*BGdescriptor[])(sf::RenderWindow &window, s32 x, s32 y) = { BG_air, BG_grass, BG_dirt, BG_stone, BG_sand };














void drawPlayer(sf::RenderWindow &window, float prot) {
    {
        sf::RectangleShape rect(sf::Vector2f(TILE_SIZE/3, TILE_SIZE/3));
        rect.setPosition(screenXhalf, screenYhalf);
        rect.setFillColor(sf::Color::Blue);
        rect.setRotation(prot);
        window.draw(rect);
	}{
        sf::RectangleShape rect(sf::Vector2f(TILE_SIZE/3, TILE_SIZE/3));
        rect.setPosition(screenXhalf, screenYhalf);
        rect.setFillColor(sf::Color::Black);
        rect.setRotation(prot+90);
        window.draw(rect);
	}{
        sf::RectangleShape rect(sf::Vector2f(TILE_SIZE/3, TILE_SIZE/3));
        rect.setPosition(screenXhalf, screenYhalf);
        rect.setFillColor(sf::Color::Red);
        rect.setRotation(prot+180);
        window.draw(rect);
	}{
        sf::RectangleShape rect(sf::Vector2f(TILE_SIZE/3, TILE_SIZE/3));
        rect.setPosition(screenXhalf, screenYhalf);
        rect.setFillColor(sf::Color::Black);
        rect.setRotation(prot-90);
        window.draw(rect);
	}
}
