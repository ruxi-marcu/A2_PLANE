#version 330 core

in vec2 UV;
in vec3 Position_worldspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

in vec3 LightDirection_tangentspace;
in vec3 EyeDirection_tangentspace;

out vec3 color;

uniform sampler2D DiffuseTex;
uniform sampler2D NormalTex;
uniform sampler2D SpecularTex;
uniform mat4 view;
uniform mat4 model;
uniform mat3 MV3x3;
uniform vec3 LightPosW;

void main(){

	vec3 LightColor = vec3(1,1,1);
	float LightPower = 40.0;
	
	vec3 MaterialDiffuseColor = texture( DiffuseTex, UV ).rgb;
	vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = texture( SpecularTex, UV ).rgb * 0.3;

	
	vec3 TextureNormal_tangentspace = normalize(texture( NormalTex, vec2(UV.x,-UV.y) ).rgb*2.0 - 1.0);
	
	float distance = length( LightPosW - Position_worldspace );

	vec3 n = TextureNormal_tangentspace;
	vec3 l = normalize(LightDirection_tangentspace);
	float cosTheta = clamp( dot( n,l ), 0,1 );

	vec3 E = normalize(EyeDirection_tangentspace);
	vec3 R = reflect(-l,n);
	float cosAlpha = clamp( dot( E,R ), 0,1 );
	
	//color = 
		//MaterialAmbientColor +
		///MaterialDiffuseColor * LightColor * LightPower * 0.5 / (distance*distance) +
		///MaterialSpecularColor * LightColor * LightPower * 0.5 / (distance*distance);
		//MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
		//MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);
	color =vec3(1.0,1.0,1.0);
}