#include <queue>
#include <stdio.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <list>
#include <iterator>
#include <cassert>
#include <functional>

//#include <windows.h>

#define DO_PRAGMA(x) _Pragma (#x)
#define TODO(x) DO_PRAGMA(message ("TODO - " #x))

#define prefix_unused(variable) ((void)variable)

#include <stdlib.h>
#include <cctype>

#include "libs/Queue2.h"
#include "libs/Vector.cpp"
#include "libs/UFSF33.cpp"

sf::View view;
sf::RenderWindow window(sf::VideoMode(800, 600), "My window");
int frame_rate = 60;

#include "libs/Global6.h"
#include "libs/Event6.h"

//-lWs2_32 -liphlpapi

class Basic;

#include "libs/RandomAccessStorage10.cpp"

#define EPS 0.001

#define DEBUG if (false)

int id_global = 0;

static
class SpecialView {
public:
	XY * new_center, curent_center;
	XY * control_left_up = nullptr, * control_right_down = nullptr;
	int coef = 3;
	SpecialView() {
		curent_center(0, 0);
		view.reset(sf::FloatRect(0.0f, 0.0f, window.getSize().x, window.getSize().y));
		window.setView(view);
	};
	~SpecialView(){};
	void step() {
		if (new_center != nullptr) {
			curent_center += (*new_center - curent_center) / coef;
			view.setCenter(curent_center.x, curent_center.y);
			window.setView(view);
		}
		else if (control_left_up && control_right_down) {
			XY temp_center = (*control_left_up + *control_right_down)/2;

			curent_center += (temp_center - curent_center) / coef;
			view.setCenter(curent_center.x, curent_center.y);
			window.setView(view);
		}
	}
}
SView;

static
class TexturesClass {
public:
    Ras<sf::Texture> set;
    Ras<sf::String> paths;
	Ras<sf::Image> images;

    TexturesClass() {
        paths.add(new sf::String("source/void.png"));
        paths.add(new sf::String("source/robot-open.png"));
        paths.add(new sf::String("source/robot-closed.png"));
        sf::Image * image;
        sf::Texture * texture;
        for (int i = 0; i < paths.pos; i++) {
            std::cout << paths[i]->toAnsiString() << '\n';
            texture = new sf::Texture();
            image = new sf::Image();
			image->loadFromFile(*(paths[i]));
            texture->loadFromImage(*image);
            set.add(texture);
            images.add(image);
        }
    }

}
Textures;

enum class ClassType {
    Null,
    Basic,
    Entity,
    Block,
    BlockTeleporter,
    Player
};
enum class ProcessingState {
    none,
    start,
    end
};
class Basic {
public:
    int id = 0;
    int default_image = 0;
	ClassType type = ClassType::Basic;
    ProcessingState processing = ProcessingState::none;

	XY  center     = XY(0, 0),
        left_up    = XY(0, 0),
        right_down = XY(0, 0);

	bool solid      = true;

	//Физические переменные

	Vector v    = XY(0, 0);

	//Конец физических переменных

	Vector distance = XY(0, 0);

	sf::Image   image;
	sf::Texture texture;
	sf::Sprite  sprite;

	unsigned short frame_int = 0;
	unsigned short image_int = 0;

	XY size;

	RasRec<Basic> * ras_record;

	Basic() {
		id = id_global++;
	};
	Basic(const XY & xy1, const XY & xy2) {
	    id = id_global++;;

		left_up    = xy1 + XY(EPS, EPS);
		right_down = xy2 - XY(EPS, EPS);
		center     = (left_up + right_down)/2;
		size       = xy2 - xy1;
	}

	virtual void addDefault() {

	}

	virtual void moveDelta(const XY & delta) {
        left_up    += delta;
		right_down += delta;
		center     += delta;
	}
	void setImageString(int i) {
		image_int = i;
		texture = sf::Texture(*(Textures.set[i]));
		sprite.setTexture(texture);
		sprite.setTextureRect(sf::IntRect(0, 0, size.x, size.y));
	}

	virtual void setDefaultImage() {
        setImageString(default_image);
	}

	virtual void textAllInfo() {
	    Global.Text << "\nid = " << id;
	    Global.Text << "\npointer = " << this;
		Global.Text << "\ntype = " << (int)type;
		Global.Text << "\ncenter = " << center.x << " " << center.y;
		Global.Text << "\nleft_up = " << left_up.x << " " << left_up.y;
		Global.Text << "\nright_down = " << right_down.x << " " << right_down.y;
		Global.Text << "\nsolid = " << solid;
		Global.Text << "\nstanding = ";

		Global.Text << "\nv = " << v.xy.x << " " << v.xy.y;
	}

