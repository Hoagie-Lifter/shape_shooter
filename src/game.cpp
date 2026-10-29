#include "game.h"
#include <fstream>
#include <iostream>
#include <string>

#include <SFML/Graphics.hpp>

Game::Game(const std::string & config)
	: m_text(m_font)
{
	// random number generator seeder thing?
	std::random_device seeder;
	m_rng.seed(seeder());
	
	init(config);
}

void Game::init(const std::string & path)
{
	// TODO: read in config file here
	// use premade PlayerConfig, EnemyConig, ulletConfig, variables
	
	// implementation
	if (!m_font.openFromFile("fonts/PPFraktionMonoBold.otf"))
	{
		std::cout << "Could not load font.\n";
	}
	
	std::ifstream fin(path);
	std::string token;
	while (fin >> token)
	{
		if (token == "Window:")
		{
			// int FL, FS, BR, BG, BB; unsigned int W, H;
			fin >> m_windowConfig.W;
			fin >> m_windowConfig.H;
			fin >> m_windowConfig.FL;
			fin >> m_windowConfig.FS;
			fin >> m_windowConfig.BR;
			fin >> m_windowConfig.BG;
			fin >> m_windowConfig.BB;
		}
		
		else if (token == "Text:")
		{
			// set default string
			//std::string score_text = "Score: ";
			//std::string score_string = score_text + std::to_string(m_score);
			
			// set default text scale
			int scale;
			fin >> scale;
			m_text.setCharacterSize(scale);
			
			// set default fill color
			int r;
			int g;
			int b;
			fin >> r >> g >> b;
			m_text.setFillColor(sf::Color(r, g, b));
		}
		
		else if (token == "Player:")
		{
			// int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S;
			fin >> m_playerConfig.SR;
			fin >> m_playerConfig.CR;
			fin >> m_playerConfig.FR;
			fin >> m_playerConfig.FG;
			fin >> m_playerConfig.FB;
			fin >> m_playerConfig.OR;
			fin >> m_playerConfig.OG;
			fin >> m_playerConfig.OB;
			fin >> m_playerConfig.OT;
			fin >> m_playerConfig.V;
			fin >> m_playerConfig.S;
			
		}
		
		else if (token == "Enemy:")
		{
			// int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX;
			fin >> m_enemyConfig.SR;
			fin >> m_enemyConfig.CR;
			fin >> m_enemyConfig.OR;
			fin >> m_enemyConfig.OG;
			fin >> m_enemyConfig.OB;
			fin >> m_enemyConfig.OT;
			fin >> m_enemyConfig.VMIN;
			fin >> m_enemyConfig.VMAX;
			fin >> m_enemyConfig.SI;
			fin >> m_enemyConfig.SL;
			fin >> m_enemyConfig.SMIN;
			fin >> m_enemyConfig.SMAX;
		}
		
		else if (token == "Bullet:")
		{
			// int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S;
			fin >> m_bulletConfig.SR;
			fin >> m_bulletConfig.CR;
			fin >> m_bulletConfig.FR;
			fin >> m_bulletConfig.FG;
			fin >> m_bulletConfig.FB;
			fin >> m_bulletConfig.OR;
			fin >> m_bulletConfig.OG;
			fin >> m_bulletConfig.OB;
			fin >> m_bulletConfig.OT;
			fin >> m_bulletConfig.V;
			fin >> m_bulletConfig.L;
			fin >> m_bulletConfig.S;
		}
	}
	
	// set up default window parameters
	m_window.create(sf::VideoMode({m_windowConfig.W, m_windowConfig.H}), "Shape Shooter", sf::State(m_windowConfig.FS));
	m_window.setFramerateLimit(m_windowConfig.FL);
	m_window.setKeyRepeatEnabled(false);
	
	spawnPlayer();
}

void Game::run()
{
	// TODO: add pause functionality in here
	// some systems should function while paused (rendering)
	// some systems shouldnt funtion while paused (movement, input)
	
	// implementation
	while (m_running)
	{
		m_entities.update();
		
		if (!m_paused)
		{
			sEnemySpawner();
			sMovement();
			sCollision();
			sLifespan();
			sUserInput();
			
			m_currentFrame++;
		}

		sUserInput();
		sRender();
	}
}

