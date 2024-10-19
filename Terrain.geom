#version 400

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 mvpMatrix;
uniform vec4 light;
uniform vec3 scale;
in vec4 color[3];
out vec4 oColor;
out vec3 textureWeights;
out vec2 textureCoords;

uniform float tick; 

float rippleEffect(float x, float z) {
    float frequency = 10.0;
    float amplitude = 0.5;
    float speed = 5.0;
    return sin(frequency * (x + z + speed * tick)) * amplitude;
}

vec4 calc(vec3 p1, vec3 p2, vec3 p3) {
    return normalize(vec4(cross(p2 - p1, p3 - p1), 0));
}

vec4 getWaterColor(float depth) {
    vec4 shallowColor = vec4(0.3, 0.6, 1.0, 1.0);  // Light blue
    vec4 deepColor = vec4(0.0, 0.0, 0.5, 1.0);     // Deep blue
    float factor = clamp(depth / 10.0, 0.0, 1.0);  
    return mix(shallowColor, deepColor, factor);
}

void main()
{
    float wScale = scale.x;
    float gScale = scale.y;
    float sScale = scale.z;
    float wLevel = 10 * wScale;
    float gLevel = 10 * gScale;
    float sLevel = 10 * sScale;

    vec4 outPosition[3];
    for (int i = 0; i < 3; i++) {
        outPosition[i] = gl_in[i].gl_Position;
        if (outPosition[i].y < wLevel) outPosition[i].y = wLevel;
    }

    vec4 n = calc(outPosition[0].xyz, outPosition[1].xyz, outPosition[2].xyz);
    vec4 d = dot(light, n) * vec4(1);
    oColor = d + vec4(0.3);

    for (int i = 0; i < 3; i++) {

        float textureScaleFactor = 30;
        textureCoords.s = (outPosition[i].x + 45) / textureScaleFactor;
        textureCoords.t = (outPosition[i].z + 100) / textureScaleFactor;

        float y = outPosition[i].y;
        if (y == wLevel){
            textureWeights = vec3(1, 0, 0);
        } else if (y < gLevel) {
            textureWeights = vec3(0, 1, 0);
        } else if (y < sLevel){
            float sTerm = (y - gLevel) / (sLevel - gLevel);
            textureWeights = vec3(0, 1 - sTerm, sTerm);
        } else {
            textureWeights = vec3(0, 0, 1);
        }

        gl_Position = mvpMatrix * outPosition[i];
        EmitVertex();
    }
    EndPrimitive();
}