	virtual void drawAllInfo() {
        textAllInfo();
        Global.drawText(XY(right_down, left_up));
	}

	void printAllInfo() {
		printf("type %i\n", (int)type);
		printf("center %lf %lf\n", center.x, center.y);
		printf("left_up %lf %lf\n", left_up.x, left_up.y);
		printf("right_down %lf %lf\n", right_down.x, right_down.y);
		printf("solid %i\n", solid);
		printf("v %lf %lf\n", v.xy.x, v.xy.y);
		printf("distance %lf %lf\n", distance.xy.x, distance.xy.y);
		printf("size %lf %lf\n", size.x, size.y);
	}

	void addRasBasic(Ras<Basic> & set) {ras_record = set.add(this);}
	void removeRasBasic() {if (ras_record != nullptr) ras_record = ras_record->remove();}

	virtual int action() { return 0; }
	virtual int draw() {
		XY left_down(left_up.x, right_down.y);
		XY right_up(right_down.x, left_up.y);
		Global.drawLine(left_up,right_up);
		Global.drawLine(left_up,left_down);
		Global.drawLine(left_down,right_down);
		Global.drawLine(right_down,right_up);

		window.draw(sprite);
		//Global.text_stream << speed.xy.x << "|" << speed.xy.y;
		//Global.drawText(center);
		return 1;
	}

	virtual int remove() {
		removeRasBasic();
		delete this;
		return 0;
	}

	int bisection_iteration_normal = 64;
	int bisection_iteration_special = 16;

	bool ScI2B(const XY & xy, const Basic & o) {
		return Global.S.cI2R(left_up + xy, right_down + xy, o.left_up, o.right_down);
	}

	virtual ~Basic() {
	}
};

static
class ActionStack {
public:

	Ras<Basic> action_set;
	Ras<Basic> action_win_set;
	int action_count, draw_count;
	int action_i = 0, action_win_i = 0;
	XY mouse = XY(0, 0);

	Basic * focus           = nullptr,
	      * focus_next_turn = nullptr;

	ActionStack() {
		sf::Vector2f mw = window.mapPixelToCoords(sf::Mouse::getPosition( window ) );
		mouse(mw.x, mw.y);
		action_set.current     = &action_i;
		action_win_set.current = &action_win_i;
	}
	~ActionStack() {
		for (int i = 0; i < action_set.pos; i++) {
			delete action_set[i];
		}
		for (int i = 0; i < action_win_set.pos; i++) {
			delete action_win_set[i];
		}
	}
	void stepAction() {

		sf::Vector2f mw = window.mapPixelToCoords(sf::Mouse::getPosition( window ) );
		mouse(mw.x, mw.y);


		for (action_i = 0; action_i < action_set.pos; action_i++) {
			action_set[action_i]->processing = ProcessingState::none;
			//printf("%i -> %I64u\n", action_i, (unsigned long long)action_set[action_i]);
		}
		//printf("Action Start\n");
		action_count = 0;
		for (action_i = 0; action_i < action_set.pos; action_i++) {
			//printf("AStack now\n");
			//action_set.print();

			//printf("Process %i -> ", action_i);
			if (action_set[action_i]->processing == ProcessingState::none) {
				//printf("Now\n");
				action_count += action_set[action_i]->action();
			}
			//else {printf("Was\n");}
		}
		//printf("Action End\n");
		action_set._fixArray();
		//printf("Action Fix\n");

		//sendToServers();
	}
	void stepActionWin() {

	    sf::Vector2f mw = window.mapPixelToCoords(sf::Mouse::getPosition( window ) );
		mouse(mw.x, mw.y);

	    focus = focus_next_turn;
		focus_next_turn = nullptr;

        for (action_win_i = 0; action_win_i < action_win_set.pos; action_win_i++) {
			action_win_set[action_win_i]->processing = ProcessingState::none;
		}
		for (action_win_i = 0; action_win_i < action_win_set.pos; action_win_i++) {
			if (action_win_set[action_win_i]->processing == ProcessingState::none) {
				action_win_set[action_win_i]->action();
			}
		}
		action_win_set._fixArray();
	}

