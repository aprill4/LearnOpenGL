#version 330 core
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

out vec4 FragColor;

struct Material {
  sampler2D diffuse;
  sampler2D specular;
  float shinness;
};

struct DirLight {
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct PointLight {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};

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

uniform Material material;

uniform DirLight dirLight;
#define NR_POINT_LIGHT 4
uniform PointLight pointLight[NR_POINT_LIGHT];
uniform SpotLight spotLight;

uniform vec3 viewPos;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 fragPos);

void main() {
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 normal = normalize(Normal);

  vec3 result = vec3(0.0f);
  result += CalcDirLight(dirLight, normal, viewDir);
  
  for (int i = 0; i < NR_POINT_LIGHT; ++i) {
    result += CalcPointLight(pointLight[i], normal, viewDir, FragPos);
  }

  result += CalcSpotLight(spotLight, normal, viewDir, FragPos);

  FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
  vec3 sampleDiffuse = vec3(texture(material.diffuse, TexCoord));

  vec3 ambient = sampleDiffuse * light.ambient;

  vec3 lightDir = normalize(-light.direction);
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = sampleDiffuse * diff * light.diffuse;

  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shinness);
  vec3 specular = vec3(texture(material.specular, TexCoord)) * spec * light.specular;

  return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos) {
  vec3 sampleDiffuse = vec3(texture(material.diffuse, TexCoord));

  vec3 ambient = sampleDiffuse * light.ambient;

  vec3 lightDir = normalize(light.position - fragPos);
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = sampleDiffuse * diff * light.diffuse;

  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shinness);
  vec3 specular = texture(material.specular, TexCoord).rgb * spec * light.specular;

  float distance = length(light.position - fragPos);
  float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;

  return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 fragPos) {
  vec3 lightDir = normalize(light.position - fragPos);
  float theta = dot(lightDir, normalize(-light.direction));

  vec3 sampleDiffuse = texture(material.diffuse, TexCoord).rgb;
  vec3 ambient = sampleDiffuse * light.ambient;

  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = sampleDiffuse * diff * light.diffuse;

  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shinness);
  vec3 specular = texture(material.specular, TexCoord).rgb * spec * light.specular;

  float distance = length(light.position - fragPos);
  float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  float epsilon = light.cutOff - light.outerCutOff;
  float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

  ambient *= attenuation;
  diffuse *= attenuation * intensity;
  specular *= attenuation * intensity;

  return (ambient + diffuse + specular);
}
