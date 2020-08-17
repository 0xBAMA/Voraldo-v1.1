#version 430

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in; //3d workgroup

uniform layout(rgba8) image3D current;
uniform layout(r8) image3D lighting;

uniform vec3 light_position;
uniform float light_intensity;

uniform float decay_power;
uniform float distance_power;

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

#define NUM_STEPS 5000
#define MIN_DISTANCE -10000.0
#define MAX_DISTANCE 10000.0

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


void traceray(vec3 dir)
{
    // the location you need to consider is specified by gl_GlobalInvocationID

    vec3 org = (vec3(2*gl_GlobalInvocationID.xyz) - vec3(imageSize(lighting).x)) / imageSize(lighting).x;
    vec3 lpos = (vec3(2*light_position) - vec3(imageSize(lighting).x)) / imageSize(lighting).x;

    hit(lpos, dir); // we now have tmin and tmax set


  // note that we need to consider which is smaller:
    //   the distance to the light (point light is inside the volume)
    float lightdist = distance(org, lpos);

    //   the distance to the ray-box intersection point (tmin) - (point light is outside the volume)
    float bounddist = distance(org, org+float(tmin)*dir);

    // this is done by comparing the distance from the location specified by the shader index
    // and each of those two points, to figure out which is smaller



    float prev_intensity = imageLoad(lighting, ivec3(gl_GlobalInvocationID.xyz)).r; // the lighting value that was in the cell, before this operation
    float current_intensity = light_intensity; // the strength of the light, before traversing any of the volume


    // note also that in the shader invocation that shares a location with the point light, there is a case that needs to be handled -
    //   that shader invocation will simply take the light intensity as specified by the uniform input, skipping any traversal of the space
    if(vec3(gl_GlobalInvocationID.xyz) != light_position)
    {
        if(bounddist < lightdist)  // boundary distance is less than the distance to the light
        {
            float current_t = float(tmin);

            ivec3 sample_location;
            float alpha_sample;

            // float step = float((tmax-tmin))/NUM_STEPS;
            float step = 0.003; // uniform step seems to fit better here

            for(int i = 0; i < NUM_STEPS; i++)
            {
                if(current_t < 0 && current_intensity > 0)
                {
                    // new sample location
                    sample_location = ivec3((vec3(imageSize(lighting))/2.0f)*(org+current_t*dir+vec3(1)));

                    if(sample_location == ivec3(gl_GlobalInvocationID.xyz))
                        break;

                    // new read for the alpha_sample
                    alpha_sample = imageLoad(current, sample_location).a;

                    // decrement intensity with the value of alpha_sample
                    current_intensity *= 1-pow(alpha_sample, decay_power);

                    // increment t with the step
                    current_t += step;

                }
                else
                {
                    break;
                }
            }
        }
        else   // light is inside the volume - t will be initialized with 0 (so we start at the position of the light)
        {
            float current_t = -lightdist;

            ivec3 sample_location;
            float alpha_sample;

            // float step = float((tmax-tmin))/NUM_STEPS;
            float step = 0.003; // uniform step seems to fit better here

            for(int i = 0; i < NUM_STEPS; i++)
            {
                if(current_t < 0 && current_intensity > 0)
                {
                    // new sample location
                    sample_location = ivec3((vec3(imageSize(lighting))/2.0f)*(org+current_t*dir+vec3(1)));

                    if(sample_location == ivec3(gl_GlobalInvocationID.xyz))
                        break;

                    // new read for the alpha_sample
                    alpha_sample = imageLoad(current, sample_location).a;

                    // decrement intensity with the value of alpha_sample
                    current_intensity *= 1-pow(alpha_sample, decay_power);

                    // increment t with the step
                    current_t += step;

                }
                else
                {
                    break;
                }
            }
        }
    }

    // this goes to 1 for distance_power = 0, otherwise models some approximation of the inverse square law
    current_intensity *= 1/(pow(lightdist, distance_power));

    // add the current_intensity to the previous intensity
    current_intensity += prev_intensity;
    imageStore(lighting, ivec3(gl_GlobalInvocationID.xyz), vec4(current_intensity));

}


void main()
{
    // compute direction - this is the normalized vector that goes from the light position to the point specified by the compute shader index
    vec3 dir = normalize(vec3(gl_GlobalInvocationID.xyz) - light_position); // going to specify this in the image coordinate space, for simplicity - normalized vector will be fine to use in traceray
    // this will be invalid for the invocation whose location is the same as the light's location - we'll resolve that with a check in traceray

    // this is all the information traceray needs, the direction from which the light is coming
    traceray(dir);
}