	void stepDraw() {
		//printf("Draw Start\n");
		draw_count = 0;
		for (int i = 0; i < action_set.pos; i++) {
			draw_count += action_set[i]->draw();
		}
		//printf("Draw End\n");
		Global.Text << "Count(A|D): " << action_count << " | " << draw_count;
		Global.drawText(XY(0,20));
		Global.drawText("ACTIVE: Ubuntu::Mono::Regular", XY(0,40));
		Global.Text << "focus: " << focus << " | " << "focus on next turn: " << focus_next_turn;
		Global.drawText(XY(0,60));
		
		Global.drawLine(mouse - XY(5, 5), mouse + XY(5, 5));
		Global.drawLine(mouse - XY(-5, 5), mouse + XY(-5, 5));



		//Global.Text << "Mouse Position(X|Y): " << mouse.x << " | " << mouse.y;
		//Global.drawText(XY(0,60));

	}

	void stepDrawWin() {
		for (int i = 0; i < action_win_set.pos; i++) {
			action_win_set[i]->draw();
		}
	}
}
AStack;

//Engine part

class BaseGui : public Basic {
public:

    XY left_down, right_up;

    void moveDelta(const XY & delta) {
	    Basic::moveDelta(delta);
		left_down(left_up   , right_down);
		right_up (right_down, left_up);
	}

	void drawSelf() {
        Global.drawLine(left_up,left_down);
		Global.drawLine(left_down,right_down);
		Global.drawLine(right_down,right_up);
		Global.drawLine(right_up,left_up);
		//drawAllInfo();
	}
	int draw() {
		drawSelf();
		return 1;
	}

    bool prev_focused = false;
    struct State {
        bool pressed = false;
        bool focused = false;
    };

    bool active = true;

    State state_before;
    State state_after;

    bool & pressed = state_before.pressed;
    bool & focused = state_before.focused;

    void textAllInfo() {
        Basic::textAllInfo();
		Global.Text << "\nleft_down = " << left_down.x << " " << left_down.y;
		Global.Text << "\nright_up = " << right_up.x << " " << right_up.y;
		Global.Text << "\nstate_before_exec = " << state_before.pressed << " " << state_before.focused;
		Global.Text << "\nstate_after_exec = " << state_after.pressed << " " << state_after.focused;
		Global.Text << "\nprev_focused = " << prev_focused;
    }

    void addDefault() {
		addRasBasic(AStack.action_win_set);
	}

	BaseGui(const XY & begin, const XY & end) : Basic(begin, end) {
        solid = false;
        left_down(left_up.x, right_down.y);
		right_up(right_down.x, left_up.y);
		type = ClassType::Basic;TODO(remake!)
	}

	virtual void actionNotFocused() {}
	virtual void actionNotFocusing() {}
	virtual void actionFocused() {}
	virtual void actionFocusing() {}
	virtual void actionPressed() {}
	virtual void actionPressing() {}
	virtual void actionRealized() {}

	//#define EXEC(some) {state_after = state_before; some; state_before = state_after; printf("%s\n", #some);}
	#define EXEC(some) {state_after = state_before; some; state_before = state_after;}

	void focusTracker(const XY & focus_left_up, const XY & focus_right_down) {
		if (AStack.focus == nullptr || AStack.focus == this) {

			focused = Global.checkMouseInBox(AStack.mouse, focus_left_up, focus_right_down);

			if (!focused) {
                EXEC(
                    state_after.pressed = false;
                    if (prev_focused) actionNotFocused();
                    actionNotFocusing();
                )

            }
			else {
				AStack.focus = this;
				AStack.focus_next_turn = this;
				if (!pressed) if ((pressed = Event.e[ev::eMouseButtonPressedLeft]) == true) EXEC(actionPressed();)
				if (!pressed) {
                    if (!prev_focused) EXEC(actionFocused();)
                    EXEC(actionFocusing();)
                }
				else {
					if (Event.e[ev::eMouseButtonReleasedLeft]) {
						EXEC(state_after.pressed = false; actionRealized();)
					}
					else EXEC(actionPressing();)
				}
			}

			prev_focused = focused;

		}
		
	}

	#undef EXEC
};

#include "libs/Frame.cpp"

class Button : public BaseGui {
public:
	Frame * frame;
	sf::Color color_focus_not;//(255,255,255,127)
	sf::Color color_focus;//(255,255,255,255)
	sf::Color color_press;//(063,063,063,255)

