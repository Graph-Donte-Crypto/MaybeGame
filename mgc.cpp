#ifdef WIN32
#include <SFML/graphics.hpp>
#else 
#include <SFML/Graphics.hpp>
#endif
#include <UseFull/Math/Vector.hpp>
#include <UseFull/Math/Intersect.hpp>
#include <UseFull/SFMLUp/EventHandler.hpp>
#include <UseFull/Utils/Bytes.hpp>
#include <UseFull/Utils/Macro.hpp>
#include <UseFull/Templates/Array.hpp>

#define EPS 0.001

bool checkIntersection2Rectangles(
	const math::Vector<2> & a0,
	const math::Vector<2> & a1,
	const math::Vector<2> & b0,
	const math::Vector<2> & b1) {
	bool fline13 = ( (a0[0] < b1[0] + EPS) && (a0[0] + EPS > b0[0] || a1[0] + EPS > b1[0]) ) || ( (b0[0] < a1[0] + EPS) && (a0[0] < b0[0] + EPS || a1[0] < b1[0] + EPS) );
	bool fline24 = ( (a0[1] < b1[1] + EPS) && (a0[1] + EPS > b0[1] || a1[1] + EPS > b1[1]) ) || ( (b0[1] < a1[1] + EPS) && (a0[1] < b0[1] + EPS || a1[1] < b1[1] + EPS) );
	return fline24 && fline13;
}
/*
utils::Ok<math::Vector<2>> smoothCollision(const math::Line<2> & delta, const math::Line<2> & line) {
	using namespace math;
	using namespace utils;
	const double virtual_distance = 0.01;
	Vector<2> projection_real = projectionPointOnEquationLine<2>(delta.a, line).ok("smoothCollision::projection1: Fatal Error\n");
	EquationLine<2> el(projection_real, delta.a);
	Line<2> virtual_line = (line + el.vector * virtual_distance).lengthenBy(virtual_distance, LengthenWay::Equally);
	Line<2> real_ex_line = Line<2>(line).lengthenBy(virtual_distance, LengthenWay::Equally);
	bool intersect = 
		checkIntersectLineWithLine2D(delta, virtual_line) ||
		checkIntersectLineWithLine2D(delta, Line<2>(virtual_line.b, real_ex_line.b)) ||
		checkIntersectLineWithLine2D(delta, real_ex_line) ||
		checkIntersectLineWithLine2D(delta, Line<2>(real_ex_line.a, virtual_line.a));
	if (intersect) return projectionPointOnEquationLine<2>(delta.b, virtual_line).ok("smoothCollision::projection2: Fatal Error\n");
	else return {};
}
*/


struct GameLoopContext;
struct GameObject;

struct CollisionEventStruct {
	GameObject * object = nullptr;
	math::Vector<2> collision_point = {0, 0};
	math::Line<2> collision_line = math::Line<2>({0, 0}, {0, 0});
	enum class Direction {
		Up, 
		Left, 
		Down, 
		Right
	} relative_to_object_collision_direction;
	math::Vector<2> getDirVec() {
		switch(relative_to_object_collision_direction) {
			case Direction::Up:    return {0 , 1};
			case Direction::Left:  return {-1, 0};
			case Direction::Down:  return {0 ,-1};
			case Direction::Right: return {1 , 0};
		}
		printf("CollisionEventStruct::getDirVec::Error FATAL\n");
		exit(1);
	}
	static Direction getDirectionFromIndex(size_t i) {
		switch (i) {
			case 0: return Direction::Up;
			case 1: return Direction::Left;
			case 2: return Direction::Down;
			case 3: return Direction::Right;
			default:
				printf("CollisionEventStruct::getDirectionFromIndex::Error: Illegal Index = %llu\n", i);
				exit(1);
		}
	}
	CollisionEventStruct(GameObject * o, const math::Vector<2> & v, const math::Line<2> & l, Direction dir) {
		object = o; 
		collision_point = v;
		collision_line = l;
		relative_to_object_collision_direction = dir;
	}
};

