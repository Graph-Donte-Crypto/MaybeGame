#ifdef WIN32
#include <SFML/graphics.hpp>
#else 
#include <SFML/Graphics.hpp>
#endif
#include <UseFull/Math/Vector.hpp>
#include <UseFull/Math/Intersect.hpp>
#include <UseFull/SFMLUp/Event.hpp>
#include <UseFull/Utils/Bytes.hpp>
#include <UseFull/Utils/Macro.hpp>
#include <UseFull/Templates/Array.hpp>

#include <vector>

#define EPS 0.001

struct ObjectsStruct{
	std::vector<std::pair<void *, const char *>> pairs;
	
	void add(void * ptr, const char * name) {
		pairs.push_back({ptr, name});
	}
	
	const char * getByPtr(void * ptr) {
		for (size_t i = 0; i < pairs.size(); i++)
			if (pairs[i].first == ptr) return pairs[i].second;
		return nullptr;
	}
	
	void * getByName(const char * name) {
		for (size_t i = 0; i < pairs.size(); i++)
			if (strcmp(pairs[i].second, name) == 0) return pairs[i].first;
		return nullptr;
	}
} Object;

struct GameLoopContext;
struct GameObject;

struct CollisionEventStruct2 {
	GameObject * object = nullptr;
	math::Vector<2> new_delta = {0, 0};
	CollisionEventStruct2(GameObject * go, const math::Vector<2> & ndelta) {
		object = go;
		new_delta = ndelta;
	}
};

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
	
	math::Vector<2> position, temp_position;
	math::Vector<2> speed, temp_speed;
	math::Vector<2> delta, temp_delta;
	
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
	void checkCollisionsGrt(
		uft::Array<GameObject *> & array,
		utils::CoLambda<void, CollisionEventStruct *> auto onCollision);
	void checkCollisionsStupid(
		uft::Array<GameObject *> & array,
		utils::CoLambda<void, CollisionEventStruct *> auto onCollision);
	void checkCollisionsSegment(
		uft::Array<GameObject *> & array,
		utils::CoLambda<void, CollisionEventStruct2 *> auto onCollision);
		
	void drawCollisionBox();
	
	void standartSmoothCollision(uft::Array<GameObject *> & array) {
		checkCollisions(
			array, 
			[this](CollisionEventStruct * ces) {
				math::Vector<2> end_point = 
					projectionPointOnEquationLine<2>(position + delta, ces->collision_line)
					.ok("projectionPointOnEquationLine::fail");
				temp_delta = end_point - position - ces->getDirVec() * EPS;
				printf("me %llu direVec (%lf, %lf)\n", this, ces->getDirVec()[0], ces->getDirVec()[1]);
				end_point.printf("%lf ");
				position.printf("%lf ");
				delta.printf("%lf ");
				temp_delta.printf("%lf ");
				(position + delta).printf("%lf ");
				(position + temp_delta).printf("%lf ");
			}
		);
	}
	void standartSegmentCollision(uft::Array<GameObject *> & array) {
		checkCollisionsSegment(
			array,
			[this](CollisionEventStruct2 * ces2) {
				delta = ces2->new_delta;
			}
		);
	}
};

namespace drawer {
	sf::Vertex vertexes[2];
}

struct GlobalStruct {
	
	uft::Array<GameObject *> game_objects;
	uft::Array<GameObject *> game_objects_next_turn;
	
	const char * window_name = "MaybeGame 0.1";
	sf::RenderWindow window;
	
	sf::View view;
	size_t limit_fps = 2;
	
	sf::Clock clock;
	double current_time;
	double current_fps;
	
	void setFrameRate(size_t fps) {
		limit_fps = fps;
		window.setFramerateLimit(limit_fps);
	}
	
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
		for (size_t i = 0; i < game_objects.length; i++) {
			if (game_objects[i]->deleteFlag) {
				delete game_objects[i];
				game_objects.remove(i);
				i -= 1;
			}
		}

		for (size_t i = 0; i < game_objects_next_turn.length; i++)
			game_objects.addCopy(game_objects_next_turn[i]);
		game_objects_next_turn.removeAll();

		
		game_objects.foreach([](GameObject ** go){(*go)->temp_speed = (*go)->speed;});
		game_objects.foreach([](GameObject ** go){(*go)->updateBase();});
		game_objects.foreach([](GameObject ** go){(*go)->speed = (*go)->temp_speed;});
		//After updateBase we know speed

