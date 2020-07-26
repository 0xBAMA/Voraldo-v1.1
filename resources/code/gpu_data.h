#ifndef GPU_DATA
#define GPU_DATA

#include "includes.h"

class GLContainer
{
    public:

        GLContainer()  {}
        ~GLContainer() {}


        // initialization
        void init() { compile_shaders(); buffer_geometry(); }


        // display functions
        void display_block();
        void display_orientation_widget();


        // manipulating the block

          // shapes

          // GPU-side utilities

          // lighting


        // CPU-side utilities



        // base color
        glm::vec4 clear_color;

       
        // display parameters
        float scale, theta, phi;

    private:

        // init helper functions
        void compile_shaders();
        void buffer_geometry();

        // shows the texture containing the rendered block
        GLuint display_shader;
        GLuint display_vao;
        GLuint display_vbo;


        // shows where x, y and z are pointing
        GLuint orientation_widget_shader;
        GLuint orientation_widget_vao;
        GLuint orientation_widget_vbo;

};

#endif