struct GameObject {
	math::Vector<2> position;
	math::Vector<2> speed;
	math::Vector<2> delta;
	math::Vector<2> collision_size;
	uft::Array<GameObject *> ignored = uft::Array<GameObject *>(8);
	
	bool deleteFlag = false;
	
	virtual void updateBase() = 0;
	virtual void updateVectors() = 0;
	virtual void updateMove() = 0;
	virtual void draw() = 0;
	
	GameObject() {
		ignored.addCopy(this);
	}
	
	virtual ~GameObject() {}
	
	void checkCollisions(
		uft::Array<GameObject *> & array,
		utils::CoLambda<void, CollisionEventStruct *> auto onCollision);
};

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
	
	void drawLine(const math::Vector<2> & p1, const math::Vector<2> & p2, sf::Color color = sf::Color::White) {
		drawer::vertexes[0] = sf::Vertex(sf::Vector2f(p1[0], p1[1]), color);
		drawer::vertexes[1] = sf::Vertex(sf::Vector2f(p2[0], p2[1]), color);
		window.draw(drawer::vertexes, 2, sf::Lines);
	}
	
	void drawRectangle(const math::Vector<2> & left_up, const math::Vector<2> & right_down, sf::Color color = sf::Color::White) {
		drawLine(left_up, {left_up[0], right_down[1]}, color);
		drawLine({left_up[0], right_down[1]}, right_down, color);
		drawLine(right_down, {right_down[0], left_up[1]}, color);
		drawLine({right_down[0], left_up[1]}, left_up, color);
	}
	
	void drawCircle(math::Sphere<2> circle, sf::Color color) {
		sf::CircleShape circle_shape;
		circle_shape.setRadius(circle.r);
		circle_shape.setPosition(circle.center[0] - circle.r, circle.center[1] - circle.r);
		circle_shape.setFillColor(color);
		window.draw(circle_shape);
	}
	
	size_t gameLoop(GameLoopContext & context);
} Global;

void GameObject::checkCollisions(
	uft::Array<GameObject *> & array,
	utils::CoLambda<void, CollisionEventStruct *> auto onCollision) {
		
	using namespace utils;
	using namespace math;
	if (delta.norm() < EPS) return;
	
	for (size_t i = 0; i < array._length; i++) {
		GameObject * obj = array[i];
		if (ignored.indexByEquation(obj).isOk) continue;
		
		Codir<2> c(
			obj->position + obj->delta - collision_size,
			obj->position + obj->collision_size + obj->delta
		);
		
		//Global.drawRectangle(c.left_up, c.right_down, sf::Color::Green);
		
		Line<2> delta_line(position, position + delta);
		
		//Global.drawLine(position, position + delta, sf::Color::Red);
		
		//Directions: Up, Left, Down, Right
		Line<2> lines[4] = {
			Line<2>(c.left_up, {c.right_down[0], c.left_up[1]}),
			Line<2>({c.right_down[0], c.left_up[1]}, c.right_down),
			Line<2>({c.left_up[0], c.right_down[1]}, c.right_down),
			Line<2>(c.left_up, {c.left_up[0], c.right_down[1]})
		};
		/*
		for (size_t i = 0; i < 4; i++) {
			Global.drawLine(lines[i].a, lines[i].b, sf::Color::Green);
		}
		*/
		
		Ok<Vector<2>> oks[4] = {
			intersectLineWithLine2D(delta_line, lines[0]),
			intersectLineWithLine2D(delta_line, lines[1]),
			intersectLineWithLine2D(delta_line, lines[2]),
			intersectLineWithLine2D(delta_line, lines[3])
		};
		
		Ok<Vector<2>> eoks[4] = {
			intersectEquationLineWithEquationLine2D(delta_line, lines[0]),
			intersectEquationLineWithEquationLine2D(delta_line, lines[1]),
			intersectEquationLineWithEquationLine2D(delta_line, lines[2]),
			intersectEquationLineWithEquationLine2D(delta_line, lines[3])
		};
		
		/*
		for (size_t j = 0; j < 4; j++) {
			if (eoks[j].isOk) {
				Global.drawCircle(math::Sphere<2>(eoks[j].value, 2), sf::Color(255, 000, 000, 255));
			}
		}
		*/
		
		double min_dist = 0;
		size_t index = 0;
		bool collision_was = false;
		
		size_t j = 0;
		for (; j < 4; j++) {
			if (oks[j].isOk) {
				index = j; 
				min_dist = position.distanceTo(oks[j].value);
				collision_was = true;
				break;
			}
		}
		for (; j < 4; j++) {
			if (oks[j].isOk) {
				double dis = position.distanceTo(oks[j].value);
				if (dis < min_dist) {
					index = j; 
					min_dist = position.distanceTo(oks[j].value);
					//mb need 'break' here
				}
			}
		}
		
		if (collision_was) {
			CollisionEventStruct ces(
				obj, 
				oks[index].value, 
				lines[index], 
				CollisionEventStruct::getDirectionFromIndex(index)
			);
			onCollision(&ces);
		}
	}
}