		game_objects.foreach([](GameObject ** go){(*go)->delta = (*go)->speed; (*go)->temp_delta = (*go)->delta;});
		game_objects.foreach([](GameObject ** go){(*go)->updateVectors();});
		game_objects.foreach([](GameObject ** go){(*go)->delta = (*go)->temp_delta;});
		//After updateVectors we know delta
		
		game_objects.foreach([](GameObject ** go){(*go)->temp_position = (*go)->position;});
		game_objects.foreach([](GameObject ** go){(*go)->updateMove();});
		game_objects.foreach([](GameObject ** go){(*go)->position = (*go)->temp_position;});
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

void GameObject::drawCollisionBox() {
	Global.drawRectangle(position, position + collision_size);
}

void GameObject::checkCollisionsSegment(
	uft::Array<GameObject *> & array,
	utils::CoLambda<void, CollisionEventStruct2 *> auto onCollision) {
		
}

void GameObject::checkCollisionsStupid(
	uft::Array<GameObject *> & array,
	utils::CoLambda<void, CollisionEventStruct *> auto onCollision) {
	
	using namespace utils;
	using namespace math;	
	
	for (size_t i = 0; i < array.length; i++) {
		GameObject * obj = array[i];
		if (ignored.indexByEquation(obj).isOk) continue;
		
		/*
		Codir<2> c(
			obj->position + obj->delta - collision_size,
			obj->position + obj->collision_size + obj->delta
		);
		
		if (checkPointInCodir(position, c)) c -= obj->delta;
		*/
		Codir<2> c = Codir<2>(
			obj->position - collision_size,
			obj->position + obj->collision_size
		);
		
		if (!checkPointInCodir(position + delta, c + obj->delta)) c += obj->delta;
		
		
		Line<2> delta_line(position, position + delta);
		
		//Directions: Up, Left, Down, Right
		Line<2> lines[4] = {
			Line<2>(c.left_up, {c.right_down[0], c.left_up[1]}),
			Line<2>({c.right_down[0], c.left_up[1]}, c.right_down),
			Line<2>({c.left_up[0], c.right_down[1]}, c.right_down),
			Line<2>(c.left_up, {c.left_up[0], c.right_down[1]})
		};

		Ok<Vector<2>> oks[4] = {
			intersectLineWithLine2D(delta_line, lines[0]),
			intersectLineWithLine2D(delta_line, lines[1]),
			intersectLineWithLine2D(delta_line, lines[2]),
			intersectLineWithLine2D(delta_line, lines[3])
		};
		
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
					min_dist = dis;
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

void GameObject::checkCollisionsGrt(
	uft::Array<GameObject *> & array,
	utils::CoLambda<void, CollisionEventStruct *> auto onCollision) {
		
	using namespace utils;
	using namespace math;

	for (size_t i = 0; i < array.length; i++) {
		GameObject * obj = array[i];
		if (ignored.indexByEquation(obj).isOk) continue;
		
		Vector<2> grt_delta = delta - obj->delta;
		
		Codir<2> c = Codir<2>(
			obj->position - collision_size,
			obj->position + obj->collision_size
		);
		
		Line<2> delta_line(position, position + grt_delta);
		
		//Directions: Up, Left, Down, Right
		Line<2> lines[4] = {
			Line<2>(c.left_up, {c.right_down[0], c.left_up[1]}),
			Line<2>({c.right_down[0], c.left_up[1]}, c.right_down),
			Line<2>({c.left_up[0], c.right_down[1]}, c.right_down),
			Line<2>(c.left_up, {c.left_up[0], c.right_down[1]})
		};

		Ok<Vector<2>> oks[4] = {
			intersectLineWithLine2D(delta_line, lines[0]),
			intersectLineWithLine2D(delta_line, lines[1]),
			intersectLineWithLine2D(delta_line, lines[2]),
			intersectLineWithLine2D(delta_line, lines[3])
		};
		
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
					min_dist = dis;
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

void GameObject::checkCollisions(
	uft::Array<GameObject *> & array,
	utils::CoLambda<void, CollisionEventStruct *> auto onCollision) {
		
	using namespace utils;
	using namespace math;
	if (delta.norm() < EPS) return;
	
	/*
	
	Ошибка в том, что если объекты движуться друг на друга, то
		точка position оказывается внутри объекта для колижена
		И так как вектор направлен внутрь объекта, пересечение не будет зафиксировано
		
	*/
	
	for (size_t i = 0; i < array.length; i++) {
		
		GameObject * obj = array[i];
		
		//bool check = (Object.getByName("RO") == this) && (Object.getByName("Player") == obj) ;
		
		if (ignored.indexByEquation(obj).isOk) continue;
		
		Codir<2> c = Codir<2>(
			obj->position - collision_size,
			obj->position + obj->collision_size
		) + obj->delta;
		
		printf("\n%s\n", Object.getByPtr(this));
		
		printf("delta: ");
		delta.printf("%lf ");
		printf("collision size: ");
		collision_size.printf("%lf ");
		printf("position: ");
		position.printf("%lf ");
		printf("c:\n");
		c.printf("%lf ");
		
		if (checkPointInCodir(position, c + delta)) {
			
			printf("Collision was\n");
			
			std::pair<Ok<Vector<2>>, Ok<Vector<2>>> l = intersectLineWithCodir(
				Line<2>(position, position + delta),
				c
			);
			
			//c -= obj->delta;
			
			if (l.first.isOk) {
				/*
				c =  Codir<2>(
					obj->position - collision_size,
					obj->position + obj->collision_size
				) + l.first.value;
				*/
				c -= position + obj->delta - l.first.value;
			}
			else {
				/*
				printf("%i\n", checkPointInCodir(
					position, Codir<2>(
						obj->position - collision_size, 
						obj->position + obj->collision_size)
					), EPS
				);
				printf("I am %llu\n", this);
				(c - obj->delta).printf();
				c.printf();
				position.printf();
				obj->delta.printf();
				*/
				c -= obj->delta;
				//exit(1);
			}
		}
		
		
		
		
		
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
		
		for (size_t i = 0; i < 4; i++) {
			//Global.drawLine(lines[i].a, lines[i].b, sf::Color::Green);
		}
		
		
		Ok<Vector<2>> oks[4] = {
			intersectLineWithLine2D(delta_line, lines[0]),
			intersectLineWithLine2D(delta_line, lines[1]),
			intersectLineWithLine2D(delta_line, lines[2]),
			intersectLineWithLine2D(delta_line, lines[3])
		};
		
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
					min_dist = dis;
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
		for (size_t i = 0; i < Global.game_objects.length; i++) {
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
		//TODO: if parent deleted, program will crush
		//
		
		Ok<size_t> index = parent->ignored.indexByEquation(this);
		if (index.isOk) parent->ignored.remove(index);
		else printf("-_- Again... Bullet bug!\n");
	}
	
	virtual void updateBase() {
		if (active_time == 0) deleteFlag = true;
		else active_time -= 1;
	};
	virtual void updateVectors() {
		
	}
	virtual void updateMove() {
		intersectCheck();
		temp_position += delta;
	}
	virtual void draw() {
		drawCollisionBox();
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
		temp_speed += {1.0/9, 0};
		temp_speed *= 0.9;
	}
	virtual void updateVectors() {
		standartSmoothCollision(Global.game_objects);
	}
	virtual void updateMove() {
		temp_position += delta;
	}
	virtual void draw() {
		drawCollisionBox();
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
		
		if (input->movement[0] == 1) temp_speed[1] -= max_speed / 8;//W
		if (input->movement[1] == 1) temp_speed[0] -= max_speed / 8;//A
		if (input->movement[2] == 1) temp_speed[1] += max_speed / 8;//S
		if (input->movement[3] == 1) temp_speed[0] += max_speed / 8;//D
		
		temp_speed *= 0.9;
		temp_speed.truncateTo(max_speed);	
		
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
				Global.game_objects_next_turn.addCopy(bullet);
			}
		}
	}
	
	void updateVectors() {;
		standartSmoothCollision(Global.game_objects);
	}
	
	void updateMove() {
		temp_position += delta;
	}
	
	void draw() {
		drawCollisionBox();
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

		if (Event.KeyPressing[sf::Keyboard::Key::Q]) Global.setFrameRate(1);
		if (Event.KeyPressing[sf::Keyboard::Key::E]) Global.setFrameRate(60);

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
	RandomObject ro({250, 100});
	
	Global.game_objects.addCopy(&ro);
	Global.game_objects.addCopy(context.local_player);
	
	Object.add(&ro, "RO");
	Object.add(context.local_player, "Player");
	
	printf("RO is %llu\n", Global.game_objects[0]);
	printf("Player is %llu\n", Global.game_objects[1]);
	
	
	size_t game_status = Global.gameLoop(context);
	
	switch (game_status) {
		case 0: 
			return 0;
		default:
			printf("ErrorCode: %llu\nMessage: %s\n", (long long unsigned)game_status, game_error == nullptr ? "None" : game_error);
			break;
	}
	
}
