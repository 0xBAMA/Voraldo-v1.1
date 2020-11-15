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
#define MAX_DISTANCE 5.0


// the display texture
uniform layout(rgba16) image2D current; // we can get the dimensions with imageSize
uniform layout(rgba8) image3D block;
uniform layout(r8) image3D lighting;

// because this is going to have to be tile-based, we need this local offset
uniform int x_offset;
uniform int y_offset;

uniform int clickndragx;
uniform int clickndragy;

//gl_GlobalInvocationID will define the tile size, so doing anything to define it here would be redundant
// this shader is general up to tile sizes of 2048x2048, since those are the maximum dispatch values

uniform float theta;
uniform float phi;

uniform vec4 clear_color;

uniform float scale;

uniform float upow;


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

vec4 get_color_for_pixel(vec3 org, vec3 dir)
{
  float current_t = float(tmax);
  //vec4 t_color = vec4(1, 1, 1, 0);

  vec4 t_color = clear_color;

  float step = float((tmax-tmin))/NUM_STEPS;
  if(step < 0.001f)
    step = 0.001f;
    
  vec3 block_size = vec3(imageSize(block));

  ivec3 samp = ivec3((block_size/2.0f)*(org+current_t*dir+vec3(1)));

  vec4 new_read = imageLoad(block,samp);
  vec4 new_light_read = imageLoad(lighting,samp);

  float alpha_squared;

  for(int i = 0; i < NUM_STEPS; i++)
  {
    if(current_t>=tmin)
    {
      //apply the lighting scaling
      new_read.rgb *= (4*new_light_read.r);

		alpha_squared = pow(new_read.a, upow); // parameterizing the alpha power

      // it's a over b, where a is the new sample and b is the current color, t_color
      t_color.rgb = new_read.rgb * alpha_squared + t_color.rgb * t_color.a * ( 1 - alpha_squared );
      t_color.a = alpha_squared + t_color.a * ( 1 - alpha_squared );

      current_t -= step;
      samp = ivec3((block_size/2.0f)*(org+current_t*dir+vec3(1)));

      new_read = imageLoad(block,samp);
      new_light_read = imageLoad(lighting,samp);
    }
  }
  return t_color;
}

void main()
{
	ivec2 Global_Loc = ivec2(gl_GlobalInvocationID.xy) + ivec2(x_offset+clickndragx, y_offset+clickndragy);
	ivec2 dimensions = ivec2(imageSize(current));
	
	float aspect_ratio = float(dimensions.y) / float(dimensions.x);
	
	float x_start = scale*((Global_Loc.x/float(dimensions.x)) - 0.5);
	float y_start = scale*((Global_Loc.y/float(dimensions.y)) - 0.5)*(aspect_ratio);
	
	//start with a vector pointing down the z axis (greater than half the corner to corner distance, i.e. > ~1.75)
	vec3 org = vec3(x_start, y_start,  2); //add the offsets in x and y
	vec3 dir = vec3(      0,       0, -2); //simply a vector pointing in the opposite direction, no xy offsets

	//rotate both vectors 'up' by phi, e.g. about the x axis
	mat3 rotphi = rotationMatrix(vec3(1,0,0), phi);
	org *= rotphi;
	dir *= rotphi;

	//rotate both about the y axis by theta
	mat3 rottheta = rotationMatrix(vec3(0,1,0), theta);
	org *= rottheta;
	dir *= rottheta;

  Global_Loc -= ivec2(clickndragx, clickndragy);
	if(Global_Loc.x < dimensions.x && Global_Loc.y < dimensions.y)
	{  // we are good to check the ray against the AABB
		if(hit(org,dir))
		{
			imageStore(current, Global_Loc, get_color_for_pixel(org, dir));
		}
		else
		{
			imageStore(current, Global_Loc, clear_color);
			// imageStore(current, Global_Loc, vec4(0));
		}
	}  // else, this part of the tile falls outside of the image bounds, no operation should take place
}
