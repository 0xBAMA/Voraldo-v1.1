#version 430

in vec3 vPosition;

out vec2 v_pos;

void main()
{
  v_pos = vPosition.xy;
  
  gl_Position = vec4(vPosition,1.0);
}
