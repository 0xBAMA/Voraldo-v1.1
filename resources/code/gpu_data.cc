#include "gpu_data.h"
#include "includes.h"

// display functions
void GLContainer::display_block()
{
    // ------------------------
    // compute shader raycasts, puts result into texture


    // ------------------------
    // display shader takes texture and puts it on the screen

    glUseProgram( display_shader );
    glBindVertexArray( display_vao );
    glBindBuffer( GL_ARRAY_BUFFER, display_vbo );

    // two triangles, 6 verticies
    glDrawArrays( GL_TRIANGLES, 0, 6 );


    glUseProgram( orientation_widget_shader );
    glBindVertexArray( orientation_widget_vao );
    glBindBuffer( GL_ARRAY_BUFFER, orientation_widget_vbo );

    glUniform1f(glGetUniformLocation(orientation_widget_shader, "time"), 0.001*SDL_GetTicks());

    // 4 cubes, 6 faces apiece, 2 triangles per face - total is 144 verticies
    glDrawArrays( GL_TRIANGLES, 0, 144);
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
    orientation_widget_shader = Shader("resources/code/shaders/widget.vs.glsl", "resources/code/shaders/widget.fs.glsl").Program;

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
    points.push_back(glm::vec3(-1, 1, 0.9));  //A
    points.push_back(glm::vec3(-1,-1, 0.9));  //C
    points.push_back(glm::vec3( 1, 1, 0.9));  //B

    points.push_back(glm::vec3( 1, 1, 0.9));  //B
    points.push_back(glm::vec3(-1,-1, 0.9));  //C
    points.push_back(glm::vec3( 1,-1, 0.9));  //D

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
    float factor = 0.25f;
    float length = 2.0f;

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
    cube_geometry(ax,bx,cx,dx,ex,fx,gx,hx, points, normals, colors, glm::vec3(0.618f, 0.180f, 0.180f));
    cube_geometry(ay,by,cy,dy,ey,fy,gy,hy, points, normals, colors, glm::vec3(0.180f, 0.618f, 0.180f));
    cube_geometry(az,bz,cz,dz,ez,fz,gz,hz, points, normals, colors, glm::vec3(0.180f, 0.180f, 0.618f));

    // vao, vbo
    glGenVertexArrays( 1, &orientation_widget_vao );
    glBindVertexArray( orientation_widget_vao );

    glGenBuffers( 1, &orientation_widget_vbo );
    glBindBuffer( GL_ARRAY_BUFFER, orientation_widget_vbo );

    // buffer the data
    int num_bytes_points =  sizeof(glm::vec3) * points.size();
    int num_bytes_normals = sizeof(glm::vec3) * normals.size();
    int num_bytes_colors =  sizeof(glm::vec3) * colors.size();
    int total_size = num_bytes_points + num_bytes_normals + num_bytes_colors;

    glBufferData(GL_ARRAY_BUFFER, total_size, NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, num_bytes_points, &points[0]);
    glBufferSubData(GL_ARRAY_BUFFER, num_bytes_points, num_bytes_normals, &normals[0]);
    glBufferSubData(GL_ARRAY_BUFFER, num_bytes_points + num_bytes_normals, num_bytes_colors, &colors[0]);

    // set up attributes
    GLuint points_attribute = glGetAttribLocation(orientation_widget_shader, "vPosition");
    glEnableVertexAttribArray(points_attribute);
    glVertexAttribPointer(points_attribute, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) (static_cast<const char*>(0) + (0)));

    GLuint normals_attribute = glGetAttribLocation(orientation_widget_shader, "vNormal");
    glEnableVertexAttribArray(normals_attribute);
    glVertexAttribPointer(normals_attribute, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) (static_cast<const char*>(0) + (num_bytes_points)));

    GLuint colors_attribute = glGetAttribLocation(orientation_widget_shader, "vColor");
    glEnableVertexAttribArray(colors_attribute);
    glVertexAttribPointer(colors_attribute, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) (static_cast<const char*>(0) + (num_bytes_points + num_bytes_normals)));

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

    glm::vec3 normal;

    // face ABCD
    normal = glm::normalize(glm::cross(a-b, a-c));
    points.push_back(a);
    points.push_back(b);
    points.push_back(c);

    points.push_back(c);
    points.push_back(b);
    points.push_back(d);
    for(int i = 0; i < 6; i++)
    {
        normals.push_back(normal);
        colors.push_back(color);
    }

    // face CDGH
    normal = glm::normalize(glm::cross(c-d, c-g));
    points.push_back(c);
    points.push_back(d);
    points.push_back(g);

    points.push_back(g);
    points.push_back(d);
    points.push_back(h);
    for(int i = 0; i < 6; i++)
    {
        normals.push_back(normal);
        colors.push_back(color);
    }

    // face EGFH
    normal = glm::normalize(glm::cross(g-h, g-e));
    points.push_back(f);
    points.push_back(g);
    points.push_back(h);

    points.push_back(g);
    points.push_back(f);
    points.push_back(e);
    for(int i = 0; i < 6; i++)
    {
        normals.push_back(normal);
        colors.push_back(color);
    }

    // face AEBF
    normal = glm::normalize(glm::cross(e-f, e-a));
    points.push_back(a);
    points.push_back(e);
    points.push_back(f);

    points.push_back(f);
    points.push_back(a);
    points.push_back(b);
    for(int i = 0; i < 6; i++)
    {
        normals.push_back(normal);
        colors.push_back(color);
    }

    // face AECG
    normal = glm::normalize(glm::cross(e-a, e-g));
    points.push_back(a);
    points.push_back(e);
    points.push_back(c);

    points.push_back(c);
    points.push_back(e);
    points.push_back(g);
    for(int i = 0; i < 6; i++)
    {
        normals.push_back(normal);
        colors.push_back(color);
    }

    // face BDFH
    normal = glm::normalize(glm::cross(b-f, b-d));
    points.push_back(b);
    points.push_back(f);
    points.push_back(d);

    points.push_back(f);
    points.push_back(d);
    points.push_back(h);
    for(int i = 0; i < 6; i++)
    {
        normals.push_back(normal);
        colors.push_back(color);
    }
   
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
