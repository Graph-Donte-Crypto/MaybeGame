struct Room;
struct Player;

struct Player {
	Room * room = nullptr;
};

struct ArtefactParts;
struct KeyOrder;

struct PassiveEffect;
struct ActiveEffect;



struct JsonParsePath {
	
};

struct Artefact {
	Array<ArtefactParts> parts;
	Array<KeyOrder> execute_order;
	Array<PassiveEffect> effect_passive;
	Array<ActiveEffect> effect_active;
}

struct CallBackFunction;

struct EntityStatResource {
	size_t max, min, value, delta;
	
	Array<CallBackFunction> onMax;
	Array<CallBackFunction> onMin;
	Array<CallBackFunction> onChange;
	
	void update() {
		changeValue(delta);
	}
	
	void changeValue(size_t value) {
		this->value += value;
		checkMin();
		checkMax();
		onChange.foreach([](CallBackFunction * cbf){
			cbf->execute();
		});
	}
	
	void setValue(size_t value) {
		this->value = value;
		checkMin();
		checkMax();
		onChange.foreach([](CallBackFunction * cbf){
			cbf->execute();
		});
	}
	
	void checkMin() {
		if (value <= min) {
			value = min; 
			onMin.foreach([](CallBackFunction * cbf){
				cbf->execute();
			});
		}
	}
	
	void checkMax() {
		if (value >= max) {
			value = max; 
			onMax.foreach([](CallBackFunction * cbf){
				cbf->execute();
			});
		}
	}
	
}

struct EntityStat {
	EntityStatResource hp;
	EntityStatResource mana;
	size_t move_speed;	
}

enum class PlayerDificulty;
enum class PlayerRace;
enum class PlayerClass;

struct Player {
	Array<Artefact> artefact;
	
	Array<PassiveEffect> effect_passive;
	Array<ActiveEffect> effect_active;
	
	
	
	PlayerDificulty p_dificulty;
	/*
	1  - 3  buttons, attack ~~ attack
	4  - 6  buttons, attack ~~ aim, attack
	7  - 10 buttons, attack ~~ readying, aim, attack
	11 - 15 buttons, attack ~~ readying, aim, attack, reload
	*/
	PlayerRace p_race;
	PlayerClass p_class;
	
	
	EntityStat stats;
}


struct Room {
	size_t width, height;
	size_t * cells;
	
	Room(size_t w, size_t h) : width(w), height(h) {		
		cells = new size_t[width * height];
	}
	
	~Room() {
		delete[] cells;
	}
	
	size_t * operator [] (size_t i) {
		return cells + i * height;
	}
};