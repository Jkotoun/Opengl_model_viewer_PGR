#version 460 core



in vec3 vColor;
in vec3 vNormal;
in vec3 vPosition;
in vec3 vLightPos;

uniform vec3 lightColor;


vec3 phong(
    vec4 diffuseColor,
	vec3 position,
	vec3 normal,
	vec3 lightPosition,
	vec3 lightColor){

  //diffuse
  vec3 norm = normalize(normal);
  vec3 lightDir = normalize(lightPosition-position);
  float diffuse = max(0.f,dot(norm,lightDir));

  //ambient
  float ambientStrength = 0.2;

//  float specularStrength = 0.5;
//  vec3 viewDir = normalize(-position); 
//  vec3 reflectDir = reflect(-lightDir, norm);  
//  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
//  vec3 specular = specularStrength * spec * lightColor; 
//    


  return (diffuse+ambientStrength)*lightColor*diffuseColor.rgb;
}


out vec4 fColor;

void main() {

	vec3 col = phong(vec4(vColor,1),vPosition,vNormal,vLightPos,lightColor);
	fColor = vec4(col, 1);
}