	sf::Text text;
    XY text_position = XY(0,0);
	Button(const XY & begin, const XY & end) : BaseGui(begin, end) {
		frame = new Frame(8, (left_up + right_down) / 2);
		color_focus_not = sf::Color(255,255,255,127);
		color_focus 	= sf::Color(255,255,255,255);
		color_press 	= sf::Color(063,063,063,255);
		frame->line[0].color = color_focus_not;
		frame->line[1].color = color_focus_not;

		text.setFont(ubuntu_mono_r);
		text.setCharacterSize(14);
		text.setString("");
		text_position = center;
		setColor(color_focus_not);
	}
	~Button() {
        delete frame;
	}
	int draw() {
		drawSelf();
		frame->draw();
		text.setPosition(text_position.x, text_position.y);
		window.draw(text);
		return 1;
	}
	void setColor(const sf::Color & color) {
	    frame->line[0].color = color;
		frame->line[1].color = color;
		text.setFillColor(color);
	}
	void actionNotFocused() {
		setColor(color_focus_not);
	}
	void actionFocused() {
	    setColor(color_focus);
	}
	void actionPressed() {
	    setColor(color_press);
	}
	void actionRealized() {
	    setColor(color_focus);
	}
	int action() {
		if (active) focusTracker(left_up, right_down);
		return 1;
	}
	void moveDelta(const XY & delta) {
	    BaseGui::moveDelta(delta);
	    frame->center += delta;
	    text_position += delta;
	}
};

class ButtonVoidExec : public Button {
public:
    std::function<void (void)> lambda = nullptr;

    ButtonVoidExec(const XY & begin, const XY & end) : Button(begin, end) {
	}

	void actionRealized() {
	    Button::actionRealized();
        if (lambda != nullptr) lambda();
	}
};

class Window : public BaseGui {
public:
    //ctrlm -> control_menu
	XY ctrlm_right_down;
    double ctrlm_paddind = 20;
	XY moveble_center = XY(0,0);

	Ras<BaseGui> elements;



	void textAllInfo() {
        BaseGui::textAllInfo();
		Global.Text << "\nctrlm_right_down = " << ctrlm_right_down.x << " " << ctrlm_right_down.y;
		Global.Text << "\nctrlm_paddind = " << ctrlm_paddind;
		Global.Text << "\nmoveble_center = " << moveble_center.x << " " << moveble_center.y;
    }

	Window(const XY & begin, const XY & end) : BaseGui(begin, end) {
		ctrlm_right_down = right_up + XY(0, ctrlm_paddind);
	}
	~Window() {
        elements.flushHard();
	}
	int draw() {
        drawSelf();

		Global.drawLine(
            XY(left_up, ctrlm_right_down),
            ctrlm_right_down
        );

        //drawAllInfo();

	    for (int i = 0; i < elements.pos; i++) {
	        elements[i]->draw();
	    }

		return 1;
	}
	void actionNotFocused() {
		if (state_before.pressed) {
			actionPressing();
			AStack.focus = this;
			AStack.focus_next_turn = this;
			state_after.pressed = true;
			state_after.focused = true;
		}
	}
	void actionFocused() {

	}
	void actionPressed() {
		moveble_center = center - AStack.mouse;
	}
	void moveDelta(const XY & delta) {
        BaseGui::moveDelta(delta);
        ctrlm_right_down = right_up + XY(0, ctrlm_paddind);
        for (int i = 0; i < elements.pos; i++) {
            elements[i]->moveDelta(delta);
        }
	}
	void actionPressing() {
	    moveDelta(moveble_center - center + AStack.mouse);
	}
	int action() {
	    if (active) {
            Basic * focus_buf = nullptr;
            if (pressed == false) {
                focus_buf = AStack.focus;
                AStack.focus = nullptr;
            }
            for (int i = 0; i < elements.pos; i++) {
                elements[i]->action();
            }
            if (AStack.focus == nullptr) AStack.focus = focus_buf;
            focusTracker(left_up, ctrlm_right_down);
	    }
	    return 1;
	}
};

enum class MainLoopType {start, resume};