void Game::setPaused(bool& paused)
{
	paused = !paused;
}

// sets player location and appearance information
void Game::spawnPlayer()
{
	// create player entity by calling EntityManager.addEntity()
	// this returns a std::shared_ptr<Entity>, so we use 'auto' to save typing
	auto entity = m_entities.addEntity("player");
	
	// use window x and y / 2 to get coords for the middle of the screen
	float mx = m_window.getSize().x / 2.0f;
	float my = m_window.getSize().y / 2.0f;
	
	// setting player spawn using above coords and giving angle of 0.0
	entity -> cTransform = std::make_shared<CTransform>(Vec2(mx, my), Vec2(m_playerConfig.S, m_playerConfig.S), 0.0f);
	
	// adding shape component to player entity and setting shape radius, shape vertices, fill color,
	// outline color, and outline thickness
	entity -> cShape = std::make_shared<CShape>(
	m_playerConfig.SR,
	m_playerConfig.V,
	sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB),
	sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB),
	m_playerConfig.OT
	);
	
	// adding input component to player entity
	entity -> cInput = std::make_shared<CInput>();
	
	// since this entity will be the player, set games player variable to this entity
	// ONLY FOR THIS PROJECT: this goes against the EntityManager paradigm, but player is used so much that its worth it here
	m_player = entity;
}

// spawns enemies within screen bounds and according to config file
void Game::spawnEnemy()
{
	// create enemy entities by calling EntityManager.addEntity()
	auto entity = m_entities.addEntity("enemy");
	
	// random number between (shape radius) and window size x, and (shape radius) and window size y
	std::uniform_int_distribution<int> dist_x(m_enemyConfig.SR, m_window.getSize().x - m_enemyConfig.SR);
	std::uniform_int_distribution<int> dist_y(m_enemyConfig.SR, m_window.getSize().y - m_enemyConfig.SR);
	float rand_x = dist_x(m_rng);
	float rand_y = dist_y(m_rng);
	
	// random number between minimum and maximum speed  
	std::uniform_int_distribution<int> dist_dir_x(-(m_enemyConfig.SMAX), m_enemyConfig.SMAX);
	std::uniform_int_distribution<int> dist_dir_y(-(m_enemyConfig.SMAX), m_enemyConfig.SMAX);
	float rand_dir_x = 0;
	float rand_dir_y = 0;

	// only accept a value with below exclusions (default: between -2 and 10, excluding combonations of -1, 0, and 1)
	do
	{
		rand_dir_x = dist_dir_x(m_rng);
		rand_dir_y = dist_dir_y(m_rng);
	}	while (rand_dir_x >= -1 && rand_dir_x <= 1 && rand_dir_y >= -1 && rand_dir_y <= 1);

	// enemy entity will spawn randomly within bounds of window with velocity (1, 1) and angle 1
	entity -> cTransform = std::make_shared<CTransform>(Vec2(rand_x, rand_y), Vec2(rand_dir_x, rand_dir_y), 0);
	
	// random numbers for vertices and RGB values
	std::uniform_int_distribution<int> dist_vertices(m_enemyConfig.VMIN, m_enemyConfig.VMAX);
	std::uniform_int_distribution<int> dist_r(0, 255);
	std::uniform_int_distribution<int> dist_g(0, 255);
	std::uniform_int_distribution<int> dist_b(0, 255);
	int rand_vertices = dist_vertices(m_rng);
	int rand_r = dist_r(m_rng);
	int rand_g = dist_g(m_rng);
	int rand_b = dist_b(m_rng);
	
	// adding shape component to enemy entities and setting shape radius, random shape vertices,
	// random fill color, outline color, and outline thickness
	entity -> cShape = std::make_shared<CShape>(
	m_enemyConfig.SR,
	rand_vertices,
	sf::Color(rand_r, rand_g, rand_b),
	sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB),
	m_enemyConfig.OT
	);
	
	entity -> cScore = std::make_shared<CScore>(rand_vertices * 10);
	
	//std::cout << entity -> cScore -> score << "\n";
	
	// sets the current frame as the time that the last enemy spawned
	// sEnemySpawner() - used to set spawn interval in conjunction with m_enemyConfig.SI 
	m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
	// TODO: Spawn small enemies at the location of the input enemy eback
	// when we create the smaller enemies, we must read the values of the original enemy
	// set each smaller enemy to the same color as the original and half the size
	// small enemies are worth double the points of the original enemies when hit
	
	float enemy_count = e -> cShape -> circle.getPointCount();
	std::vector<float> angles;
	float step = 360.0f / enemy_count;
	for (int i = 0; i < enemy_count; ++i)
	{
		float current_angle = 360.0f - (i * step);
		angles.push_back(current_angle);
	}
	
	for (auto a : angles)
	{
		auto entity = m_entities.addEntity("small_enemy");
		
		float radians = a * (3.14159265f / 180.0f);
		Vec2 direction = { std::cos(radians), std::sin(radians) };
		Vec2 velocity = direction * m_enemyConfig.SMAX;
		
		entity -> cLifespan = std::make_shared<CLifespan>(m_enemyConfig.SL);
	
		entity -> cScore = std::make_shared<CScore>(e -> cScore -> score * 2);
	
		entity -> cShape = std::make_shared<CShape>(
		e -> cShape -> circle.getRadius() / 2,
		e -> cShape -> circle.getPointCount(),
		sf::Color(e -> cShape -> circle.getFillColor()),
		sf::Color(e -> cShape -> circle.getOutlineColor()),
		e -> cShape -> circle.getOutlineThickness() / 2
		);
		
		entity -> cTransform = std::make_shared<CTransform>(Vec2(e -> cTransform -> pos), velocity, 0);
		
		//std::cout << a << "\n";
	}
}

