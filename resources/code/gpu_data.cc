#include "gpu_data.h"
#include "includes.h"

// display functions
void GLContainer::display_block()
{
    // ------------------------
    // compute shader raycasts, puts result into texture


    // ------------------------
    // display shader takes texture and puts it on the screen

    glUseProgram(display_shader);
    glBindVertexArray( display_vao );
    glBindBuffer( GL_ARRAY_BUFFER, display_vbo );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
}

void GLContainer::display_orientation_widget()
{
    // ------------------------
    // this helps the user know where they are placing their geometry
    //  this became a litte bit of an issue sometimes when using v1.0 -
    //  it can be difficult at times to tell where the x, y and z vectors
    //  are once you have rotated around a bit - by using this widget,
    //  I hope to aleviate this as an issue to some extent.

   
}


// ------------------------
// ------------------------
// initialization functions
void GLContainer::compile_shaders()
{
    // ------------------------
    // compiling display shaders

    display_shader = Shader("resources/code/shaders/blit.vs.glsl", "resources/code/shaders/blit.fs.glsl").Program;

   
    // ------------------------
    // compiling compute shaders


}

void GLContainer::buffer_geometry()
{
    // ------------------------
    // display geometry, two triangles, just to get a screen full of fragments

    //  A---------------B
    //  |          .    |
    //  |       .       |
    //  |    .          |
    //  |               |
    //  C---------------D

    // diagonal runs from C to B
    //  A is -1, 1
    //  B is  1, 1
    //  C is -1,-1
    //  D is  1,-1
    std::vector<glm::vec3> points;

    points.clear();
    points.push_back(glm::vec3(-1, 1, 0.5));  //A
    points.push_back(glm::vec3(-1,-1, 0.5));  //C
    points.push_back(glm::vec3( 1, 1, 0.5));  //B

    points.push_back(glm::vec3( 1, 1, 0.5));  //B
    points.push_back(glm::vec3(-1,-1, 0.5));  //C
    points.push_back(glm::vec3( 1,-1, 0.5));  //D

    // vao, vbo
    glGenVertexArrays( 1, &display_vao );
    glBindVertexArray( display_vao );

    glGenBuffers( 1, &display_vbo );
    glBindBuffer( GL_ARRAY_BUFFER, display_vbo );

    // buffer the data
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * points.size(), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * points.size(), &points[0]);
    points.clear(); // done with this data

    // set up attributes
    GLuint points_attrib = glGetAttribLocation(display_shader, "vPosition");
    glEnableVertexAttribArray(points_attrib);
    glVertexAttribPointer(points_attrib, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) (static_cast<const char*>(0) + (0)));



    // ------------------------
    // orientation widget, to indicate the orientation of the block

    // this is going to consist of 4 rectangular prisms
    //  1 - small grey cube centered at the origin
    //  2 - red,   extended along the x axis
    //  3 - green, extended along the y axis
    //  4 - blue,  extended along the z axis

    //      e-------g    +y
    //     /|      /|     |
    //    / |     / |     |___+x
    //   a-------c  |    /
    //   |  f----|--h   +z
    //   | /     | /
    //   |/      |/
    //   b-------d


}

void GLContainer::load_textures()
{
    // ------------------------
    // for v1.1, I am planning out the locations of all textures at the
    //  beginning of the project - I hope to keep a consistent environment
    //  across all the shaders, to make it easier to understand and extend

}


// ------------------------
// ------------------------
// manipulating the block
void GLContainer::swap_blocks()
{

}

// ------------------------
// shapes


// ------------------------
// GPU-side utilities


// ------------------------
// lighting


// ------------------------
// ------------------------
// CPU-side utilities
