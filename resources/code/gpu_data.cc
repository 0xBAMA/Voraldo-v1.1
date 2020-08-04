#include "gpu_data.h"
#include "includes.h"

// display functions
void GLContainer::display_block()
{
    // ------------------------
    // compute shader raycasts, puts result into texture

    // Optimization idea: it is not neccesary to raycast if there has been no changes since the last frame -
    //   these changes would include drawing, lighting, rotation, zooming...

    static float temp_scale;
    static float temp_theta;
    static float temp_phi;

    if((temp_scale != scale) || (temp_theta != theta) || (temp_phi != phi))
        redraw_flag = true;

    temp_scale = scale;
    temp_theta = theta;
    temp_phi = phi;

    if(redraw_flag)
    {
        // cout << "redrawing" << endl;

        // do the tile based rendering using the raycast compute shader
        glUseProgram(display_compute_shader);

        // display texture
        glUniform1i(glGetUniformLocation(display_compute_shader, "current"), 0);
        glUniform1i(glGetUniformLocation(display_compute_shader, "block"),   2 + tex_offset);
        glUniform1i(glGetUniformLocation(display_compute_shader, "lighting"), 6);

        // rotation parameters
        glUniform1f(glGetUniformLocation(display_compute_shader, "theta"), theta);
        glUniform1f(glGetUniformLocation(display_compute_shader, "phi"), phi);

        // zoom parameter
        glUniform1f(glGetUniformLocation(display_compute_shader, "scale"), scale);

        // clear color
        glUniform4fv(glGetUniformLocation(display_compute_shader, "clear_color"), 1, glm::value_ptr(clear_color));

        // loop through tiles
        for(int x = 0; x < SSFACTOR*screen_width; x += TILESIZE)
        {
            // update the x offset
            glUniform1i(glGetUniformLocation(display_compute_shader, "x_offset"), x);
            for(int y = 0; y < SSFACTOR*screen_height; y += TILESIZE)
            {
                // update the y offset
                glUniform1i(glGetUniformLocation(display_compute_shader, "y_offset"), y);

                // dispatch tiles
                glDispatchCompute(TILESIZE/32, TILESIZE/32, 1);
            }
        }

        glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT ); // make sure everything finishes before blitting

        redraw_flag = false; // we won't need to draw anything again, till something changes
    }


    // ------------------------
    // display shader takes texture and blits it to the screen

    glUseProgram( display_shader );
    glBindVertexArray( display_vao );
    glBindBuffer( GL_ARRAY_BUFFER, display_vbo );

    glUniform1f(glGetUniformLocation(display_shader, "ssfactor"), SSFACTOR);

    // two triangles, 6 verticies
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

    glUseProgram( orientation_widget_shader );
    glBindVertexArray( orientation_widget_vao );
    glBindBuffer( GL_ARRAY_BUFFER, orientation_widget_vbo );

    ImGuiIO& io = ImGui::GetIO();

    // glUniform1f(glGetUniformLocation(orientation_widget_shader, "time"), 0.001*SDL_GetTicks());
    glUniform1f(glGetUniformLocation(orientation_widget_shader, "theta"), theta);
    glUniform1f(glGetUniformLocation(orientation_widget_shader, "phi"), phi);
    glUniform1f(glGetUniformLocation(orientation_widget_shader, "ratio"), io.DisplaySize.x/io.DisplaySize.y);

    // 4 cubes, 6 faces apiece, 2 triangles per face - total is 144 verticies
    glDrawArrays( GL_TRIANGLES, 0, 144);

}


