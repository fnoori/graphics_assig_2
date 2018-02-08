// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

#define PI 3.14159265358979

// interpolated colour received from vertex stage
in vec3 Colour;
in vec2 TextureCoords;

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

uniform sampler2D textureImage_one;

uniform vec3 luminanceValues;
uniform float adjustBrightness;
uniform float doSobel;
uniform float doUnSharp;
uniform float horSobel;
uniform float doGauss;
uniform float gaussVal;

uniform float imageHeight;
uniform float imageWidth;

float step_w = 1.0/imageWidth;
float step_h = 1.0/imageHeight;

vec2 regOffset[9];
float regKernel[9];

void setUpOffset()
{
    regOffset[0] = vec2(-step_w, -step_h);
    regOffset[1] = vec2(0.0, -step_h);
    regOffset[2] = vec2(step_w, -step_h);
    
    regOffset[3] = vec2(-step_w, 0.0);
    regOffset[4] = vec2(0.0, 0.0);
    regOffset[5] = vec2(step_w, 0.0);
    
    regOffset[6] = vec2(-step_w, step_h);
    regOffset[7] = vec2(0.0, step_h);
    regOffset[8] = vec2(step_w, step_h);
}

vec4 unSharpen()
{
    vec4 unsharpened = vec4(0.0);
    
    // kernel for unsharpening
    regKernel[0] = 0.0;       regKernel[1] = -1.0;       regKernel[2] = 0.0;
    regKernel[3] = -1.0;      regKernel[4] = 5.0;        regKernel[5] = -1.0;
    regKernel[6] = 0.0;       regKernel[7] = -1.0;       regKernel[8] = 0.0;
    
    for( int i = 0; i < 9; i++ )
    {
        vec4 tmp = texture(textureImage_one, TextureCoords.st + regOffset[i]);
        unsharpened += tmp * regKernel[i];
    }
    
    return unsharpened;
}

vec4 sobel()
{
    vec4 sobelRes = vec4(0.0);
    
    if (horSobel > 0) {
        
        // kernel for vertical sobel
        regKernel[0] = -1.0;    regKernel[1] = -2.0;    regKernel[2] = -1.0;
        regKernel[3] = 0.0;     regKernel[4] = 0.0;     regKernel[5] = 0.0;
        regKernel[6] = 1.0;     regKernel[7] = 2.0;     regKernel[8] = 1.0;
        
        for( int i = 0; i < 9; i++ )
        {
            vec4 tmp = texture(textureImage_one, TextureCoords.st + regOffset[i]);
            sobelRes += tmp * regKernel[i];
        }
        
    } else {
        
        // kernel horizontal sobel
        regKernel[0] = 1.0;       regKernel[1] = 0.0;       regKernel[2] = -1.0;
        regKernel[3] = 2.0;       regKernel[4] = 0.0;       regKernel[5] = -2.0;
        regKernel[6] = 1.0;       regKernel[7] = 0.0;       regKernel[8] = -1.0;
        
        for( int i = 0; i < 9; i++ )
        {
            vec4 tmp = texture(textureImage_one, TextureCoords.st + regOffset[i]);
            sobelRes += tmp * regKernel[i];
        }
    }
    
    return sobelRes;
}

vec4 brightness(vec4 newColour)
{
    float inRed = newColour.r;
    float inGreen = newColour.g;
    float inBlue = newColour.b;
    
    newColour.r = (inRed * 0.1f) + (inGreen * 0.1f) + (inBlue * 0.1f);
    newColour.g = (inRed * 0.1f) + (inGreen * 0.1f) + (inBlue * 0.1f);
    newColour.b = (inRed * 0.1f) + (inGreen * 0.1f) + (inBlue * 0.1f);
    
    return newColour;
}

vec4 luminance(vec4 newColour)
{
    float luminanceIs = newColour.r * luminanceValues.r + newColour.g * luminanceValues.g + newColour.b * luminanceValues.b;
    
    newColour.r = luminanceIs;
    newColour.g = luminanceIs;
    newColour.b = luminanceIs;
    
    return newColour;
}

vec4 gauss()
{
    vec2 size = vec2(textureSize(textureImage_one, 0));
    int r = int(ceil(gaussVal - 0.5));
    
    vec3 gaussRes = vec3(0, 0, 0);
    float k = 0.9342 / (r * r);
    
    for (int y = -r; y <= r; y++) {
        
        for (int x = -r; x <= r; x++) {
            
            float d = length(vec2(x, y));
            if (d >= r) {
                continue;
            }
            float weight = k * (cos(PI * d/r) + 1) / 2;
            gaussRes += textureLod(textureImage_one, TextureCoords.xy + vec2(x,y)/size, 0.0).rgb * weight;
        }
    }
    return vec4(gaussRes, 1.0);
}

void main(void)
{
    vec4 newColour = texture(textureImage_one, TextureCoords);
    
    setUpOffset();
    
    if (luminanceValues.r != 1) {
        newColour = luminance(newColour);
    } else if (adjustBrightness > 0) {
        newColour = brightness(newColour);
    } else if (doSobel > 0) {
        newColour = sobel();
    } else if (doUnSharp > 0) {
        newColour = unSharpen();
    } else if (doGauss > 0) {
        newColour = gauss();
    }
    
    FragmentColour = newColour;
}
