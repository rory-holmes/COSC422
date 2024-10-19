#version 330

in vec4 oColor;
in vec3 textureWeights;
in vec2 textureCoords;

uniform sampler2D grass;
uniform sampler2D water;
uniform sampler2D snow;

void main()
{
    vec4 gColour = texture(grass, textureCoords);
    vec4 wColour = texture(water, textureCoords);
    vec4 sColour = texture(snow, textureCoords);

    gl_FragColor = oColor *
                  (gColour * textureWeights.y
                 + wColour * textureWeights.x
                 + sColour  * textureWeights.z);
}