// ------------------------
// ------------------------
// initialization functions
void GLContainer::compile_shaders() // going to make this more compact this time around
{
    // ------------------------
    // compiling display shaders

    display_compute_shader    = CShader("resources/code/shaders/raycast.cs.glsl").Program;
    display_shader            = Shader("resources/code/shaders/blit.vs.glsl", "resources/code/shaders/blit.fs.glsl").Program;
    orientation_widget_shader = Shader("resources/code/shaders/widget.vs.glsl", "resources/code/shaders/widget.fs.glsl").Program;

    // ------------------------
    // compiling compute shaders - note that ___.cs.glsl is just a placeholder with the bare minimum to compile

    // Shapes
    aabb_compute                 = CShader("resources/code/shaders/___.cs.glsl").Program;
    cuboid_compute               = CShader("resources/code/shaders/___.cs.glsl").Program;
    cylinder_compute             = CShader("resources/code/shaders/___.cs.glsl").Program;
    ellipsoid_compute            = CShader("resources/code/shaders/___.cs.glsl").Program;
    grid_compute                 = CShader("resources/code/shaders/___.cs.glsl").Program;
    heightmap_compute            = CShader("resources/code/shaders/___.cs.glsl").Program;
    perlin_compute               = CShader("resources/code/shaders/___.cs.glsl").Program;
    sphere_compute               = CShader("resources/code/shaders/___.cs.glsl").Program;
    tube_compute                 = CShader("resources/code/shaders/___.cs.glsl").Program;
    triangle_compute             = CShader("resources/code/shaders/___.cs.glsl").Program;

    // GPU-side utilities
    clear_all_compute            = CShader("resources/code/shaders/___.cs.glsl").Program;
    unmask_all_compute           = CShader("resources/code/shaders/___.cs.glsl").Program;
    invert_mask_compute          = CShader("resources/code/shaders/___.cs.glsl").Program;
    mask_by_color_compute        = CShader("resources/code/shaders/___.cs.glsl").Program;
    box_blur_compute             = CShader("resources/code/shaders/___.cs.glsl").Program;
    gaussian_blur_compute        = CShader("resources/code/shaders/___.cs.glsl").Program;
    shift_compute                = CShader("resources/code/shaders/___.cs.glsl").Program;
    copy_loadbuff_compute        = CShader("resources/code/shaders/___.cs.glsl").Program;

    // Lighting
    lighting_clear_compute       = CShader("resources/code/shaders/___.cs.glsl").Program;
    directional_lighting_compute = CShader("resources/code/shaders/___.cs.glsl").Program;
    ambient_occlusion_compute    = CShader("resources/code/shaders/___.cs.glsl").Program;
    fakeGI_compute               = CShader("resources/code/shaders/___.cs.glsl").Program;
    mash_compute                 = CShader("resources/code/shaders/___.cs.glsl").Program;

}

void GLContainer::buffer_geometry()
{
    // ------------------------
    // display geometry, two triangles, just to get a screen's worth of fragments

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
    //  beginning of the project - I hope to keep a more consistent environment
    //  across all the shaders, to make it easier to understand and extend

    // see gpu_data.h for the numbered listing

    // data arrays
    std::vector<unsigned char> ucxor, light, zeroes, random;

    std::default_random_engine generator;
    std::uniform_int_distribution<unsigned char> distribution(0,255);

    for(unsigned int x = 0; x < DIM; x++)
        for(unsigned int y = 0; y < DIM; y++)
            for(unsigned int z = 0; z < DIM; z++)
            {
                for(int i = 0; i < 3; i++) // fill r, g, b with the result of the xor
                    ucxor.push_back(((unsigned char)(x%256) ^ (unsigned char)(y%256) ^ (unsigned char)(z%256)));

                ucxor.push_back(255); // alpha channel gets 255
            }


    for(unsigned int y = 0; y < (screen_height*SSFACTOR); y++)
        for(unsigned int x = 0; x < (screen_width*SSFACTOR); x++)
        {
            random.push_back((unsigned char)(x%256) ^ (unsigned char)(y%256));
            random.push_back(y);
            random.push_back(distribution(generator));
            random.push_back(255);
        }


    light.resize(3*DIM*DIM*DIM, 64); // fill the array with '64'
    zeroes.resize(3*DIM*DIM*DIM, 0); // fill the array with zeroes


    // create all the texture handles
    glGenTextures(13, &textures[0]);


    // main render texture - this is going to be a rectangular texture, larger than the screen so we can do some supersampling
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_RECTANGLE, textures[0]);
    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA16, screen_width*SSFACTOR, screen_height*SSFACTOR, 0, GL_RGBA, GL_UNSIGNED_BYTE, &random[0]);
    glBindImageTexture(0, textures[0], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16); // 16 bits, hopefully higher precision is helpful
    // set up filtering for this texture
    glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    // copy/paste buffer render texture - this is going to be a small rectangular texture, will only be shown inside the menus
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, 512, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindImageTexture(1, textures[1], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    // main block front color buffer - initialize with xor
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_3D, textures[2]);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0,  GL_RGBA, GL_UNSIGNED_BYTE, &ucxor[0]);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTexture(2, textures[2], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);


    // main block back color buffer - initially empty
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_3D, textures[3]);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0,  GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTexture(3, textures[3], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);


    // main block front mask buffer - initially empty
    glActiveTexture(GL_TEXTURE0 + 4);
    glBindTexture(GL_TEXTURE_3D, textures[4]);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, DIM, DIM, DIM, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTexture(4, textures[4], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8);


    // main block back mask buffer - initially empty
    glActiveTexture(GL_TEXTURE0 + 5);
    glBindTexture(GL_TEXTURE_3D, textures[5]);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, DIM, DIM, DIM, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTexture(5, textures[5], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8);


    // display lighting buffer - initialize with some base value representing neutral coloration
    glActiveTexture(GL_TEXTURE0 + 6);
    glBindTexture(GL_TEXTURE_3D, textures[6]);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, DIM, DIM, DIM, 0, GL_RED, GL_UNSIGNED_BYTE, &light[0]);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTexture(6, textures[6], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8);


    // lighting cache buffer - this is going to have the same data in it as the regular lighting buffer initially
    glActiveTexture(GL_TEXTURE0 + 7);
    glBindTexture(GL_TEXTURE_3D, textures[7]);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, DIM, DIM, DIM, 0, GL_RED, GL_UNSIGNED_BYTE, &light[0]);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTexture(7, textures[7], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8);


    // copy/paste front buffer - initally empty
    glActiveTexture(GL_TEXTURE0 + 8);
    glBindTexture(GL_TEXTURE_3D, textures[8]);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0,  GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTexture(8, textures[8], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);


    // copy/paste back buffer - initially empty
    glActiveTexture(GL_TEXTURE0 + 9);
    glBindTexture(GL_TEXTURE_3D, textures[9]);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0,  GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTexture(9, textures[9], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);


    // load buffer - initially empty
    glActiveTexture(GL_TEXTURE0 + 10);
    glBindTexture(GL_TEXTURE_3D, textures[10]);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0,  GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindImageTexture(10, textures[10], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);


    // perlin noise - initialize with noise at some default scaling
    glActiveTexture(GL_TEXTURE0 + 11);
    glBindTexture(GL_TEXTURE_3D, textures[11]);

    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
    // 3d texture for perlin noise - DIM on a side
    generate_perlin_noise(0.014, 0.04, 0.014);


    // heightmap - initialize with a generated diamond square heightmap
    glActiveTexture(GL_TEXTURE0 + 12);
    glBindTexture(GL_TEXTURE_2D, textures[12]);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    // 2d texture for representation of a heightmap (greyscale - use some channels to hold more data?) - also, DIM on a side
    generate_heightmap_diamond_square();
}


