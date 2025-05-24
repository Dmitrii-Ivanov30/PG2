#version 460 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D tex0;

// Light settings (directional light)
uniform vec3 lightDir = normalize(vec3(-0.5, -1.0, -0.5));
uniform vec3 lightColor = vec3(1.0);
uniform float ambientStrength = 0.2;

void main() {
    // Texture sampling
    vec4 texColor = texture(tex0, TexCoord);
    
    // Basic lighting calculations
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse lighting
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, -lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Combine lighting with texture
    vec3 result = (ambient + diffuse) * texColor.rgb;
    
    FragColor = vec4(result, texColor.a);
    
    // Alpha discard
    if(FragColor.a < 0.1)
        discard;
}