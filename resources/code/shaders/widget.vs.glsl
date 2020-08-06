#version 430

in vec3 vPosition;
in vec3 vNormal;
in vec3 vColor;

out vec3 normal;
out vec3 color;
out vec3 vpos;

uniform float theta;
uniform float phi;
uniform float ratio;

uniform vec3 offset;

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

	mat3 rotphi = rotationMatrix(vec3(1,0,0), -1.0*phi);
	
	mat3 rottheta = rotationMatrix(vec3(0,1,0), -1.0*theta);

	normal = rotphi * rottheta * vNormal;
	color = vColor;
	vpos = 0.2 * (rotphi * rottheta * vPosition);
	
	vpos.x /= ratio;
	vpos += offset;

	gl_Position = vec4(vpos, 1.0);
}
