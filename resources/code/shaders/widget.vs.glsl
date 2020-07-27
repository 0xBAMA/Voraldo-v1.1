#version 430

in vec3 vPosition;
in vec3 vNormal;
in vec3 vColor;

out vec3 normal;
out vec3 color;
out vec3 vpos;

uniform float time;
uniform float ratio;

//thanks to Neil Mendoza via http://www.neilmendoza.com/glsl-rotation-about-an-arbitrary-axis/
mat3 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}


void main()
{

	mat3 rot = rotationMatrix(vec3(0,1,0), time) * rotationMatrix(vec3(1,0,0), time*0.2);

	normal = rot * vNormal;
	color = vColor;
	vpos = 0.2*(rot*vPosition);
	
	vpos.x /= ratio;
	vpos += vec3(0.88, -0.68, 0.0);

	gl_Position = vec4(vpos, 1.0);
}
