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
    static float acp; // alpha correction power
    static glm::vec4 temp_clear_color;

    if((temp_scale != scale) || (temp_theta != theta) || (temp_phi != phi) || (acp != alpha_correction_power) || (clear_color != temp_clear_color))
        redraw_flag = true;

    temp_scale = scale;
    temp_theta = theta;
    temp_phi = phi;
    acp = alpha_correction_power;
    temp_clear_color = clear_color;

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

        // alpha power
        glUniform1f(glGetUniformLocation(display_compute_shader, "upow"), alpha_correction_power);

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

    // clear the screen
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);   //from hsv picker
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                     //clear the background

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

    glUniform3fv(glGetUniformLocation(orientation_widget_shader, "offset"), 1, glm::value_ptr(orientation_widget_offset));

    // 4 cubes, 6 faces apiece, 2 triangles per face - total is 144 verticies
    glDrawArrays( GL_TRIANGLES, 0, 144);

}


// ------------------------
// ------------------------
// initialization functions
void GLContainer::compile_shaders() // going to make this more compact this time around
{
  // reporting status is not super important, just compile them - may add some code to the
  // shader class (and cshader class) to report the filename upon unsuccessful compilation

    // ------------------------
    // compiling display shaders

    display_compute_shader    = CShader("resources/code/shaders/raycast.cs.glsl").Program;
    display_shader            = Shader("resources/code/shaders/blit.vs.glsl", "resources/code/shaders/blit.fs.glsl").Program;
    orientation_widget_shader = Shader("resources/code/shaders/widget.vs.glsl", "resources/code/shaders/widget.fs.glsl").Program;
    cout << "display shaders done." << endl;

    // ------------------------
    // compiling compute shaders - note that ___.cs.glsl is just a placeholder with the bare minimum to compile

    // Shapes
    aabb_compute                 = CShader("resources/code/shaders/aabb.cs.glsl").Program;               cout << "aabb shader done." << endl;
    cuboid_compute               = CShader("resources/code/shaders/cuboid.cs.glsl").Program;             cout << "cuboid shader done." << endl;
    cylinder_compute             = CShader("resources/code/shaders/cylinder.cs.glsl").Program;           cout << "cylinder shader done." << endl;
    ellipsoid_compute            = CShader("resources/code/shaders/ellipsoid.cs.glsl").Program;          cout << "ellipsoid shader done." << endl;
    grid_compute                 = CShader("resources/code/shaders/grid.cs.glsl").Program;               cout << "grid shader done." << endl;
    heightmap_compute            = CShader("resources/code/shaders/heightmap.cs.glsl").Program;          cout << "heightmap shader done." << endl;
    perlin_compute               = CShader("resources/code/shaders/perlin.cs.glsl").Program;             cout << "perlin shader done." << endl;
    sphere_compute               = CShader("resources/code/shaders/sphere.cs.glsl").Program;             cout << "sphere shader done." << endl;
    tube_compute                 = CShader("resources/code/shaders/tube.cs.glsl").Program;               cout << "tube shader done." << endl;
    triangle_compute             = CShader("resources/code/shaders/triangle.cs.glsl").Program;           cout << "triangle shader done." << endl;

    // GPU-side utilities
    clear_all_compute            = CShader("resources/code/shaders/clear_all.cs.glsl").Program;          cout << "clear_all shader done." << endl;
    unmask_all_compute           = CShader("resources/code/shaders/unmask_all.cs.glsl").Program;         cout << "unmask_all shader done." << endl;
    invert_mask_compute          = CShader("resources/code/shaders/invert_mask.cs.glsl").Program;        cout << "invert_mask shader done." << endl;
    mask_by_color_compute        = CShader("resources/code/shaders/mask_by_color.cs.glsl").Program;      cout << "mask_by_color shader done." << endl;
    box_blur_compute             = CShader("resources/code/shaders/box_blur.cs.glsl").Program;           cout << "box blur shader done." << endl;
    gaussian_blur_compute        = CShader("resources/code/shaders/___.cs.glsl").Program;
    shift_compute                = CShader("resources/code/shaders/shift.cs.glsl").Program;              cout << "shift shader done." << endl;
    copy_loadbuff_compute        = CShader("resources/code/shaders/copy_loadbuff.cs.glsl").Program;      cout << "loadbuffer copy shader done." << endl;

    // Lighting
    lighting_clear_compute       = CShader("resources/code/shaders/light_clear.cs.glsl").Program;        cout << "light_clear shader done." << endl;
    directional_lighting_compute = CShader("resources/code/shaders/directional.cs.glsl").Program;        cout << "directional light shader done." << endl;
    new_directional_lighting_compute = CShader("resources/code/shaders/new_directional.cs.glsl").Program;        cout << "new directional light shader done." << endl;
    ambient_occlusion_compute    = CShader("resources/code/shaders/ambient_occlusion.cs.glsl").Program;  cout << "ambient occlusion shader done." << endl;
    fakeGI_compute               = CShader("resources/code/shaders/fakeGI.cs.glsl").Program;             cout << "fake global illumination shader done." << endl;
    mash_compute                 = CShader("resources/code/shaders/mash.cs.glsl").Program;               cout << "lighting mash shader done." << endl;

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
                for(int i = 0; i < 4; i++) // fill r, g, b with the result of the xor
                    ucxor.push_back(((unsigned char)(x%256) ^ (unsigned char)(y%256) ^ (unsigned char)(z%256)));

                // ucxor.push_back(distribution(generator)); // alpha channel gets 255
            }


    // for(unsigned int y = 0; y < (screen_height*SSFACTOR); y++)
    //     for(unsigned int x = 0; x < (screen_width*SSFACTOR); x++)
    //     {
    //         random.push_back((unsigned char)(x%256) ^ (unsigned char)(y%256));
    //         random.push_back(y);
    //         random.push_back(distribution(generator));
    //         random.push_back(255);
    //     }

    random.resize(4*screen_height*screen_width*SSFACTOR*SSFACTOR);
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
    tex_offset = tex_offset==1 ? 0 : 1; // because the blocks are in neighboring units, adding this value
                                     // to the number of the lower unit works to switch between them
}

