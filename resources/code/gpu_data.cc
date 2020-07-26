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


    // display_compute_shader = CShader("...");

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
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> colors;

    points.clear();
    points.push_back(glm::vec3(-1, 1, -0.9));  //A
    points.push_back(glm::vec3(-1,-1, -0.9));  //C
    points.push_back(glm::vec3( 1, 1, -0.9));  //B

    points.push_back(glm::vec3( 1, 1, -0.9));  //B
    points.push_back(glm::vec3(-1,-1, -0.9));  //C
    points.push_back(glm::vec3( 1,-1, -0.9));  //D

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

    #define POS  0.125f
    #define NEG -0.125f

    glm::vec3 a(NEG, POS, POS);
    glm::vec3 b(NEG, NEG, POS);
    glm::vec3 c(POS, POS, POS);
    glm::vec3 d(POS, NEG, POS);
    glm::vec3 e(NEG, POS, NEG);
    glm::vec3 f(NEG, NEG, NEG);
    glm::vec3 g(POS, POS, NEG);
    glm::vec3 h(POS, NEG, NEG);

    glm::vec3 offset;
    float factor = 1.0f-0.618f;
    float length = 4.0f;

    offset = glm::vec3(POS, 0, 0);
    glm::vec3 ax = a + offset; ax.x *= length; ax.y *= factor; ax.z *= factor;
    glm::vec3 bx = b + offset; bx.x *= length; bx.y *= factor; bx.z *= factor;
    glm::vec3 cx = c + offset; cx.x *= length; cx.y *= factor; cx.z *= factor;
    glm::vec3 dx = d + offset; dx.x *= length; dx.y *= factor; dx.z *= factor;
    glm::vec3 ex = e + offset; ex.x *= length; ex.y *= factor; ex.z *= factor;
    glm::vec3 fx = f + offset; fx.x *= length; fx.y *= factor; fx.z *= factor;
    glm::vec3 gx = g + offset; gx.x *= length; gx.y *= factor; gx.z *= factor;
    glm::vec3 hx = h + offset; hx.x *= length; hx.y *= factor; hx.z *= factor;

    offset = glm::vec3(0, POS, 0);
    glm::vec3 ay = a + offset; ay.x *= factor; ay.y *= length; ay.z *= factor;
    glm::vec3 by = b + offset; by.x *= factor; by.y *= length; by.z *= factor;
    glm::vec3 cy = c + offset; cy.x *= factor; cy.y *= length; cy.z *= factor;
    glm::vec3 dy = d + offset; dy.x *= factor; dy.y *= length; dy.z *= factor;
    glm::vec3 ey = e + offset; ey.x *= factor; ey.y *= length; ey.z *= factor;
    glm::vec3 fy = f + offset; fy.x *= factor; fy.y *= length; fy.z *= factor;
    glm::vec3 gy = g + offset; gy.x *= factor; gy.y *= length; gy.z *= factor;
    glm::vec3 hy = h + offset; hy.x *= factor; hy.y *= length; hy.z *= factor;

    offset = glm::vec3(0, 0, POS);
    glm::vec3 az = a + offset; az.x *= factor; az.y *= factor; az.z *= length;
    glm::vec3 bz = b + offset; bz.x *= factor; bz.y *= factor; bz.z *= length;
    glm::vec3 cz = c + offset; cz.x *= factor; cz.y *= factor; cz.z *= length;
    glm::vec3 dz = d + offset; dz.x *= factor; dz.y *= factor; dz.z *= length;
    glm::vec3 ez = e + offset; ez.x *= factor; ez.y *= factor; ez.z *= length;
    glm::vec3 fz = f + offset; fz.x *= factor; fz.y *= factor; fz.z *= length;
    glm::vec3 gz = g + offset; gz.x *= factor; gz.y *= factor; gz.z *= length;
    glm::vec3 hz = h + offset; hz.x *= factor; hz.y *= factor; hz.z *= length;

    cube_geometry( a, b, c, d, e, f, g, h, points, normals, colors, glm::vec3(0.618f, 0.618f, 0.618f));
    cube_geometry(ax,bx,cx,dx,ex,fx,gx,hx, points, normals, colors, glm::vec3(0.618f, 0.18f, 0.18f));
    cube_geometry(ay,by,cy,dy,ey,fy,gy,hy, points, normals, colors, glm::vec3(0.18f, 0.618f, 0.18f));
    cube_geometry(az,bz,cz,dz,ez,fz,gz,hz, points, normals, colors, glm::vec3(0.18f, 0.18f, 0.618f));

    // buffer the geometry

    // set up attributes
}

void GLContainer::cube_geometry(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 e, glm::vec3 f, glm::vec3 g, glm::vec3 h, std::vector<glm::vec3> &points, std::vector<glm::vec3> &normals, std::vector<glm::vec3> &colors, glm::vec3 color)
{
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