void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2 & target)
{
	// TODO: implement the spawning of a bullet that travels towards targets
	// bullet speed is given as a scalar speed
	// must set velocity by using formula in notes

	
	auto bullet = m_entities.addEntity("bullet");
	
	Vec2 direction = target.dir(entity -> cTransform -> pos);
	Vec2 velocity = direction * m_bulletConfig.S;
	
	bullet -> cTransform = std::make_shared<CTransform>(entity -> cTransform -> pos, velocity, 0);

	bullet -> cShape = std::make_shared<CShape>(
	m_bulletConfig.SR,
	m_bulletConfig.V,
	sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB),
	sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB),
	m_bulletConfig.OT
	);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity, const Vec2 & target)
{
	float bullet_count = 10;
	std::vector<float> angles;
	float step = 360.0f / bullet_count;
	for (int i = 0; i < bullet_count; ++i)
	{
		float current_angle = 360.0f - (i * step);
		angles.push_back(current_angle);
	}
	
	for (auto a : angles)
	{
		auto bullet = m_entities.addEntity("bullet");
		
		float radians = a * (3.14159265f / 180.0f);
		Vec2 direction = {std::cos(radians), std::sin(radians)};
		Vec2 velocity = direction * m_bulletConfig.S;
		
		bullet -> cLifespan = std::make_shared<CLifespan>(120);
	
		bullet -> cShape = std::make_shared<CShape>(
		m_bulletConfig.SR / 2,
		m_bulletConfig.V,
		sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB),
		sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB),
		m_bulletConfig.OT / 2
		);
		
		bullet -> cTransform = std::make_shared<CTransform>(Vec2(entity -> cTransform -> pos), velocity, 0);
		
		//std::cout << a << "\n";
	}
}