// ------------------------
// Shapes -- all will require redraw_flag to be set

       // aabb
void GLContainer::draw_aabb(glm::vec3 min, glm::vec3 max, glm::vec4 color, bool draw, bool mask)
{
    // need to redraw after any drawing operation is done
    redraw_flag = true;

    swap_blocks();
    glUseProgram(aabb_compute);

    // Uniforms
    glUniform1i(glGetUniformLocation(aabb_compute, "mask"), mask);
    glUniform1i(glGetUniformLocation(aabb_compute, "draw"), draw);
    glUniform4fv(glGetUniformLocation(aabb_compute, "color"), 1, glm::value_ptr(color));

    glUniform3fv(glGetUniformLocation(aabb_compute, "mins"), 1, glm::value_ptr(min));
    glUniform3fv(glGetUniformLocation(aabb_compute, "maxs"), 1, glm::value_ptr(max));

    glUniform1i(glGetUniformLocation(aabb_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(aabb_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(aabb_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(aabb_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

       // cuboid
void GLContainer::draw_cuboid(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 e, glm::vec3 f, glm::vec3 g, glm::vec3 h, glm::vec4 color, bool draw, bool mask)
{
    redraw_flag = true;

    swap_blocks();
    glUseProgram(cuboid_compute);

    glUniform1i(glGetUniformLocation(cuboid_compute, "mask"), mask);
    glUniform1i(glGetUniformLocation(cuboid_compute, "draw"), draw);
    glUniform4fv(glGetUniformLocation(cuboid_compute, "color"), 1, glm::value_ptr(color));

    glUniform3fv(glGetUniformLocation(cuboid_compute, "a"), 1, glm::value_ptr(a));
    glUniform3fv(glGetUniformLocation(cuboid_compute, "b"), 1, glm::value_ptr(b));
    glUniform3fv(glGetUniformLocation(cuboid_compute, "c"), 1, glm::value_ptr(c));
    glUniform3fv(glGetUniformLocation(cuboid_compute, "d"), 1, glm::value_ptr(d));
    glUniform3fv(glGetUniformLocation(cuboid_compute, "e"), 1, glm::value_ptr(e));
    glUniform3fv(glGetUniformLocation(cuboid_compute, "f"), 1, glm::value_ptr(f));
    glUniform3fv(glGetUniformLocation(cuboid_compute, "g"), 1, glm::value_ptr(g));
    glUniform3fv(glGetUniformLocation(cuboid_compute, "h"), 1, glm::value_ptr(h));

    glUniform1i(glGetUniformLocation(aabb_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(aabb_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(aabb_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(aabb_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

       // cylinder
void GLContainer::draw_cylinder(glm::vec3 bvec, glm::vec3 tvec, float radius, glm::vec4 color, bool draw, bool mask)
{
    redraw_flag = true;

    swap_blocks();
    glUseProgram(cylinder_compute);

    glUniform1i(glGetUniformLocation(cylinder_compute, "mask"), mask);
    glUniform1i(glGetUniformLocation(cylinder_compute, "draw"), draw);
    glUniform4fv(glGetUniformLocation(cylinder_compute, "color"), 1, glm::value_ptr(color));

    glUniform1fv(glGetUniformLocation(cylinder_compute, "radius"), 1, &radius);
    glUniform3fv(glGetUniformLocation(cylinder_compute, "bvec"), 1, glm::value_ptr(bvec));
    glUniform3fv(glGetUniformLocation(cylinder_compute, "tvec"), 1, glm::value_ptr(tvec));

    glUniform1i(glGetUniformLocation(aabb_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(aabb_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(aabb_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(aabb_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

       // ellipsoid
void GLContainer::draw_ellipsoid(glm::vec3 center, glm::vec3 radii, glm::vec3 rotation, glm::vec4 color, bool draw, bool mask)
{
    redraw_flag = true;

    swap_blocks();
    glUseProgram(ellipsoid_compute);

    glUniform1i(glGetUniformLocation(ellipsoid_compute, "mask"), mask);
    glUniform1i(glGetUniformLocation(ellipsoid_compute, "draw"), draw);
    glUniform4fv(glGetUniformLocation(ellipsoid_compute, "color"), 1, glm::value_ptr(color));

    glUniform3fv(glGetUniformLocation(ellipsoid_compute, "radii"), 1, glm::value_ptr(radii));
    glUniform3fv(glGetUniformLocation(ellipsoid_compute, "rotation"), 1, glm::value_ptr(rotation));
    glUniform3fv(glGetUniformLocation(ellipsoid_compute, "center"), 1, glm::value_ptr(center));

    glUniform1i(glGetUniformLocation(ellipsoid_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(ellipsoid_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(ellipsoid_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(ellipsoid_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

       // grid
void GLContainer::draw_grid(glm::ivec3 spacing, glm::ivec3 widths, glm::ivec3 offsets, glm::vec4 color, bool draw, bool mask)
{
    redraw_flag = true;

    swap_blocks();
    glUseProgram(grid_compute);

    glUniform1i(glGetUniformLocation(grid_compute, "mask"), mask);
    glUniform1i(glGetUniformLocation(grid_compute, "draw"), draw);
    glUniform4fv(glGetUniformLocation(grid_compute, "color"), 1, glm::value_ptr(color));

    glUniform3i(glGetUniformLocation(grid_compute, "spacing"), spacing.x, spacing.y, spacing.z);
    glUniform3i(glGetUniformLocation(grid_compute, "offsets"), offsets.x, offsets.y, offsets.z);
    glUniform3i(glGetUniformLocation(grid_compute, "width"), widths.x, widths.y, widths.z);

    glUniform1i(glGetUniformLocation(grid_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(grid_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(grid_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(grid_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

       // heightmap
void GLContainer::draw_heightmap(float height_scale, bool height_color, glm::vec4 color, bool mask, bool draw)
{
    redraw_flag = true;

    swap_blocks();
    glUseProgram(heightmap_compute);

    glUniform1i(glGetUniformLocation(heightmap_compute, "mask"), mask);
    glUniform1i(glGetUniformLocation(heightmap_compute, "draw"), draw);
    glUniform4fv(glGetUniformLocation(heightmap_compute, "color"), 1, glm::value_ptr(color));

    glUniform1i(glGetUniformLocation(heightmap_compute, "height_color"), height_color);
    glUniform1i(glGetUniformLocation(heightmap_compute, "map"), 12);
    glUniform1f(glGetUniformLocation(heightmap_compute, "vscale"), height_scale);

    glUniform1i(glGetUniformLocation(heightmap_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(heightmap_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(heightmap_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(heightmap_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

       // perlin noise
void GLContainer::draw_perlin_noise(float low_thresh, float high_thresh, bool smooth, glm::vec4 color, bool draw, bool mask)
{
    redraw_flag = true;

    swap_blocks();
    glUseProgram(perlin_compute);

    glUniform1i(glGetUniformLocation(perlin_compute, "usmooth"), smooth);

    glUniform1i(glGetUniformLocation(perlin_compute, "mask"), mask);
    glUniform1i(glGetUniformLocation(perlin_compute, "draw"), draw);
    glUniform4fv(glGetUniformLocation(perlin_compute, "ucolor"), 1, glm::value_ptr(color));

    glUniform1i(glGetUniformLocation(perlin_compute, "tex"), 11);

    glUniform1f(glGetUniformLocation(perlin_compute, "low_thresh"), low_thresh);
    glUniform1f(glGetUniformLocation(perlin_compute, "high_thresh"), high_thresh);

    glUniform1i(glGetUniformLocation(heightmap_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(heightmap_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(heightmap_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(heightmap_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

       // sphere
void GLContainer::draw_sphere(glm::vec3 location, float radius, glm::vec4 color, bool draw, bool mask)
{
    redraw_flag = true;

    swap_blocks();
    glUseProgram(sphere_compute);

    glUniform1i(glGetUniformLocation(sphere_compute, "mask"), mask);
    glUniform1i(glGetUniformLocation(sphere_compute, "draw"), draw);
    glUniform4fv(glGetUniformLocation(sphere_compute, "color"), 1, glm::value_ptr(color));

    glUniform1fv(glGetUniformLocation(sphere_compute, "radius"), 1, &radius);
    glUniform3fv(glGetUniformLocation(sphere_compute, "location"), 1, glm::value_ptr(location));

    glUniform1i(glGetUniformLocation(sphere_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(sphere_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(sphere_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(sphere_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

       // tube
void GLContainer::draw_tube(glm::vec3 bvec, glm::vec3 tvec, float inner_radius, float outer_radius, glm::vec4 color, bool draw, bool mask)
{
    redraw_flag = true;

    swap_blocks();
    glUseProgram(tube_compute);

    glUniform1i(glGetUniformLocation(tube_compute, "mask"), mask);
    glUniform1i(glGetUniformLocation(tube_compute, "draw"), draw);
    glUniform1fv(glGetUniformLocation(tube_compute, "iradius"), 1, &inner_radius);
    glUniform1fv(glGetUniformLocation(tube_compute, "oradius"), 1, &outer_radius);
    glUniform3fv(glGetUniformLocation(tube_compute, "bvec"), 1, glm::value_ptr(bvec));
    glUniform3fv(glGetUniformLocation(tube_compute, "tvec"), 1, glm::value_ptr(tvec));
    glUniform4fv(glGetUniformLocation(tube_compute, "color"), 1, glm::value_ptr(color));

    glUniform1i(glGetUniformLocation(tube_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(tube_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(tube_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(tube_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

       // triangle
void GLContainer::draw_triangle(glm::vec3 point1, glm::vec3 point2, glm::vec3 point3, float thickness, glm::vec4 color, bool draw, bool mask)
{
    redraw_flag = true;

    swap_blocks();
    glUseProgram(triangle_compute);

    glUniform1i(glGetUniformLocation(triangle_compute, "mask"), mask);
    glUniform1i(glGetUniformLocation(triangle_compute, "draw"), draw);
    glUniform4fv(glGetUniformLocation(triangle_compute, "color"), 1, glm::value_ptr(color));

    glUniform1fv(glGetUniformLocation(triangle_compute, "thickness"), 1, &thickness);
    glUniform3fv(glGetUniformLocation(triangle_compute, "point1"), 1, glm::value_ptr(point1));
    glUniform3fv(glGetUniformLocation(triangle_compute, "point2"), 1, glm::value_ptr(point2));
    glUniform3fv(glGetUniformLocation(triangle_compute, "point3"), 1, glm::value_ptr(point3));

    glUniform1i(glGetUniformLocation(triangle_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(triangle_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(triangle_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(triangle_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}


// ------------------------
// GPU-side utilities -- all except masking functions will require redraw_flag be set true

        // clear all
void GLContainer::clear_all(bool respect_mask)
{
    redraw_flag = true;

    swap_blocks();
    glUseProgram(clear_all_compute);

    glUniform1i(glGetUniformLocation(clear_all_compute, "respect_mask"), respect_mask);

    glUniform1i(glGetUniformLocation(clear_all_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(clear_all_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(clear_all_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(clear_all_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

        // unmask all
void GLContainer::unmask_all()
{
    // don't need to redraw
    swap_blocks();
    glUseProgram(unmask_all_compute);

    glUniform1i(glGetUniformLocation(unmask_all_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(unmask_all_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(unmask_all_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(unmask_all_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

        // invert mask
void GLContainer::invert_mask()
{
    // don't need to redraw
    swap_blocks();
    glUseProgram(invert_mask_compute);

    glUniform1i(glGetUniformLocation(invert_mask_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(invert_mask_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(invert_mask_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(invert_mask_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

        // mask by color
void GLContainer::mask_by_color(bool r, bool g, bool b, bool a, bool l, glm::vec4 color, float l_val, float r_var, float g_var, float b_var, float a_var, float l_var)
{
    // don't need to redraw
    swap_blocks();
    glUseProgram(mask_by_color_compute);
 
    glUniform1i(glGetUniformLocation(mask_by_color_compute, "use_r"), r);
    glUniform1i(glGetUniformLocation(mask_by_color_compute, "use_g"), g);
    glUniform1i(glGetUniformLocation(mask_by_color_compute, "use_b"), b);
    glUniform1i(glGetUniformLocation(mask_by_color_compute, "use_a"), a);
    glUniform1i(glGetUniformLocation(mask_by_color_compute, "use_l"), l);

    glUniform4fv(glGetUniformLocation(mask_by_color_compute, "color"), 1, glm::value_ptr(color));
    glUniform1f(glGetUniformLocation(mask_by_color_compute, "l_val"), l_val);

    glUniform1f(glGetUniformLocation(mask_by_color_compute, "r_var"), r_var);
    glUniform1f(glGetUniformLocation(mask_by_color_compute, "g_var"), g_var);
    glUniform1f(glGetUniformLocation(mask_by_color_compute, "b_var"), b_var);
    glUniform1f(glGetUniformLocation(mask_by_color_compute, "a_var"), a_var);
    glUniform1f(glGetUniformLocation(mask_by_color_compute, "l_var"), l_var);

    glUniform1i(glGetUniformLocation(mask_by_color_compute, "lighting"), 6);

    glUniform1i(glGetUniformLocation(mask_by_color_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(mask_by_color_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(mask_by_color_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(mask_by_color_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

        // box blur
void GLContainer::box_blur(int radius, bool touch_alpha, bool respect_mask)
{
    redraw_flag = true;
    swap_blocks();
    glUseProgram(box_blur_compute);

    glUniform1i(glGetUniformLocation(box_blur_compute, "radius"), radius);
    glUniform1i(glGetUniformLocation(box_blur_compute, "respect_mask"), respect_mask);
    glUniform1i(glGetUniformLocation(box_blur_compute, "touch_alpha"), touch_alpha);

    glUniform1i(glGetUniformLocation(box_blur_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(box_blur_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(box_blur_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(box_blur_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

        // gaussian blur
void GLContainer::gaussian_blur(int radius, bool touch_alpha, bool respect_mask)
{
    redraw_flag = true;

    // I think I'm going to restrict the range of radii, since I'm not sure about what the best way to do different sized kernels is
}

        // limiter
void GLContainer::limiter()
{
    redraw_flag = true;

    // the details of this operation still need to be worked out - there is a couple of different modes
}

        // shifting
void GLContainer::shift(glm::ivec3 movement, bool loop, int mode)
{
    redraw_flag = true;
    swap_blocks();

    glUseProgram(shift_compute);

    glUniform1i(glGetUniformLocation(shift_compute, "loop"), loop);
    glUniform1i(glGetUniformLocation(shift_compute, "mode"),  mode);
    glUniform3i(glGetUniformLocation(shift_compute, "movement"), movement.x, movement.y, movement.z);

    // glUniform1i(glGetUniformLocation(shift_compute, "lighting"), 6);
    
    glUniform1i(glGetUniformLocation(shift_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(shift_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(shift_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(shift_compute, "previous_mask"), 5-tex_offset);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}


// ------------------------
// Lighting -- all of these will require redraw_flag be set true

        // lighting clear (to cached level, or to some set level, default zero)
void GLContainer::lighting_clear(bool use_cache_level, float intensity)
{
    redraw_flag = true;

    glUseProgram(lighting_clear_compute);

    glUniform1i(glGetUniformLocation(lighting_clear_compute, "lighting"), 6);
    glUniform1i(glGetUniformLocation(lighting_clear_compute, "lighting_cache"), 7);
    glUniform1i(glGetUniformLocation(lighting_clear_compute, "use_cache"), use_cache_level);
    glUniform1f(glGetUniformLocation(lighting_clear_compute, "intensity"), intensity);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

        // directional
void GLContainer::compute_directional_lighting(float theta, float phi, float initial_ray_intensity, float decay_power)
{
    redraw_flag = true;
    glUseProgram(directional_lighting_compute);

    glUniform1f(glGetUniformLocation(directional_lighting_compute, "utheta"), theta);
    glUniform1f(glGetUniformLocation(directional_lighting_compute, "uphi"), phi);
    glUniform1f(glGetUniformLocation(directional_lighting_compute, "light_dim"), LIGHT_DIM);
    glUniform1f(glGetUniformLocation(directional_lighting_compute, "light_intensity"), initial_ray_intensity);
    glUniform1f(glGetUniformLocation(directional_lighting_compute, "decay_power"), decay_power);

    glUniform1i(glGetUniformLocation(directional_lighting_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(directional_lighting_compute, "lighting"), 6);

    glDispatchCompute( LIGHT_DIM/8, LIGHT_DIM/8, 1 ); //workgroup is 8x8x1, so divide x and y by 8

    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

void GLContainer::compute_new_directional_lighting(float theta, float phi, float initial_ray_intensity, float decay_power)
{
    redraw_flag = true;
    glUseProgram(new_directional_lighting_compute);

    glUniform1f(glGetUniformLocation(new_directional_lighting_compute, "utheta"), theta);
    glUniform1f(glGetUniformLocation(new_directional_lighting_compute, "uphi"), phi);
    glUniform1f(glGetUniformLocation(new_directional_lighting_compute, "light_dim"), LIGHT_DIM);
    glUniform1f(glGetUniformLocation(new_directional_lighting_compute, "light_intensity"), initial_ray_intensity);
    glUniform1f(glGetUniformLocation(new_directional_lighting_compute, "decay_power"), decay_power);

    glUniform1i(glGetUniformLocation(new_directional_lighting_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(new_directional_lighting_compute, "lighting"), 6);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 ); //workgroup is 8x8x8

    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

        // ambient occlusion
void GLContainer::compute_ambient_occlusion(int radius)
{
    redraw_flag = true;
    glUseProgram(ambient_occlusion_compute);

    glUniform1i(glGetUniformLocation(ambient_occlusion_compute, "radius"), radius);

    glUniform1i(glGetUniformLocation(ambient_occlusion_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(ambient_occlusion_compute, "lighting"), 6);

    glDispatchCompute(DIM/8, DIM/8, DIM/8);

    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}

        // fake GI
void GLContainer::compute_fake_GI(float factor, float sky_intensity, float thresh)
{
    redraw_flag = true;
    glUseProgram(fakeGI_compute);

    glUniform1i(glGetUniformLocation(fakeGI_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(fakeGI_compute, "lighting"), 6);

    glUniform1f(glGetUniformLocation(fakeGI_compute, "scale_factor"), factor);
    glUniform1f(glGetUniformLocation(fakeGI_compute, "alpha_thresh"), thresh);
    glUniform1f(glGetUniformLocation(fakeGI_compute, "sky_intensity"), sky_intensity);

    // This has a sequential dependence - from the same guy who did the Voxel Automata Terrain, Brent Werness:
    //   "Totally faked the GI!  It just casts out 9 rays in upwards facing the lattice directions.
    //    If it escapes it gets light from the sky, otherwise it gets some fraction of the light
    //    from whatever cell it hits.  Run from top to bottom and you are set!"

    // For that reason, I'm doing 2d workgroups, starting from the top, going to the bottom.

    for (int y = DIM-1; y >= 0; y--) //iterating through y, from top to bottom
    {
        // update y index
        glUniform1i(glGetUniformLocation(fakeGI_compute, "y_index"), y);

        // send the job, for one xz plane
        glDispatchCompute(DIM/8, 1, DIM/8);

        // wait for all those shader invocations to finish
        glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
    }
}

        // mash (combine light into color buffer)
void GLContainer::mash()
{
    redraw_flag = true;
    glUseProgram(mash_compute);

    glUniform1i(glGetUniformLocation(mash_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(mash_compute, "lighting"), 6);

    glDispatchCompute(DIM/8, DIM/8, DIM/8);

    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
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

// VAT and Load will need a shader, that can copy and respect the mask - save is more trivial, just read out the buffer and save it, same as last time
void GLContainer::copy_loadbuffer(bool respect_mask)
{
    redraw_flag = true;
    swap_blocks();
    glUseProgram(copy_loadbuff_compute);

    glUniform1i(glGetUniformLocation(copy_loadbuff_compute, "respect_mask"), respect_mask);

    glUniform1i(glGetUniformLocation(copy_loadbuff_compute, "current"), 2+tex_offset);
    glUniform1i(glGetUniformLocation(copy_loadbuff_compute, "current_mask"), 4+tex_offset);

    glUniform1i(glGetUniformLocation(copy_loadbuff_compute, "previous"), 3-tex_offset);
    glUniform1i(glGetUniformLocation(copy_loadbuff_compute, "previous_mask"), 5-tex_offset);

    glUniform1i(glGetUniformLocation(copy_loadbuff_compute, "loadbuff"), 10);

    glDispatchCompute( DIM/8, DIM/8, DIM/8 );
    glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
}


   // Brent Werness's Voxel Automata Terrain - set redraw_flag to true
std::string GLContainer::vat(float flip, std::string rule, int initmode, glm::vec4 color0, glm::vec4 color1, glm::vec4 color2, float lambda, float beta, float mag, bool respect_mask)
{
    redraw_flag = true;

    int dimension;

    // this is the easiest way to handle the dimension I think
    if(DIM == 32)
        dimension = 5;
    else if(DIM == 64)
        dimension = 6;
    else if(DIM == 128)
        dimension = 7;
    else if(DIM == 256)
        dimension = 8;
    else if(DIM == 512)
        dimension = 9;

    // check for equality with 'r' or 'i' to do random or isingRandom, else interpret as a shortrule
    // I want to add different init modes, to seed multiple faces instead of just the one
    voxel_automata_terrain v(dimension, flip, rule, initmode, lambda, beta, mag);

    // pull out the texture data
    std::vector<unsigned char> loaded_bytes; // used the same way as load(), below

    // triple for-loop to pull the data out
    for (int x = 0; x < DIM; x++)
    {
        for (int y = 0; y < DIM; y++)
        {
            for (int z = 0; z < DIM; z++)
            {
                // append data with the colors specified as input
                glm::vec4 color;
                switch (v.state[x][y][z])
                {
                    case 0: color = color0; break; // use color0
                    case 1: color = color1; break; // use color1
                    case 2: color = color2; break; // use color2

                    default: color = color0; break; // this shouldn't come up, but the compiler was mad
                }

                // put it in the vector as bytes
                loaded_bytes.push_back(static_cast<unsigned char>(color.x * 255));
                loaded_bytes.push_back(static_cast<unsigned char>(color.y * 255));
                loaded_bytes.push_back(static_cast<unsigned char>(color.z * 255));
                loaded_bytes.push_back(static_cast<unsigned char>(color.w * 255));

                // cout << v.state[x][y][z] << " ";
            }
            // cout << endl;
        }
        // cout << endl;
    }

    // send it
    glBindTexture(GL_TEXTURE_3D, textures[10]); // put it in the loadbuffer
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0,  GL_RGBA, GL_UNSIGNED_BYTE, &loaded_bytes[0]);

    copy_loadbuffer(respect_mask);

    // get the rule out of v
    return v.getShortRule();
}

   // load - set redraw_flag to true
void GLContainer::load(std::string filename, bool respect_mask)
{
    redraw_flag = true;

    std::vector<unsigned char> image_loaded_bytes;
    unsigned width, height;

    unsigned error = lodepng::decode(image_loaded_bytes, width, height, filename.c_str());

    //report any errors
    if(error) std::cout << "decode error during load(\" "+ filename +" \") " << error << ": " << lodepng_error_text(error) << std::endl;

    //put that shit in the front buffer with glTexImage3D()
    glBindTexture(GL_TEXTURE_3D, textures[10]); // put it in the loadbuffer
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, DIM, DIM, DIM, 0,  GL_RGBA, GL_UNSIGNED_BYTE, &image_loaded_bytes[0]);

    copy_loadbuffer(respect_mask);

    cout << "filename on load is: " << filename << std::endl << std::endl;
}

   // save
void GLContainer::save(std::string filename)
{
    // don't need to redraw
    std::vector<unsigned char> image_bytes_to_save;
    unsigned width, height;

    width = DIM;
    height = DIM*DIM;

    image_bytes_to_save.resize(4*DIM*DIM*DIM);
    filename = std::string("saves/") + filename;

    glGetTextureImage( textures[2+tex_offset], 0, GL_RGBA, GL_UNSIGNED_BYTE, 4*DIM*DIM*DIM, &image_bytes_to_save[0]);

    unsigned error = lodepng::encode(filename.c_str(), image_bytes_to_save, width, height);
    if(error) std::cout << "encode error during save(\" "+ filename +" \") " << error << ": " << lodepng_error_text(error) << std::endl;

    cout << "filename on save is: " << filename << std::endl << std::endl;
}


void GLContainer::delete_textures()
{
    // delete the textures
   glDeleteTextures(13, &textures[0]); 
}
