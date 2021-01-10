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
        bool show_widget = true;
        void display() { display_block(); if(show_widget) display_orientation_widget(); }

        // part of the quitting operation
        void delete_textures();
        
        // manipulating the block
        void swap_blocks();



// Shapes
       // aabb
       void draw_aabb(glm::vec3 min, glm::vec3 max, glm::vec4 color, bool draw, bool mask);

       // cuboid
       void draw_cuboid(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 e, glm::vec3 f, glm::vec3 g, glm::vec3 h, glm::vec4 color, bool draw, bool mask);

       // cylinder
       void draw_cylinder(glm::vec3 bvec, glm::vec3 tvec, float radius, glm::vec4 color, bool draw, bool mask);

       // ellipsoid
       void draw_ellipsoid(glm::vec3 center, glm::vec3 radii, glm::vec3 rotation, glm::vec4 color, bool draw, bool mask);

       // grid
       void draw_grid(glm::ivec3 spacing, glm::ivec3 widths, glm::ivec3 offsets, glm::vec4 color, bool draw, bool mask);

       // heightmap
       void draw_heightmap(float height_scale, bool height_color, glm::vec4 color, bool mask, bool draw);

       // perlin noise
       void draw_perlin_noise(float low_thresh, float high_thresh, bool smooth, glm::vec4 color, bool draw, bool mask);

       // sphere
       void draw_sphere(glm::vec3 location, float radius, glm::vec4 color, bool draw, bool mask);

       // tube
       void draw_tube(glm::vec3 bvec, glm::vec3 tvec, float inner_radius, float outer_radius, glm::vec4 color, bool draw, bool mask);

       // triangle
       void draw_triangle(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, float thickness, glm::vec4 color, bool draw, bool mask);

       // icosahedron
       void draw_regular_icosahedron(double x_rot, double y_rot, double z_rot, double scale, glm::vec3 center_point, glm::vec4 vertex_material, double verticies_radius, glm::vec4 edge_material, double edge_thickness, glm::vec4 face_material, float face_thickness, bool draw, bool mask);


// GPU-side utilities
        // clear all
        void clear_all(bool respect_mask);

        // unmask all
        void unmask_all();

        // invert mask
        void invert_mask();

        // mask by color
        void mask_by_color(bool r, bool g, bool b, bool a, bool l, glm::vec4 color, float l_val, float r_var, float g_var, float b_var, float a_var, float l_var);

        // box blur
        void box_blur(int radius, bool touch_alpha, bool respect_mask);

        // gaussian blur
        void gaussian_blur(int radius, bool touch_alpha, bool respect_mask);

        // limiter - details tbd
        void limiter();

        // shifting
        void shift(glm::ivec3 movement, bool loop, int mode);



// Lighting
        // lighting clear (to cached level, or to some set level, default zero)
        void lighting_clear(bool use_cache_level, float intensity = 0.0);

        // directional
        void compute_new_directional_lighting(float theta, float phi, float initial_ray_intensity, float decay_power);

        // point lighting
        void compute_point_lighting(glm::vec3 location, float initial_intensity, float decay_power, float distance_power);

        // cone lighting
        void compute_cone_lighting(glm::vec3 location, float theta, float phi, float cone_angle, float initial_intensity, float decay_power, float distance_power);
        
        // ambient occlusion
        void compute_ambient_occlusion(int radius);

        // fake GI
        void compute_fake_GI(float factor, float sky_intensity, float thresh);

        // mash (combine light into color buffer)
        void mash();


// CPU-side utilities
        // functions to generate new heightmaps & buffer them to the GPU
        void generate_heightmap_diamond_square();
        void generate_heightmap_perlin();
        void generate_heightmap_XOR();

        // generate 3d perlin noise & buffer to the GPU
        void generate_perlin_noise(float xscale, float yscale, float zscale);


        // this is a helper function, called by both VAT and Load
        void copy_loadbuffer(bool respect_mask);

        // Brent Werness's Voxel Automata Terrain
        std::string vat(float flip, std::string rule, int initmode, glm::vec4 color0, glm::vec4 color1, glm::vec4 color2, float lambda, float beta, float mag, bool respect_mask, glm::bvec3 mins, glm::bvec3 maxs);

        // load
        void load(std::string filename, bool respect_mask);

        // save
        void save(std::string filename);




        // OpenGL clear color
        glm::vec4 clear_color;

        // move the orientation widget around
        glm::vec3 orientation_widget_offset = glm::vec3(0.9, -0.74, 0.0);

        // display parameters - public so they can be manipulated
        float scale = 7.0f, theta = 0.0f, phi = 0.0f;

        float alpha_correction_power = 2.0;

        unsigned int screen_width, screen_height;
        int clickndragx = 0;
        int clickndragy = 0;
        
    private:

        bool redraw_flag = true;
        int tex_offset = 0; //this is better than rebinding textures, it is either 0 or 1

        // display helper functions
        void display_block();
        void display_orientation_widget();


        // init helper functions
        void compile_shaders();
        void buffer_geometry();
        void load_textures();


        // helper function for buffer_geometry (used to generate the orientation widget)
        void cube_geometry(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 e, glm::vec3 f, glm::vec3 g, glm::vec3 h, std::vector<glm::vec3> &points, std::vector<glm::vec3> &normals, std::vector<glm::vec3> &colors, glm::vec3 color);


    // The texture breakdown is as follows:
    //  0  - main block render texture
    //  1  - copy/paste buffer's render texture
    //  2  - main block front color buffer
    //  3  - main block back color buffer
    //  4  - main block front mask buffer
    //  5  - main block back mask buffer
    //  6  - display lighting buffer
    //  7  - lighting cache buffer
    //  8  - copy/paste front buffer
    //  9  - copy/paste back buffer
    //  10 - load buffer (used for load, Voxel Automata Terrain)
    //  11 - perlin noise
    //  12 - heightmap

        GLuint textures[13];


        // shows the texture containing the rendered block - workgroup is 32x32x1
        GLuint display_compute_shader;   // raycast -> texture
        GLuint display_shader;           // texture -> window
        GLuint display_vao;
        GLuint display_vbo;


        // shows where x, y and z are pointing
        GLuint orientation_widget_shader;   // shows the orientation widget
        GLuint orientation_widget_vao;
        GLuint orientation_widget_vbo;


        // Compute Shader Handles
        // Shapes
        GLuint aabb_compute;
        GLuint cuboid_compute;
        GLuint cylinder_compute;
        GLuint ellipsoid_compute;
        GLuint grid_compute;
        GLuint heightmap_compute;
        GLuint perlin_compute;
        GLuint sphere_compute;
        GLuint tube_compute;
        GLuint triangle_compute;

        // GPU-side utilities
        GLuint clear_all_compute;
        GLuint unmask_all_compute;
        GLuint invert_mask_compute;
        GLuint mask_by_color_compute;
        GLuint box_blur_compute;
        GLuint gaussian_blur_compute; 
        GLuint shift_compute;
        GLuint copy_loadbuff_compute;

        // Lighting
        GLuint lighting_clear_compute;
        GLuint new_directional_lighting_compute;
        GLuint point_lighting_compute;
        GLuint cone_lighting_compute;
        GLuint ambient_occlusion_compute;
        GLuint fakeGI_compute;
        GLuint mash_compute;
};

#endif
