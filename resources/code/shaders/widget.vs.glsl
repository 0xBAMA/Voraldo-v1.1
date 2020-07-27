#version 430

in vec3 vPosition;
in vec3 vNormal;
in vec3 vColor;

out vec3 normal;
out vec3 color;

uniform float time;

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
  normal = vNormal;
  color = vColor;
  
  gl_Position = vec4(rotationMatrix(vec3(0,1,0), time) * rotationMatrix(vec3(1,0,0), time*0.2) * vPosition, 1.0);
}
