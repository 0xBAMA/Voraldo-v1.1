#version 430 core

in vec2 v_pos;

// need to set up sampler for main display texture

// need to use this to get sampler
uniform float x_res;	
uniform float y_res;

out vec4 fragment_output;

void main()
{
	fragment_output = vec4(v_pos.x,v_pos.y,v_pos.x*v_pos.y,1);
}
