
int main() {
	
	
	
}

/*
bool connection_opened = false;
bool game_started = false;
bool clients_ready = false;
bool game_closed = false;
int tcp_port_host = 0;
int tcp_port_connect = 0;
int udp_port_host = 0;
int udp_port_clients = 0;

struct Object {
	Vector<2> position;
	Vector<2> vector;
	Room * room;
	
	void collision() {
		uft::Array<Object *> mas;
		room->objects.foreach([&mas](Object * object){
			if (object == this) return;
			mas.add(object);
		});
		uft::Array<Object *> buf;
		
		while (mas.length() != 0) {
			mas.foreach([&buf](Object * object){
				if (object->intersect(this)) buf.add(object);
			});
			Object * near = findNearest(this, buf);
			actCollision(near);
			buf.remove(near);
			arr.removeAll();
			arr.addAll(buf);
			buf.removeAll();
		}
	}
};

struct Room {
	
};

struct Player {
	
};


int getFreePort() {
	int port;
	while (true) {
		port = rand() % 256 * 128;
		if (Socket::FreePort(port)) return port;
	}
}

void connectionHandler() {
	tcp_port_host    = getFreePort();
	tcp_port_connect = getFreePort();
	udp_port_host    = getFreePort();
	udp_port_clients = getFreePort();
	connection_opened = true;
	while (true) {
		WaitClient();
		Sleep(100);
		if (game_started) break;
	}
	clients_ready = true;
	while (true) {
		Sleep(100);
		if (game_closed) {
			players.foreach([](Player * player){
				SendCloseData(player->tcp_socket);
			});
			break;
		}
		else {
			players.foreach([](Player * player){
				SendSyncData(player->tcp_socket);
			});
		}
	}
	
}

struct GameStartData {
	size_t fps;
}

AllMove() {
	uft::Array<math::Vector<2>> new_positions;
	active_room.foreach([&new_array] (Room * room) {
		room->movable.foreach([&new_array](Object * object){
			object->collision();
			new_positions.add(object->position + object->vector);
			pointers.add(object);
		});
		
		for (size_t i = 0; i < room->movable.length(); i++) {
			room->movable[i].position = new_positions[i];
		}	
	});
}

int main(int argc, char ** argv) {
	StartThread(connectionHandler, nullptr);
	
	while (!connection_opened) Sleep(10);
	
	WaitHost();
	GameStartData gsd;
	while (true) {
		//if receive GameStartData -> start_game;
		Sleep(50);
	}
	
	while (!clients_ready) Sleep(10);
	
	while (true) {
		
		
		AllMove();
		DrawDebug();
		
		Sleep(1000 / fps);
	}
	
	
	return 0;
}
*/