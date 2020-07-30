#version 430 core

in vec3 normal;
in vec3 color;
in vec3 vpos;

out vec4 fragment_output;

void main()
{
	vec3 light1_position = vec3(-1.4, -0.8, -0.15);
	vec3 light2_position = vec3( 1.8, -0.3,  0.25);
	vec3 eye_position = vec3(0,0,-1);
	
	vec3 l1 = normalize(vpos - light1_position);	
	vec3 l2 = normalize(vpos - light2_position);
	vec3 v = normalize(vpos - eye_position);
	vec3 n = normalize(normal);
	vec3 r1 = normalize(reflect(l1, n));
	vec3 r2 = normalize(reflect(l2, n));

	vec3 pixcol = color;
	
	float ambient = -0.2;
	pixcol += ambient*vec3(0.1,0.1,0.2);

	float diffuse1 = (1/(pow(0.25*distance(vpos,light1_position),2))) * 0.18 * max(dot(n, l1),0);
	float diffuse2 = (1/(pow(0.25*distance(vpos,light2_position),2))) * 0.18 * max(dot(n, l2),0);

	pixcol += diffuse1*vec3(0.09,0.09,0.04);
	pixcol += diffuse2*vec3(0.09,0.09,0.04);

	float specular1 = (1/(pow(0.25*distance(vpos,light1_position),2))) * 0.4 * pow(max(dot(r1,v),0),60);
	float specular2 = (1/(pow(0.25*distance(vpos,light2_position),2))) * 0.4 * pow(max(dot(r2,v),0),80);

	if(dot(n,l1) > 0)
		pixcol += specular1*vec3(0.04,0.02,0);
		
	if(dot(n,l2) > 0)
		pixcol += specular2*vec3(0.04,0.02,0);

	fragment_output = vec4(pixcol, 1);
}