bool mainLoop(MainLoopType loopType) {
    sf::Clock clock;
    sf::Text text("", ubuntu_mono_r, 14);
    text.setPosition(0, 30);
    std::ostringstream fps_stream;
    double currentTime;
    double fps;

    while (window.isOpen()) {
        //EventKeeper processor

        Event.flush();
        Event.load();

        //Action processor

        AStack.stepAction();

        //View processor

        SView.step();

        //Clear screen

        window.clear(sf::Color::Black);

        //Draw Processor

        AStack.stepDraw();
        //SView.new_center->print();
        for (int i = 0; i < 100; i++) Global.drawText(i*50, XY(i*50,500));

        currentTime = clock.restart().asSeconds();
        fps = (double)1.0 / currentTime;


        fps_stream << fps;
        text.setString("FPS: " + fps_stream.str());

        window.draw(text);
        fps_stream.str("");
        fps_stream.clear();

        //Display part

        window.display();
    }

	return 0;
}


enum class UbermenuType{main, ingame};
int ubermenu(UbermenuType type, bool active, bool pause) {
    ButtonVoidExec * b1player = nullptr;
    XY current_view = XY(400,300);
    if (type == UbermenuType::main) {
        Window * win = new Window(XY(0,0), XY(200,400));
        win->moveDelta(XY(300, 100));
        win->addDefault();

        b1player = new ButtonVoidExec(XY(0, 0), XY(150, 50));
        b1player->lambda = [&active, &pause, &current_view] () mutable {
            if (active) pause = mainLoop(MainLoopType::resume);
            else pause = mainLoop(MainLoopType::start);
            active = true;
            for (int i = 0; i < AStack.action_win_set.pos; i++) {
                AStack.action_win_set[i]->moveDelta(-current_view);
                AStack.action_win_set[i]->moveDelta(SView.curent_center);
            }
            current_view = SView.curent_center;
        };
        b1player->text.setString("Single Player Game");
        b1player->text_position -= Global.getTextSize(b1player->text) / 2;
        b1player->moveDelta(win->left_up);
        b1player->moveDelta(XY(25, 50));
        win->elements.add(b1player);


        ButtonVoidExec * but = new ButtonVoidExec(XY(0, 0), XY(150, 50));
        but->lambda = []() {
            //Sleep(1000);
        };
        but->text.setString("Multi Player Game");
        but->text_position -= Global.getTextSize(but->text) / 2;
        but->moveDelta(win->left_up);
        but->moveDelta(XY(25, 150));
        win->elements.add(but);

        but = new ButtonVoidExec(XY(0, 0), XY(150, 50));
        but->lambda = [win]() {
            win->active = false;

            Window * win1 = new Window(XY(0,0), XY(150, 120));
            XY temp = win->elements[2]->right_down - win->elements[2]->left_up;
            win1->moveDelta(win->elements[2]->center - temp/2);
            win1->addDefault();

            ButtonVoidExec * but1 = new ButtonVoidExec(XY(0, 0), XY(100, 20));
            but1->lambda = []() {
                exit(1);
            };
            but1->text.setString("Yes");
            but1->text_position -= Global.getTextSize(but1->text) / 2;
            but1->moveDelta(win1->left_up);
            but1->moveDelta(XY(20, 40));
            win1->elements.add(but1);


            but1 = new ButtonVoidExec(XY(0, 0), XY(100, 20));
            but1->lambda = [win, win1]() {
                win->active = true;
                win1->remove();
            };
            but1->text.setString("Back");
            but1->text_position -= Global.getTextSize(but1->text) / 2;
            but1->moveDelta(win1->left_up);
            but1->moveDelta(XY(20, 80));
            win1->elements.add(but1);


        };
        but->text.setString("Exit");
        but->text_position -= Global.getTextSize(but->text) / 2;
        but->moveDelta(win->left_up);
        but->moveDelta(XY(25, 250));
        win->elements.add(but);

    }

    while (window.isOpen()) {
        //EventKeeper processor
        Event.flush();
        Event.load();
        if (Event.KeyPressed[sf::Keyboard::Escape] && active) {
            b1player->actionRealized();
        }

        //Action processor
        if (!pause) AStack.stepAction();
        AStack.stepActionWin();

        //View processor
        SView.step();

        //Clear screen
        window.clear(sf::Color::Black);

        //Draw Processor
        AStack.stepDraw();
        AStack.stepDrawWin();

        //Display part
        window.display();
    }
    return 0;
}

int main(int argc, char * argv[]) {
    window.setFramerateLimit(frame_rate);
    window.setFramerateLimit(1);
    window.setVerticalSyncEnabled(false);
    ubermenu(UbermenuType::main, false, true);

	return 0;
}


