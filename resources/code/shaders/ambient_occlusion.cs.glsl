#version 430

//note that this only effects what the parameters to glDispatchCompute are - by using gl_GlobalInvocationID, you don't need to worry what any of those numbers are
layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;    //specifies the workgroup size

uniform layout(rgba8) image3D current;        //values of the block after the update
uniform layout(r8) image3D lighting;        //values held in the lighting buffer

uniform int radius; //how big a neighborhood should be considered for neighborhood occupancy?

void main()
{
    vec4 prev = imageLoad(lighting, ivec3(gl_GlobalInvocationID.xyz));    //existing color value (what is the color?)

    float alpha_sum = 0.0f;
    float total_weight = 0.0;
    float current_weight = 0.0;

    float max_dist = distance(vec3(0), vec3(radius)); // largest distance between the center cell and the other cells

    for(int x = -radius; x <= radius; x++)
        for(int y = -radius; y <= radius; y++)
            for(int z = -radius; z <= radius; z++)
            {
                current_weight = 1-(distance(vec3(0), vec3(x, y, z)));
                total_weight+=current_weight;
                alpha_sum += imageLoad(current, ivec3(gl_GlobalInvocationID.xyz)+ivec3(x,y,z)).a*current_weight;
            }

    //sum up alpha values across the cells being considered
    //compute a ratio of that alpha sum over the number of cells considered
    //scale the rgb components of pcol by this ratio

    //a high ratio of occuppancy means this cell should be darkened
    //therefore, we are multiplying the existing lighting value by one minus this ratio

    float new = prev.r * (1 - alpha_sum/total_weight);

    imageStore(lighting, ivec3(gl_GlobalInvocationID.xyz), vec4(new));
}

