#version 430

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;    //specifies the workgroup size

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

double tmin, tmax; //global scope, set in hit() to tell min and max parameters

// #define NUM_STEPS 2000
//#define NUM_STEPS 165

// #define NUM_STEPS 500
#define NUM_STEPS 780
#define MIN_DISTANCE 0.0
#define MAX_DISTANCE 10.0

bool hit(vec3 org, vec3 dir)
{
  // hit() code adapted from:
  //
  //    Amy Williams, Steve Barrus, R. Keith Morley, and Peter Shirley
  //    "An Efficient and Robust Ray-Box Intersection Algorithm"
  //    Journal of graphics tools, 10(1):49-54, 2005

  //the bounding box
  vec3 min = vec3(-1,-1,-1);
  vec3 max = vec3(1,1,1);

  int sign[3];

  vec3 inv_direction = vec3(1/dir.x, 1/dir.y, 1/dir.z);

  sign[0] = (inv_direction[0] < 0)?1:0;
  sign[1] = (inv_direction[1] < 0)?1:0;
  sign[2] = (inv_direction[2] < 0)?1:0;

  vec3 bbox[2] = {min,max};

  tmin = (bbox[sign[0]][0] - org[0]) * inv_direction[0];
  tmax = (bbox[1-sign[0]][0] - org[0]) * inv_direction[0];

  double tymin = (bbox[sign[1]][1] - org[1]) * inv_direction[1];
  double tymax = (bbox[1-sign[1]][1] - org[1]) * inv_direction[1];

  if ( (tmin > tymax) || (tymin > tmax) )
    return false;
  if (tymin > tmin)
    tmin = tymin;
  if (tymax < tmax)
    tmax = tymax;

  double tzmin = (bbox[sign[2]][2] - org[2]) * inv_direction[2];
  double tzmax = (bbox[1-sign[2]][2] - org[2]) * inv_direction[2];

  if ( (tmin > tzmax) || (tzmin > tmax) )
    return false;
  if (tzmin > tmin)
    tmin = tzmin;
  if (tzmax < tmax)
    tmax = tzmax;
  return ( (tmin < MAX_DISTANCE) && (tmax > MIN_DISTANCE) );

  return true;
}


// the display texture
uniform layout(rgba16) image2D current; // we can get the dimensions with imageSize

// because this is going to have to be tile-based, we need this local offset
uniform int x_offset;
uniform int y_offset;

//gl_GlobalInvocationID will define the tile size, so doing anything to define it here would be redundant
// this shader is general up to tile sizes of 2048x2048, since those are the maximum dispatch values

void main()
{
	ivec2 Global_Loc = ivec2(gl_GlobalInvocationID.xy) + ivec2(x_offset, y_offset);
	ivec2 dimensions = ivec2(imageSize(current));
	
	if(Global_Loc.x < dimensions.x && Global_Loc.y < dimensions.y)
	{
		// we are good to check if you hit, and raycast if you do hit
	}
	else
	{  
		// this part of the tile falls outside of the image bounds, no operation should take place
	}	
	
	imageStore(current, Global_Loc, vec4(1.0/float(pow(gl_GlobalInvocationID.x, 0.3)), 1.0/float(pow(gl_GlobalInvocationID.y, 0.3)), 0.0, 1.0));

}
