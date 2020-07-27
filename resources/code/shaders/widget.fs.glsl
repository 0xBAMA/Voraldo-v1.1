#version 430 core

in vec3 normal;
in vec3 color;
in vec3 vpos;

out vec4 fragment_output;

void main()
{
	vec3 light_position = vec3(-2,-1,-0.45);
	vec3 eye_position = vec3(0,0,-1);
	
	vec3 l = normalize(vpos - light_position);
	vec3 v = normalize(vpos - eye_position);
	vec3 n = normalize(normal);
	vec3 r = normalize(reflect(l, n));

	vec3 pixcol = color;
	
	float ambient = 0.08;
	pixcol += ambient*vec3(0.1,0.1,0.2);

	float diffuse = (1/(pow(0.25*distance(vpos,light_position),2))) * 0.3 * max(dot(n, l),0);

	pixcol += diffuse*vec3(0.39,0.39,0.18);

	float specular = (1/(pow(0.25*distance(vpos,light_position),2))) * 1.0 * pow(max(dot(r,v),0),10);

	if(dot(n,l) > 0)
		pixcol += specular*vec3(0.1,0.12,0);

	fragment_output = vec4(pixcol, 1);
}