// ------------------------
// ------------------------
// manipulating the block
void GLContainer::swap_blocks()
{
    // keep the data from moving
    tex_offset = tex_offset ? 0 : 1; // because the blocks are in neighboring units, adding this value
                                     // to the number of the lower unit works to switch between them
}

// ------------------------
// Shapes -- all will require redraw_flag to be set

       // aabb
void GLContainer::draw_aabb(glm::vec3 min, glm::vec3 max, glm::vec4 color, bool draw, bool mask)
{

}

       // cuboid
void GLContainer::draw_cuboid(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 e, glm::vec3 f, glm::vec3 g, glm::vec3 h, glm::vec4 color, bool draw, bool mask)
{

}

       // cylinder
void GLContainer::draw_cylinder(glm::vec3 bvec, glm::vec3 tvec, float radius, glm::vec4 color, bool draw, bool mask)
{

}

       // ellipsoid
void GLContainer::draw_ellipsoid(glm::vec3 center, glm::vec3 radii, glm::vec3 rotation, glm::vec4 color, bool draw, bool mask)
{

}

       // grid
void GLContainer::draw_grid(glm::ivec3 spacing, glm::ivec3 widths, glm::ivec3 offsets, glm::vec4 color, bool draw, bool mask)
{

}

       // heightmap
void GLContainer::draw_heightmap(float height_scale, bool height_color, glm::vec4 color, bool mask, bool draw)
{

}

       // perlin noise
void GLContainer::draw_perlin_noise(float low_thresh, float high_thresh, bool smooth, glm::vec4 color, bool draw, bool mask)
{

}

       // sphere
void GLContainer::draw_sphere(glm::vec3 location, float radius, glm::vec4 color, bool draw, bool mask)
{

}

       // tube
void GLContainer::draw_tube(glm::vec3 bvec, glm::vec3 tvec, float inner_radius, float outer_radius, glm::vec4 color, bool draw, bool mask)
{

}

       // triangle
void GLContainer::draw_triangle(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, float thickness, glm::vec4 color, bool draw, bool mask)
{

}


// ------------------------
// GPU-side utilities -- all except masking functions will require redraw_flag be set true

        // clear all
void GLContainer::clear_all(bool respect_mask)
{

}

        // unmask all
void GLContainer::unmask_all()
{

}

        // invert mask
void GLContainer::invert_mask()
{

}

        // mask by color
void GLContainer::mask_by_color(bool r, bool g, bool b, bool a, bool l, glm::vec4 color, float l_val, float r_var, float g_var, float b_var, float a_var, float l_var)
{

}

        // box blur
void GLContainer::box_blur(int radius, bool touch_alpha, bool respect_mask)
{

}

        // gaussian blur
void GLContainer::gaussian_blur(int radius, bool touch_alpha, bool respect_mask)
{

}

        // limiter
void GLContainer::limiter()
{

}

        // shifting
void GLContainer::shift(glm::ivec3 movement, bool loop, int mode)
{

}


// ------------------------
// Lighting -- most of these will require redraw_flag be set true

        // lighting clear (to cached level, or to some set level, default zero)
