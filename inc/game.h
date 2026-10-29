#pragma once 

#include <random>
#include "entity.h"
#include "entity_manager.h"
#include <SFML/Graphics.hpp>

struct WindowConfig {int FL, FS, BR, BG, BB; unsigned int W, H;};
struct PlayerConfig {int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S;};
struct EnemyConfig  {int SR, CR, OR, OG, OB, OT, VMIN, VMAX, SL, SI; float SMIN, SMAX;};
struct BulletConfig {int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S;};

class Game
{
	sf::RenderWindow m_window;						// the window we will draw to
	sf::Font		 m_font;						// the font we will us to draw
	sf::Text		 m_text;						// the score text to be drawn to the scene
	EntityManager	 m_entities;					// vector of entities to maintain
	WindowConfig	 m_windowConfig;
	PlayerConfig	 m_playerConfig;
	EnemyConfig		 m_enemyConfig;
	BulletConfig	 m_bulletConfig;
	int				 m_score = 0;
	int				 m_currentFrame = 0;
	int				 m_lastEnemySpawnTime = 0;
	bool 			 m_paused = false;				// pauses game logic updating
	bool 			 m_running = true;				// if the game is running or not
	std::mt19937	 m_rng;							// random number generator
	
	std::shared_ptr<Entity> m_player;				// pointer to the player
	
	void init(const std::string & config);			// initialize the GameState with a config file path
	void setPaused(bool& paused);					// pause the game
	
	void sMovement();								// system: entity position and movement updating
	void sUserInput();								// system: user input
	void sLifespan();								// system: lifespan
	void sRender(); 								// system: rendering and drawing
	void sEnemySpawner(); 							// system: spawns enemies
	void sCollision();								// system: collisions
	
	void spawnPlayer();
	void spawnEnemy();
	void spawnSmallEnemies(std::shared_ptr<Entity> entity);
	void spawnBullet(std::shared_ptr<Entity> entity, const Vec2 & mousePos);
	void spawnSpecialWeapon(std::shared_ptr<Entity> entity, const Vec2 & mousePos);
	
public:
	Game(const std::string & config);				// constructor, takes in game config
	
	void run();
};