#ifndef GPU_DATA
#define GPU_DATA

#include "includes.h"

class GLContainer
{
    public:

        GLContainer()  {}
        ~GLContainer() {}


        // initialization
        void init() { compile_shaders(); buffer_geometry(); load_textures(); }

        // display function
        void display() { display_block(); display_orientation_widget(); }

        // manipulating the block
        void swap_blocks();

          // shapes

          // GPU-side utilities

          // lighting


        // CPU-side utilities


        // base color
        glm::vec4 clear_color;

        // display parameters - public so they can be manipulated
        float scale = 5.0f, theta = 0.0f, phi = 0.0f;

    private:

        // display helper functions
        void display_block();
        void display_orientation_widget();

        // init helper functions
        void compile_shaders();
        void buffer_geometry();
        void load_textures();

        void cube_geometry(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 e, glm::vec3 f, glm::vec3 g, glm::vec3 h, std::vector<glm::vec3> &points, std::vector<glm::vec3> &normals, std::vector<glm::vec3> &colors, glm::vec3 color);

        // shows the texture containing the rendered block
        GLuint display_compute_shader;   // raycast -> texture
        GLuint display_shader;           // texture -> window
        GLuint display_vao;
        GLuint display_vbo;

        // shows where x, y and z are pointing
        GLuint orientation_widget_shader;   // shows the orientation widget
        GLuint orientation_widget_vao;
        GLuint orientation_widget_vbo;

        // compute shaders
        //  ...
};

#endif