void Game::sMovement()
{
	// TODO: implement all entity movement functionality
	// you should read the m_player -> cInput component to determine if the player is moving
	
	// implementation
	m_player -> cTransform -> velocity = {0, 0};
	
	if (m_player -> cInput -> up)
	{
		m_player -> cTransform -> velocity.y = -m_playerConfig.S;
	}
	
	if (m_player -> cInput -> left)
	{
		m_player -> cTransform -> velocity.x = -m_playerConfig.S;
	}
	
	if (m_player -> cInput -> down)
	{
		m_player -> cTransform -> velocity.y = m_playerConfig.S;
	}
	
	if (m_player -> cInput -> right)
	{
		m_player -> cTransform -> velocity.x = m_playerConfig.S;
	}
	
	if (m_player -> cInput -> up && m_player -> cInput -> right)
	{
		m_player -> cTransform -> velocity.y = -m_playerConfig.S * 0.7071f;
		m_player -> cTransform -> velocity.x = m_playerConfig.S  * 0.7071f;
	}
	
	if (m_player -> cInput -> up && m_player -> cInput -> left)
	{
		m_player -> cTransform -> velocity.y = -m_playerConfig.S * 0.7071f;
		m_player -> cTransform -> velocity.x = -m_playerConfig.S * 0.7071f;
	}
	
	if (m_player -> cInput -> down && m_player -> cInput -> right)
	{
		m_player -> cTransform -> velocity.y = m_playerConfig.S  * 0.7071f;
		m_player -> cTransform -> velocity.x = m_playerConfig.S  * 0.7071f;
	}
	
	if (m_player -> cInput -> down && m_player -> cInput -> left)
	{
		m_player -> cTransform -> velocity.y = m_playerConfig.S  * 0.7071f;
		m_player -> cTransform -> velocity.x = -m_playerConfig.S * 0.7071f;
	}
	
	// update positions for all entities in entity vector
	
	for (auto e : m_entities.getEntities("enemy"))
	{
		e -> cTransform -> pos.x += e -> cTransform -> velocity.x;
		if (e -> cTransform -> pos.x <= 0)
			e -> cTransform -> velocity.x *= -1;
		if (e -> cTransform -> pos.x >= m_window.getSize().x)
			e -> cTransform -> velocity.x *= -1;
		
		e -> cTransform -> pos.y += e -> cTransform -> velocity.y;
		if (e -> cTransform -> pos.y <= 0)
			e -> cTransform -> velocity.y *= -1;
		if (e -> cTransform -> pos.y >= m_window.getSize().y)
			e -> cTransform -> velocity.y *= -1;
	}
	
	for (auto e : m_entities.getEntities("small_enemy"))
	{
		e -> cTransform -> pos.x += e -> cTransform -> velocity.x;
		if (e -> cTransform -> pos.x <= 0)
			e -> cTransform -> velocity.x *= -1;
		if (e -> cTransform -> pos.x >= m_window.getSize().x)
			e -> cTransform -> velocity.x *= -1;
		
		e -> cTransform -> pos.y += e -> cTransform -> velocity.y;
		if (e -> cTransform -> pos.y <= 0)
			e -> cTransform -> velocity.y *= -1;
		if (e -> cTransform -> pos.y >= m_window.getSize().y)
			e -> cTransform -> velocity.y *= -1;
	}
	
	for (auto & b : m_entities.getEntities("bullet"))
	{
		b -> cTransform -> pos += b -> cTransform -> velocity;
		
		bool offScreenX = b -> cTransform -> pos.x <= 0 || b -> cTransform -> pos.x >= m_window.getSize().x;
		bool offScreenY = b -> cTransform -> pos.y <= 0 || b -> cTransform -> pos.y >= m_window.getSize().y;
			
		if (offScreenX || offScreenY)
		{
			b -> destroy();
			//std::cout << "A bullet has left the screen.\n";
		}
	}
	
	// update position.x for player entity, clamping movement to screen bounds
	m_player -> cTransform -> pos.x += m_player -> cTransform -> velocity.x;
	if (m_player -> cTransform -> pos.x <= 0)
	{
		m_player -> cTransform -> pos.x = 0;
	}
	
	else if (m_player -> cTransform -> pos.x >= m_window.getSize().x)
	{
		m_player -> cTransform -> pos.x = m_window.getSize().x;
	}
	
	// update position.y for player entity, clamping movement to screen bounds
	m_player -> cTransform -> pos.y += m_player -> cTransform -> velocity.y;
	if (m_player -> cTransform -> pos.y <= 0)
	{
		m_player -> cTransform -> pos.y = 0;
	}
	
	else if (m_player -> cTransform -> pos.y >= m_window.getSize().y)
	{
		m_player -> cTransform -> pos.y = m_window.getSize().y;
	}
}