const char * game_error = nullptr;

using namespace utils;

struct Projectile : public GameObject {
	uft::Array<GameObject *> hurt_targets = uft::Array<GameObject *>(4);
	GameObject * parent = nullptr;
	virtual void collisionAction(GameObject * go) = 0;
	virtual void afterCollisionAction() = 0;
	
	Projectile(GameObject * p) : GameObject() {
		parent = p;
		ignored.addCopy(this);
	}
	
	void intersectCheck() {
		for (size_t i = 0; i < Global.game_objects._length; i++) {
			GameObject * obj = Global.game_objects[i];
			
			if (ignored.indexByEquation(obj).isOk) continue;
			
			if (checkIntersectCodirWithCodir<2>(
				math::Codir<2>(
					position + delta, 
					position + collision_size + delta
				),
				math::Codir<2>(
					obj->position + obj->delta,
					obj->position + obj->collision_size + obj->delta
				)
				)) {
				hurt_targets.addCopy(this);
				collisionAction(obj);
				afterCollisionAction();
				return;
			}
			/*
			if (checkIntersection2Rectangles(
				position + delta, 
				position + collision_size + delta,
				obj->position + obj->delta,
				obj->position + obj->collision_size + obj->delta
				)) {
				
			}
			*/
		}
	}
};

struct Bullet : public Projectile {
	size_t active_time = 60;
	
	Bullet(GameObject * parent) : Projectile(parent) {
		collision_size = {10, 10};
		position = parent->position + (parent->collision_size - collision_size) / 2;
		ignored.addCopy(parent);
		parent->ignored.addCopy(this);
	}
	
	virtual ~Bullet() {
		Ok<size_t> index = parent->ignored.indexByEquation(this);
		if (index.isOk) parent->ignored.remove(index);
		else printf("-_- Again... Bullet bug!\n");
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
	RandomObject(const math::Vector<2> & vec) : GameObject() {
		position = vec;
		collision_size = {30, 30};
	}
	virtual void updateBase() {
		speed *= 0.9;
	}
	virtual void updateVectors() {
		delta = speed = {1, 0};
		checkCollisions(
			Global.game_objects, 
			[this](CollisionEventStruct * ces) {
				math::Vector<2> end_point = 
					projectionPointOnEquationLine<2>(position + delta, ces->collision_line)
					.ok("projectionPointOnEquationLine::fail");
				delta = end_point - position - ces->getDirVec() * EPS;
		});
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
		checkCollisions(
			Global.game_objects, 
			[this](CollisionEventStruct * ces) {
				math::Vector<2> end_point = 
					projectionPointOnEquationLine<2>(position + delta, ces->collision_line)
					.ok("projectionPointOnEquationLine::fail");
				delta = end_point - position - ces->getDirVec() * EPS;
		});
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

        //stepUpdate();

        //View processor

        //Global.SView.step();

        //Clear screen

        window.clear(sf::Color::Black);

		stepUpdate();

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
