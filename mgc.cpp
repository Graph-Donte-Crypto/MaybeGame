#ifdef WIN32
#include <SFML/graphics.hpp>
#else 
#include <SFML/Graphics.hpp>
#endif
#include <UseFull/Math/Vector.hpp>
#include <UseFull/SFMLUp/EventHandler.hpp>
#include <UseFull/Utils/Bytes.hpp>
#include <UseFull/Utils/Macro.hpp>
#include <UseFull/Templates/Array.hpp>

struct GameLoopContext;

struct GameObject {
	math::Vector<2> position;
	math::Vector<2> speed;
	math::Vector<2> delta;
	math::Vector<2> collision_size;
	
	bool deleteFlag = false;
	
	virtual void updateBase() = 0;
	virtual void updateVectors() = 0;
	virtual void updateMove() = 0;
	virtual void draw() = 0;
	
	virtual ~GameObject() {}
};

bool checkIntersection2Rectangles(
	const math::Vector<2> & a0,
	const math::Vector<2> & a1,
	const math::Vector<2> & b0,
	const math::Vector<2> & b1) {
#define EPS 0.001
	bool fline13 = ( (a0[0] < b1[0] + EPS) && (a0[0] + EPS > b0[0] || a1[0] + EPS > b1[0]) ) || ( (b0[0] < a1[0] + EPS) && (a0[0] < b0[0] + EPS || a1[0] < b1[0] + EPS) );
	bool fline24 = ( (a0[1] < b1[1] + EPS) && (a0[1] + EPS > b0[1] || a1[1] + EPS > b1[1]) ) || ( (b0[1] < a1[1] + EPS) && (a0[1] < b0[1] + EPS || a1[1] < b1[1] + EPS) );
#undef EPS
	return fline24 && fline13;
}

namespace drawer {
	sf::Vertex vertexes[2];
}

struct GlobalStruct {
	
	uft::Array<GameObject *> game_objects;
	
	const char * window_name = "MaybeGame 0.1";
	sf::RenderWindow window;
	
	sf::View view;
	size_t limit_fps = 60;
	
	sf::Clock clock;
	double current_time;
	double current_fps;
	
	void calcFps() {
		current_time = clock.restart().asSeconds();
        current_fps = (double)1.0 / current_time;
	}
	
	void createWindow(size_t mode) {
		window.create(sf::VideoMode(768, 576), window_name, mode);
		//window.create(sf::VideoMode(1024, 768), window_name, mode);
		window.setFramerateLimit(limit_fps);
	}
	
	GlobalStruct() {
		//createWindow(sf::Style::Fullscreen);
		createWindow(sf::Style::Close);
	}
	
	void stepUpdate() {
		for (size_t i = 0; i < game_objects._length; i++) {
			if (game_objects[i]->deleteFlag) {
				delete game_objects[i];
				game_objects.remove(i);
				i -= 1;
			}
		}
		game_objects.foreach([](GameObject ** go){(*go)->updateBase();});
		//After updateBase we know speed
		game_objects.foreach([](GameObject ** go){(*go)->updateVectors();});
		//After updateVectors we know delta
		game_objects.foreach([](GameObject ** go){(*go)->updateMove();});
		//After updateMove we know new position
	}
	void stepDraw() {
		game_objects.foreach([](GameObject ** go){(*go)->draw();});
	}
	
	void drawLine(const math::Vector<2> & p1, const math::Vector<2> & p2) {
		drawer::vertexes[0] = sf::Vertex(sf::Vector2f(p1[0], p1[1]), sf::Color::White);
		drawer::vertexes[1] = sf::Vertex(sf::Vector2f(p2[0], p2[1]), sf::Color::White);
		window.draw(drawer::vertexes, 2, sf::Lines);
	}
	
	void drawRectangle(const math::Vector<2> & left_up, const math::Vector<2> & right_down) {
		drawLine(left_up, {left_up[0], right_down[1]});
		drawLine({left_up[0], right_down[1]}, right_down);
		drawLine(right_down, {right_down[0], left_up[1]});
		drawLine({right_down[0], left_up[1]}, left_up);
	}
	
	size_t gameLoop(GameLoopContext & context);
} Global;

const char * game_error = nullptr;

using namespace utils;

struct Projectile : public GameObject {
	uft::Array<GameObject *> ignored_targets = uft::Array<GameObject *>(8);
	uft::Array<GameObject *> hurt_targets = uft::Array<GameObject *>(4);
	
	virtual void collisionAction(GameObject * go) = 0;
	virtual void afterCollisionAction() = 0;
	
	Projectile() {
		ignored_targets.addCopy(this);
	}
	
	void intersectCheck() {
		for (size_t i = 0; i < Global.game_objects._length; i++) {
			GameObject * obj = Global.game_objects[i];
			
			Ok<size_t> ok = ignored_targets.indexByCondition([&obj](GameObject ** go1) -> bool{
				return obj == *go1;
			});
			if (ok.isOk) continue;
			
			if (checkIntersection2Rectangles(
				position + delta, 
				position + collision_size + delta,
				obj->position + obj->delta,
				obj->position + obj->collision_size + obj->delta
				)) {
				hurt_targets.addCopy(this);
				collisionAction(obj);
				afterCollisionAction();
				return;
			}
		}
	}
};