void Game::sEnemySpawner()
{
	// checks if required amount of frames has passed to spawn a new enemy
	if (m_currentFrame == m_lastEnemySpawnTime + m_enemyConfig.SI)
	{
		spawnEnemy();
	}
}

void Game::sLifespan()
{
	// TODO: implement all lifespan functionality
	// for all entities:
	// if entity has no lifespan component, skip it
	// if entity has > 0 remaining lifespan, subtract 1
	// if entity has lifespan and is alive, scale its alpha channel properly
	// if entity has lifespan and its time is up, destroy that entity
	
	for (auto e : m_entities.getEntities("small_enemy"))
	{
		if (e -> cLifespan -> remaining > 0)
		{
			sf::Color new_alphaF = e -> cShape -> circle.getFillColor();
			new_alphaF.a = static_cast<float>(e -> cLifespan -> remaining) / static_cast<float>(e -> cLifespan -> total) * 255;
			e -> cShape -> circle.setFillColor(new_alphaF);
			
			sf::Color new_alphaO = e -> cShape -> circle.getOutlineColor();
			new_alphaO.a = static_cast<float>(e -> cLifespan -> remaining) / static_cast<float>(e -> cLifespan -> total) * 255;
			e -> cShape -> circle.setOutlineColor(new_alphaO);
			
			e -> cLifespan -> remaining--;
		}
		
		else if (e -> cLifespan -> remaining <= 0)
		{
			e -> destroy();
		}
	}
}

void Game::sCollision()
{
	// TODO: implement all proper collisions between entities
	// Be sure to use the collision radius, NOT the shape radius
	
	// implementation
	for (auto e : m_entities.getEntities("enemy"))
	{
		float dist_p = e -> cTransform -> pos.dist(m_player -> cTransform -> pos);
		
		if (dist_p <= m_enemyConfig.CR + m_playerConfig.CR)
		{
			m_player -> destroy();

			std::cout << "GAME OVER!\n";
		}
	}
	
	for (auto b : m_entities.getEntities("bullet"))
	{
		if (!b -> isActive()) continue;
		
		for (auto e : m_entities.getEntities("enemy"))
		{
			if (!e -> isActive()) continue;
			
			float dist_e = e -> cTransform -> pos.dist(b -> cTransform -> pos);
			
			if (dist_e <= m_enemyConfig.CR + m_bulletConfig.CR)
			{
				e -> destroy();
				b -> destroy();
				m_score += e -> cScore -> score;
				spawnSmallEnemies(e);
				
				//std::cout << "A bullet has collided with an enemy!\n";
			}
			
			float dist_p = e -> cTransform -> pos.dist(m_player -> cTransform -> pos);
			
			if (dist_p <= m_enemyConfig.CR + m_playerConfig.CR)
			{
				m_player -> destroy();

				std::cout << "GAME OVER!\n";
			}
			
			if (!b -> isActive()) continue;
			
			for (auto se : m_entities.getEntities("small_enemy"))
			{
				if (!se -> isActive()) continue;
				
				float dist_se = se -> cTransform -> pos.dist(b -> cTransform -> pos);
			
				if (dist_se <= m_enemyConfig.CR + m_bulletConfig.CR)
				{
					se -> destroy();
					b -> destroy();
					m_score += se -> cScore -> score;
				
					//std::cout << "A bullet has collided with a small enemy!\n";
				}
			}
		}
	}
}

