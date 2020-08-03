# Voraldo v1.1

This is the continuation of v1.0, which can be found here: https://github.com/0xBAMA/v1.0

This incorporates a lot of changes over [v1.0](https://jbaker.graphics/writings/voraldo.html) - a lot of small quality of life type fixes to make use of the editor more friendly, but I am also making it a point to move forwards on some different things I've been thinking about. 

Some of the bigger things:
 
 - completely redoing the menu - add top menu bar, togglable overlay with fps + fps graph, and a small widget that helps the user stay oriented in 3d space
 - switching to uimage3D buffers, for more consistent behavior
 - organizing buffers better
 - incorporating a copy/paste function
 - some SDF-based drawing functions (smooth min, fractals)
 - making the lighting buffer RGB, start looking at doing light with color associated with it
 - compass rose, to show block orientation (helps with positioning)
 - optimization idea: keep a bool that tells whether or not things need to be re-rendered via the raycast compute shader each frame, else just display existing texture
 - shape batching, like VIVS did - probably using SSBOs this time instead of uniform buffers
 - LUA scripting - using the same interface as the menu buttons do
 - change box blur to a gaussian kernel - weight color contribution by cell alpha
 - point lights
 - lighting cache/animation system
 - uniform environment presented to all shaders
 - addition of a load buffer, so that the VAT and Load operations can be made to respect the mask
