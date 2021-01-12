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

/* #define TRIPLE_MONITOR // enable to span all three monitors */

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

// this sets how many texels are on an edge. Trying not to hardcode this anywhere, so that I can easily switch from 256, 512, 1024, etc
#define DIM 512
// #define DIM 256

// this is effectively the shadowmap dimension - probaby want to reduce this eventually
#define LIGHT_DIM 2048


// function to get color temperature
// ideal range for input values is 1500K - 15000K
inline glm::vec3 get_color_for_temp(int t)
{
    double temp = t / 100.;
    double red, green, blue;

    // calc red
    if(temp <= 66.)
        red = 255.;
    else
    {
        red = temp - 60.;
        red = 329.698727446 * (std::pow(red,  -0.1332047592));
    }

    
    // calc green
    if(temp <= 66.)
    {
        green = temp;
        green = 99.4708025861 * std::log(green) - 161.1195681661;
    }
    else
    {
        green = temp - 60.;
        green = 288.1221695283 * std::pow(green, -0.0755148492); 
    }

    // calc blue
    if(temp >= 66.)
        blue = 255.;
    else
    {
        if(temp <= 19.)
            blue = 0;
        else
        {
            blue = temp = 10;
            blue = 138.5177312231 * std::log(blue) - 305.0447927307; 
        }
    }
    
    // normalize and return
    std::clamp(  red, 0., 255.);
    std::clamp(green, 0., 255.);
    std::clamp( blue, 0., 255.);

    red   /= 255.;
    green /= 255.;
    blue  /= 255.;

    return glm::vec3(red, green, blue);
}

#endif
