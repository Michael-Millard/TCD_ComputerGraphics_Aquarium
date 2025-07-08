#version 330 core

in vec3 fragPos;    // Fragment position in world space
in vec3 normal;     // Normal vector from vertex shader
in vec2 texCoords;  // Texture coordinates from vertex shader

out vec4 fragColor; // Final fragment color

uniform sampler2D textureDiffuse1; // Diffuse texture
uniform sampler2D textureDiffuse2; // Diffuse texture
uniform sampler2D textureDiffuse3; // Diffuse texture
uniform sampler2D textureDiffuse4; // Diffuse texture

// PointLight structure
 struct PointLight
 {
    vec3 position; // Position of the light
    vec3 ambient; // Ambient color intensity
    vec3 diffuse; // Diffuse color intensity
    vec3 specular; // Specular color intensity
    float constant; // Attenuation constant term
    float linear; // Attenuation linear term
    float quadratic;// Attenuation quadratic term
 };
 
// Point lights
uniform PointLight pointLights[5];

uniform float specularExponent;	// Specular exponent
uniform vec3 viewPosition;     // Camera position
uniform bool useTexture;       // Determines if texture should be used
uniform vec4 glassColor;       // RGBA color for glass

void main()
{
    // Init to vec3s of 0
    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    // Loop through point lights
    for (int i = 0; i < 5; i++) 
    {
        // Ambient lighting
        ambient += pointLights[i].ambient;

        // Diffuse lighting
        vec3 norm = normalize(normal);
        vec3 lightDirection = normalize(pointLights[i].position - fragPos);
        float diff = max(dot(norm, lightDirection), 0.0);
        diffuse += pointLights[i].diffuse * diff;

        // Specular lighting
        vec3 viewDirection = normalize(viewPosition - fragPos);
        vec3 reflectDirection = reflect(-lightDirection, norm);
        float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), specularExponent);
        specular += pointLights[i].specular * spec;

        // Attenuation
        float distance = length(pointLights[i].position - fragPos);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance + pointLights[i].quadratic * (distance * distance));

        // Apply attenuation to the light contributions
        ambient *= attenuation;
        diffuse *= attenuation;
        specular *= attenuation;
    }

    vec3 lighting = ambient + diffuse + specular;

    // For textures
    if (useTexture) 
    {
        vec3 texColor = texture(textureDiffuse1, texCoords).rgb;
        fragColor = vec4(lighting * texColor, 1.0);
    } 
    // For glass
    else 
    {
        fragColor = vec4(lighting * glassColor.rgb, glassColor.a);
    }
}
