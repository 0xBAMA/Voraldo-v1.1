#ifndef RTTNW
#define RTTNW

#include "includes.h"

class Voraldo
{
	public:

		Voraldo();
		~Voraldo();

	private:

		SDL_Window * window;
		SDL_GLContext GLcontext;

		int total_screen_width;
		int total_screen_height;

		ImVec4 clear_color;
		GLContainer GPU_Data;

		std::deque<float> fps_history;

		bool show_fpsoverlay  = true;
		bool show_controls    = true;
		bool show_demo_window = false;
		bool show_menu        = true;
		bool quit_confirm     = false;

		void ControlWindow(bool *open);
		void AppMainMenuBar(bool *open);
		void ShowExampleMenuFile();
		void FPSOverlay(bool* p_open);
		void HelpMarker(const char* indicator, const char* desc);
		void QuitConfirm(bool *open);
		void WrappedText(const char* string, float wrap);
		
		void create_window();
		void gl_setup();
		void draw_everything();

		void quit();

		bool pquit;
};

#endif