void Game::sRender()
{
	// clear screen and set backgound color
	m_window.clear(sf::Color(m_windowConfig.BR, m_windowConfig.BG, m_windowConfig.BB));
	
	// draw all entities
	for (auto e : m_entities.getEntities())
	{
		// set the position of the shape based on the entitys transform -> pos
		e -> cShape -> circle.setPosition({e -> cTransform -> pos.x, e -> cTransform -> pos.y});
		
		// set the rotation of the shape based on the entitys transform -> angle
		e -> cTransform -> angle += 1.0f;
		e -> cShape -> circle.setRotation(sf::degrees(e -> cTransform -> angle));
		
		// draw entitys sf::CircleShape
		m_window.draw(e -> cShape -> circle);
	}

	std::string score_string = "Score: " + std::to_string(m_score);
	m_text.setString(score_string);
	
	// draw text set in Game::Game
	m_window.draw(m_text);
	
	// display everything drawn
	m_window.display();
}

void Game::sUserInput()
{
	// TODO: handle user input here
	// should only be setting players input component variables here
	// should not implement the players movement logic here
	// the movement system will read the variables you set in this function
	
	while (const std::optional event = m_window.pollEvent())
	{
		// this event triggers when the window is closed
		if (event -> is<sf::Event::Closed>())
		{
			m_running = false;
		}
		
		else if (const auto * keyPressed = event -> getIf<sf::Event::KeyPressed>())
		{
			if (keyPressed -> scancode == sf::Keyboard::Scancode::Escape)
			{
				m_window.close();
				
				m_running = false;
			}
			
			else if (keyPressed -> scancode == sf::Keyboard::Scancode::W)
				m_player -> cInput -> up = true;
			
			else if (keyPressed -> scancode == sf::Keyboard::Scancode::A)
				m_player -> cInput -> left = true;
			
			else if (keyPressed -> scancode == sf::Keyboard::Scancode::S)
				m_player -> cInput -> down = true;
			
			else if (keyPressed -> scancode == sf::Keyboard::Scancode::D)
				m_player -> cInput -> right = true;
			
			else if (keyPressed -> scancode == sf::Keyboard::Scancode::P)
				setPaused(m_paused);
		}
		
		else if (const auto * keyReleased = event -> getIf<sf::Event::KeyReleased>())
		{
			if (keyReleased -> scancode == sf::Keyboard::Scancode::W)
				m_player -> cInput -> up = false;
			
			else if (keyReleased -> scancode == sf::Keyboard::Scancode::A)
				m_player -> cInput -> left = false;
			
			else if (keyReleased -> scancode == sf::Keyboard::Scancode::S)
				m_player -> cInput -> down = false;
			
			else if (keyReleased -> scancode == sf::Keyboard::Scancode::D)
				m_player -> cInput -> right = false;
		}
		
		else if (const auto * mouseButtonPressed = event -> getIf<sf::Event::MouseButtonPressed>())
		{
			if (mouseButtonPressed -> button == sf::Mouse::Button::Left)
			{
				if (!m_paused)
				{
					// getting mouse position variables
					// vector2f conversion so that window resizing doesnt break mouse position detection
					sf::Vector2f mouse_pos = m_window.mapPixelToCoords(mouseButtonPressed -> position);
				
					spawnBullet(m_player, Vec2(mouse_pos.x, mouse_pos.y));
				}
			}
			
			if (mouseButtonPressed -> button == sf::Mouse::Button::Right)
			{
				if (!m_paused)
				{
					// getting mouse position variables
					// vector2f conversion so that window resizing doesnt break mouse position detection
					sf::Vector2f mouse_pos = m_window.mapPixelToCoords(mouseButtonPressed -> position);
				
					spawnSpecialWeapon(m_player, Vec2(mouse_pos.x, mouse_pos.y));
				}
			}
		}
		
		else if (const auto * mouseButtonReleased = event -> getIf<sf::Event::MouseButtonReleased>())
		{
			if (mouseButtonReleased -> button == sf::Mouse::Button::Left)
			{
				//std::cout << "Left mouse button released\n";
			}
			
			if (mouseButtonReleased -> button == sf::Mouse::Button::Right)
			{
				//std::cout << "Right mouse button released\n";
			}
		}
	}
}