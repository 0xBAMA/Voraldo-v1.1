#version 430 core

in vec2 v_pos;

// need to set up sampler for main display texture
uniform sampler2DRect main_display_texture;

// need to use this to get sampler
uniform float x_res;	
uniform float y_res;

out vec4 fragment_output;

void main()
{
	// if(int(gl_FragCoord.y) % 2 == 0 && int(gl_FragCoord.x) % 3 == 0)
	// 	fragment_output = vec4(v_pos.x,v_pos.y,v_pos.x*v_pos.y,1);
	// else
	// 	discard;
	
	fragment_output = texture(main_display_texture, gl_FragCoord.xy);
}
