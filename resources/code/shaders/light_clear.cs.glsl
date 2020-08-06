#version 430

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in; //workgroup dimensions

uniform layout(rgba8) image3D current;
uniform layout(r8) image3D lighting;
uniform layout(r8) image3D lighting_cache;


uniform float intensity;
uniform bool use_cache;

void main()
{
	if(use_cache)
		imageStore(lighting, ivec3(gl_GlobalInvocationID.xyz), imageLoad(lighting_cache, ivec3(gl_GlobalInvocationID.xyz)));
	else
   	imageStore(lighting, ivec3(gl_GlobalInvocationID.xyz), vec4(intensity));
}

