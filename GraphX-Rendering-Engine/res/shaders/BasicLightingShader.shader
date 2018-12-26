#shader vertex
#version 330 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec4 vColor;

// Uniform matrices for the transformations
uniform mat4 u_Model;
uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat3 u_Normal;

// Varying variables
out vec3 v_Normal;
out vec3 v_WorldPosition;
out vec4 v_Color;

void main()
{
	v_WorldPosition = vec3(u_Model * vec4(vPosition, 1.0));
	gl_Position = u_Projection * u_View * vec4(v_WorldPosition, 1.0);
	v_Normal = u_Normal * vNormal;
	v_Color = vColor;
}

#shader fragment
#version 330 core

in vec3 v_Normal;
in vec3 v_WorldPosition;
in vec4 v_Color;

// Uniforms
uniform vec4 u_LightColor;
uniform vec3 u_LightPos;
uniform vec3 u_CameraPos;

uniform float u_AmbientStrength;
uniform float u_Shininess;
uniform float u_Reflectivity;

out vec4 fColor;

void main()
{
	// Normalize the light vector and the normal vector
	vec3 UnitNormal = normalize(v_Normal);

	// Ambient Light Color
	vec4 ambientColor = u_AmbientStrength * u_LightColor;

	// Diffuse Light Color
	vec3 UnitLightVec = normalize(u_LightPos - v_WorldPosition);
	float brightness = max(0.0f, dot(UnitNormal, UnitLightVec));
	vec4 diffuseColor = u_LightColor * brightness;

	// Specular Light Color
	vec3 ViewDir = normalize(u_CameraPos - v_WorldPosition);
	vec3 ReflectedLightDir = reflect(-UnitLightVec, UnitNormal);
	float shine = pow(max(dot(ReflectedLightDir, ViewDir), 0.0), u_Shininess);
	vec4 specularColor = shine * u_Reflectivity * u_LightColor;

	fColor = (ambientColor + diffuseColor + specularColor) * v_Color;
}