#ifndef INCLUDES
#define INCLUDES

#include <stdio.h>//stl includes
#include <vector>
#include <cmath>
#include <numeric>
#include <random>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <deque>
#include <chrono>
#include <ctime>
#include <cstdint>
#include <cstdlib>
#include <filesystem>

//iostream aliases
using std::cin;
using std::cout;
using std::cerr;

using std::flush;
using std::endl;

//vector math library GLM
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW
#include "../glm/glm.hpp" //general vector types
#include "../glm/gtc/matrix_transform.hpp" // for glm::ortho
#include "../glm/gtc/type_ptr.hpp" //to send matricies gpu-side
#include "../glm/gtx/transform.hpp"

//not sure as to the utility of this
#define GL_GLEXT_PROTOTYPES


//GUI library (dear ImGUI)
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdl.h"
#include "../imgui/imgui_impl_opengl3.h"


// Initialize glew loader with glewInit()
#include <GL/glew.h>

//SDL includes - windowing, gl context, system info
#include <SDL2/SDL.h>
//allows you to run OpenGL inside of SDL2
#include <SDL2/SDL_opengl.h>


//png loading library - very powerful
#include "lodepng.h"

//shader compilation wrapper - may need to be extended
#include "shader.h"

//up to 3d perlin noise generation
#include "perlin.h"

//diamond square heightmap generation
#include "diamond_square.h"

// voxel automata terrain
#include "vat.h"

// contains the OpenGL wrapper class
#include "gpu_data.h"


// pi definition
constexpr double pi = 3.14159265358979323846;

#define TRIPLE_MONITOR // enable to span all three monitors

// supersampling factor for main display shader
// #define SSFACTOR 5.0   // tanks performance
#define SSFACTOR 2.8  // this is for 8x multisampling
// #define SSFACTOR 2.0  // this is for 4x multisampling
// #define SSFACTOR 1.65
// #define SSFACTOR 1.25  // small amount of multisampling
// #define SSFACTOR 1.0  // no multisampling
// #define SSFACTOR 0.4 // this is <1x multisampling

// for the tile based rendering - needs to be a multiple of 32
#define TILESIZE 64

#define NUM_ROTATION_STEPS 1000

// this sets how many texels are on an edge. Trying not to hardcode this anywhere, so that I can easily switch from 256, 512, 1024, etc
#define DIM 512
// #define DIM 256




// Function to get color temperature from shadertoy user BeRo
// from the author:
//   Color temperature (sRGB) stuff
//   Copyright (C) 2014 by Benjamin 'BeRo' Rosseaux
//   Because the german law knows no public domain in the usual sense,
//   this code is licensed under the CC0 license 
//   http://creativecommons.org/publicdomain/zero/1.0/ 
// 
// Valid from 1000 to 40000 K (and additionally 0 for pure full white)
inline glm::vec3 get_color_for_temp(double temperature)
{
    // Values from: http://blenderartists.org/forum/showthread.php?270332-OSL-Goodness&p=2268693&viewfull=1#post2268693   
    glm::mat3 m = (temperature <= 6500.0) ?
        glm::mat3(glm::vec3(0.0, -2902.1955373783176, -8257.7997278925690),
                  glm::vec3(0.0, 1669.5803561666639, 2575.2827530017594),
                  glm::vec3(1.0, 1.3302673723350029, 1.8993753891711275)) : 
        glm::mat3(glm::vec3(1745.0425298314172, 1216.6168361476490, -8257.7997278925690),
                  glm::vec3(-2666.3474220535695, -2173.1012343082230, 2575.2827530017594),
                  glm::vec3(0.55995389139931482, 0.70381203140554553, 1.8993753891711275)); 

    return glm::mix(glm::clamp(glm::vec3(m[0] / (glm::vec3(glm::clamp(temperature, 1000.0, 40000.0)) + m[1]) + m[2]), glm::vec3(0.0), glm::vec3(1.0)), glm::vec3(1.0), glm::smoothstep(1000.0, 0.0, temperature));
}
#endif