void GLContainer::lighting_clear(bool use_cache_level, float intensity)
{

}

        // directional
void GLContainer::compute_directional_lighting(float theta, float phi, float initial_ray_intensity)
{

}

        // ambient occlusion
void GLContainer::compute_ambient_occlusion(int radius)
{

}

        // fake GI
void GLContainer::compute_fake_GI(float factor, float sky_intensity, float thresh)
{

}

        // mash (combine light into color buffer)
void GLContainer::mash()
{

}


// ------------------------
// ------------------------
// CPU-side utilities

   // functions to generate new heightmaps
void GLContainer::generate_heightmap_diamond_square()
{
    long unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();

    std::default_random_engine engine{seed};
    std::uniform_real_distribution<float> distribution{0, 1};

    constexpr auto size =  DIM + 1;
    constexpr auto edge = size - 1;

    uint8_t map[size][size] = {{0}};
    map[0][0] = map[edge][0] = map[0][edge] = map[edge][edge] = 128;

    heightfield::diamond_square_no_wrap(
        size,
        // random
        [&engine, &distribution](float range)
        {
            return distribution(engine) * range;
        },
        // variance
        [](int level) -> float
        {
            return 64.0f * std::pow(0.5f, level);
        },
        // at
        [&map](int x, int y) -> uint8_t&
        {
            return map[y][x];
        }
    );

    std::vector<unsigned char> data;

    for(int x = 0; x < DIM; x++)
    {
        for(int y = 0; y < DIM; y++)
        {
            data.push_back(map[x][y]);
            data.push_back(map[x][y]);
            data.push_back(map[x][y]);
            data.push_back(255);
        }
    }

    //send it to the GPU
    glBindTexture(GL_TEXTURE_2D, textures[12]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, DIM, DIM, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void GLContainer::generate_heightmap_perlin()
{
    std::vector<unsigned char> data;

    PerlinNoise p;

    float xscale = 0.014f;
    float yscale = 0.04f;

    //might add more parameters at some point

    static float offset = 0;

    for(int x = 0; x < DIM; x++)
    {
        for(int y = 0; y < DIM; y++)
        {
            data.push_back((unsigned char)(p.noise(x*xscale,y*yscale,offset) * 255));
            data.push_back((unsigned char)(p.noise(x*xscale,y*yscale,offset) * 255));
            data.push_back((unsigned char)(p.noise(x*xscale,y*yscale,offset) * 255));
            data.push_back(255);
        }
    }

    offset += 0.5;

    glBindTexture(GL_TEXTURE_2D, textures[12]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, DIM, DIM, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void GLContainer::generate_heightmap_XOR()
{
    //create the byte array
    std::vector<unsigned char> data;

    for(int x = 0; x < DIM; x++)
    {
        for(int y = 0; y < DIM; y++)
        {
            //cout << " "<< ((unsigned char)(x%256) ^ (unsigned char)(y%256));
            data.push_back((unsigned char)(x%256) ^ (unsigned char)(y%256));
            data.push_back((unsigned char)(x%256) ^ (unsigned char)(y%256));
            data.push_back((unsigned char)(x%256) ^ (unsigned char)(y%256));
            data.push_back(255);
        }
    }

    //send the data to the gpu
    glBindTexture(GL_TEXTURE_2D, textures[12]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, DIM, DIM, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
    glGenerateMipmap(GL_TEXTURE_2D);
}

   // function to generate new block of 3d perlin noise
void GLContainer::generate_perlin_noise(float xscale=0.014, float yscale=0.04, float zscale=0.014)
{
    PerlinNoise p;
    std::vector<unsigned char> data;

    for(int x = 0; x < DIM; x++)
        for(int y = 0; y < DIM; y++)
            for(int z = 0; z < DIM; z++)
            {
                data.push_back((unsigned char)(p.noise(x*xscale,y*yscale,z*zscale) * 255));
                data.push_back((unsigned char)(p.noise(x*xscale,y*yscale,z*zscale) * 255));
                data.push_back((unsigned char)(p.noise(x*xscale,y*yscale,z*zscale) * 255));
                data.push_back(255);
            }

    glBindTexture(GL_TEXTURE_3D, textures[11]);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0,  GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
    glGenerateMipmap(GL_TEXTURE_3D);
}

// VAT and Load will need a shader, that can copy and respect the mask - save is more trivial

   // Brent Werness's Voxel Automata Terrain - set redraw_flag to true
std::string GLContainer::vat(float flip, std::string rule, int initmode, glm::vec4 color0, glm::vec4 color1, glm::vec4 color2, float lambda, float beta, float mag, bool respect_mask)
{
    std::string temp;
    return temp;
}

   // load - set redraw_flag to true
void GLContainer::load(std::string filename, bool respect_mask)
{

}

   // save
void GLContainer::save(std::string filename)
{

}
