#version 460 core

in vec3 vNormal;
in vec3 vPosition;
in vec3 vLightPos;
in vec4 vColor;
in float vUseDiffuseTexture;
in vec2 TexCoords;

out vec4 fColor;

uniform vec3 lightColor;
uniform sampler2D texture_diffuse1;

vec3 phong(vec4 diffuseColor, vec3 position, vec3 normal, vec3 lightPosition, vec3 lightColor){

  //diffuse
  vec3 norm = normalize(normal);
  vec3 lightDir = normalize(lightPosition-position);
  float diffuse = max(0.f,dot(norm,lightDir));

  //ambient
  float ambientStrength = 0.2;

  //specular
  float specularStrength = 2;
  vec3 viewDir = normalize(-position); 
  vec3 reflectDir = reflect(-lightDir, norm);  
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

  return (diffuse+ambientStrength + spec*specularStrength)*lightColor*diffuseColor.rgb;
}


void main() {
	vec4 tmpFragColr;
	if(vUseDiffuseTexture == 1){
		tmpFragColr = texture(texture_diffuse1, TexCoords);
	}
	else{
		tmpFragColr = vColor;
	}

	vec3 col = phong(tmpFragColr,vPosition,vNormal,vLightPos,lightColor);
	fColor = vec4(col, tmpFragColr.a);
}