#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

struct SpotLight {
  vec3 position;
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;

  float cutOff;
  float outerCutOff;
};

uniform SpotLight spotLight;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform float shinness;

uniform vec3 viewPos;

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 fragPos);

void main()
{    
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 normal = normalize(Normal);

    vec3 result = CalcSpotLight(spotLight, normal, viewDir, FragPos);
    FragColor = vec4(result, 1.0f);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 fragPos) {
  vec3 lightDir = normalize(light.position - fragPos);
  float theta = dot(lightDir, normalize(-light.direction));

  vec3 sampleDiffuse = texture(texture_diffuse1, TexCoords).rgb;
  vec3 ambient = sampleDiffuse * light.ambient;

  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = sampleDiffuse * diff * light.diffuse;

  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), shinness);
  vec3 specular = texture(texture_specular1, TexCoords).rgb * spec * light.specular;

  float distance = length(light.position - fragPos);
  float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  float epsilon = light.cutOff - light.outerCutOff;
  float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

  ambient *= attenuation;
  diffuse *= attenuation * intensity;
  specular *= attenuation * intensity;

  return (ambient + diffuse + specular);
}
