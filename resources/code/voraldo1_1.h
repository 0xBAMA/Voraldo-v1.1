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

	ImVec4 clear_color;

	GLuint display_shader;
	GLuint display_vao;
	GLuint display_vbo;


	void create_window();
	void gl_setup();
	void draw_everything();

	void quit();

	bool pquit;

};

#endif
