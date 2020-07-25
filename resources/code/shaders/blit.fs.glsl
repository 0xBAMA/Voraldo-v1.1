#version 430 core

in vec2 v_pos;

//uniform bool show_trails;
//layout( binding = 1, r32ui ) uniform uimage2D current;

out vec4 fragment_output;

void main()
{
//	if(show_trails)
	//{
		//uint s = imageLoad(current, ivec2(imageSize(current)*(0.5*(v_pos+vec2(1))))).r>>3;
	
		fragment_output = vec4(v_pos.x,v_pos.y,v_pos.x*v_pos.y,1);
//	}
	//else
	//{
		//discard;
	//}
}
