/*
 * window.h
 *
 *  Created on: Jan 13, 2016
 *      Author: Dr. Yevgeniy Kolokoltsev
 */

#ifndef WINDOW_H_
#define WINDOW_H_

#include <thread>
#include <memory>
#include <mutex>
#include <functional>
#include <stdexcept>
#include <chrono>
#include <atomic>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "renderer.hpp"

using namespace std;

class Window {
public:
	using tWindowPtr = shared_ptr<Window>;
	using tThreadPtr = unique_ptr<thread>;
	using tRendererPtr = unique_ptr<IRenderer>;
	using tDisplayPtr = unique_ptr<ALLEGRO_DISPLAY,function<void(ALLEGRO_DISPLAY*)>>;
	using tEventQueuePtr = unique_ptr<ALLEGRO_EVENT_QUEUE,function<void(ALLEGRO_EVENT_QUEUE*)>>;

	Window(tRendererPtr&& __renderer) : renderer{move(__renderer)}{
		running = false;
		terminate = false;

		if (!al_init())
			throw runtime_error("failed to initialize allegro");

		if (!al_init_primitives_addon())
			throw runtime_error("failed to initialize allegro primitives addon");
	};

	void create_window(size_t width, size_t height){
		running = true;
		win_msg_proc.reset(new thread(Window::run, width, height, this));
	}

	void close(){
		terminate = true;
		if(win_msg_proc && win_msg_proc->joinable()){
			win_msg_proc->join();
		}
		win_msg_proc = nullptr;
	}

	bool is_running(){return running;};
	virtual ~Window(){close();};


protected:
	virtual void msg_proc(const ALLEGRO_EVENT& ev){};

private:

	static void run(size_t width, size_t height, Window* w){

		w->initialize(width,height);
		w->start_msg_queue();
		w->cleanup();

		w->running = false;
	};

	void initialize(size_t width, size_t height){
		display = tDisplayPtr(al_create_display(width,height),[=](ALLEGRO_DISPLAY* ptr){al_destroy_display(ptr);});
		if(!display) throw runtime_error("failed to create display");

		event_queue = tEventQueuePtr(al_create_event_queue(),[=](ALLEGRO_EVENT_QUEUE* ptr){al_destroy_event_queue(ptr);});
		if(!event_queue) throw runtime_error("failed to create event_queue");

		renderer->attach();
	};

	void start_msg_queue(){
		al_register_event_source(event_queue.get(), al_get_display_event_source(display.get()));
		ALLEGRO_EVENT ev;

		while (1) {
			if(terminate) goto stop;
			while(al_get_next_event(event_queue.get(), &ev)){
				if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) goto stop;
				msg_proc(ev);
			}

			renderer->render();
			this_thread::sleep_for(chrono::milliseconds(40));
		}

		stop:
		al_unregister_event_source(event_queue.get(),al_get_display_event_source(display.get()));
	};

	void cleanup(){
		renderer->detach();
		event_queue = nullptr;
		display = nullptr;
	}

	tDisplayPtr display;
	tEventQueuePtr event_queue;

	tThreadPtr win_msg_proc;
	tRendererPtr renderer;

	atomic_bool terminate;
	atomic_bool running;
};

#endif /* WINDOW_H_ */