struct Bullet : public Projectile {
	size_t active_time = 60;
	
	Bullet(GameObject * master) : Projectile() {
		collision_size = {10, 10};
		position = master->position + (master->collision_size - collision_size) / 2;
		ignored_targets.addCopy(master);
	}
	
	virtual void updateBase() {
		if (active_time == 0) deleteFlag = true;
		else active_time -= 1;
	};
	virtual void updateVectors() {
		delta = speed;
	}
	virtual void updateMove() {
		intersectCheck();
		position += delta;
	}
	virtual void draw() {
		Global.drawRectangle(position, position + collision_size);
	}
	virtual void collisionAction(GameObject * go) {
		go->speed += speed / 3;
	}
	virtual void afterCollisionAction() {
		deleteFlag = true;
	}
};


struct RandomObject : public GameObject {
	RandomObject(const math::Vector<2> & vec) {
		position = vec;
		collision_size = {30, 30};
	}
	virtual void updateBase() {
		speed *= 0.9;
	}
	virtual void updateVectors() {
		delta = speed;
	}
	virtual void updateMove() {
		position += delta;
	}
	virtual void draw() {
		Global.drawRectangle(position, position + collision_size);
	}
};

struct Player : public GameObject {
	double max_speed;
	
	size_t reload_time = 10, reload_time_current = 0;
	
	Player() {
		position = {100, 100};
		speed = {0, 0};
		collision_size = {40, 50};
		max_speed = 5;
	}
	
	struct Input {
		Player * player = nullptr;
		
		Bits8 movement;
		Bits8 atack;
		
		Input(Player * player) {
			this->player = player;
			player->input = this;
		}
		~Input() {player->input = nullptr;}
		
	} * input = nullptr;
	
	void updateBase() {		
		if (!input) {
			printf("Player::action::input == nullptr\n");
			exit(1);
		}
		
		if (input->movement[0] == 1) speed[1] -= max_speed / 8;//W
		if (input->movement[1] == 1) speed[0] -= max_speed / 8;//A
		if (input->movement[2] == 1) speed[1] += max_speed / 8;//S
		if (input->movement[3] == 1) speed[0] += max_speed / 8;//D
		
		speed *= 0.9;
		speed.truncateTo(max_speed);	
		
		if (reload_time_current > 0) reload_time_current -= 1;
		else if (input->atack.getByte() != 0) {
			//reload_time_current = reload_time;
			math::Vector<2> direction = {0, 0};
			
			direction[1] -= input->atack[0];//up, y--
			direction[0] -= input->atack[1];//left, x--
			direction[1] += input->atack[2];//down, y++
			direction[0] += input->atack[3];//right, x++
			
			if (abs(direction.norm()) > 0.001) {
				reload_time_current = reload_time;
				Bullet * bullet = new Bullet(this);
				bullet->speed = 10 * direction.ort() + speed;
				Global.game_objects.addCopy(bullet);
			}
		}
	}
	
	void updateVectors() {
		delta = speed;
	}
	
	void updateMove() {
		position += delta;
	}
	
	void draw() {
		Global.drawRectangle(position, position + collision_size);
	}
};

struct GameLoopContext {
	Player * local_player;
};

using namespace sfup;

size_t GlobalStruct::gameLoop(GameLoopContext & context) {
	Player::Input local_input(context.local_player);
	
	while (window.isOpen()) {
		
        //EventKeeper processor

        Event.flush();
        Event.load(window);
		
		local_input.movement = 0;
		local_input.atack = 0;
		
		if (Event.KeyPressing[sf::Keyboard::Key::W]) local_input.movement.setBit(0, 1);
		if (Event.KeyPressing[sf::Keyboard::Key::A]) local_input.movement.setBit(1, 1);
		if (Event.KeyPressing[sf::Keyboard::Key::S]) local_input.movement.setBit(2, 1);
		if (Event.KeyPressing[sf::Keyboard::Key::D]) local_input.movement.setBit(3, 1);
		
		if (Event.KeyPressing[sf::Keyboard::Key::Up])    local_input.atack.setBit(0, 1);
		if (Event.KeyPressing[sf::Keyboard::Key::Left])  local_input.atack.setBit(1, 1);
		if (Event.KeyPressing[sf::Keyboard::Key::Down])  local_input.atack.setBit(2, 1);
		if (Event.KeyPressing[sf::Keyboard::Key::Right]) local_input.atack.setBit(3, 1);

        //Action processor

        stepUpdate();

        //View processor

        //Global.SView.step();

        //Clear screen

        window.clear(sf::Color::Black);

        //Draw Processor

        stepDraw();
		calcFps();
		
        //Display part

        window.display();
    }
	
	return 0;
}


int main() {	
	Player player;
	GameLoopContext context;;
	context.local_player = &player;
	RandomObject ro({250, 250});
	Global.game_objects.addCopy(&ro);
	Global.game_objects.addCopy(context.local_player);
	size_t game_status = Global.gameLoop(context);
	
	switch (game_status) {
		case 0: 
			return 0;
		default:
			printf("ErrorCode: %lu\nMessage: %s\n",(long unsigned)game_status ,game_error == nullptr ? "None" : game_error);
			break;
	}
	
}
