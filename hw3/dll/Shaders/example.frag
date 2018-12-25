#version 450

layout(binding = 0) uniform sampler2D TextureMap;
layout(binding = 1) uniform sampler2D NormalMap;
layout(binding = 2) uniform sampler2D SpecularMap;

struct PhongShading
{
	vec4 amb_color;
	vec4 dif_color;
	vec4 spc_color;
	float amb_coe;
	float dif_coe;
	float spc_coe;
};

struct TextureEnable
{
	bool is_textureMap;
	bool is_normalMap;
	bool is_specularMap;
};

in vec2 frag_texcoord;
in vec3 frag_n;
in vec3 frag_pos;
in vec3 frag_lightPos;
in vec3 frag_camPos;
in mat3 frag_TBN;

out vec4 outColor;

uniform PhongShading phong_shading;
uniform TextureEnable tex_enb;

void main(){
	// ambient
	vec3 ambient = phong_shading.amb_coe * vec3(phong_shading.amb_color);
	
	// diffuse
	vec3 n;
	if(tex_enb.is_normalMap){
		n = texture2D(NormalMap, frag_texcoord).rgb;
		n = normalize(n * 2.0 - 1.0);
		n = normalize(frag_TBN * n);
	}
	else
		n = frag_n;
	

	vec3 lightDir = normalize(frag_lightPos - frag_pos);
	float dif = max(dot(n, lightDir), 0.0)* phong_shading.dif_coe;
	vec3 diffuse;
	if(tex_enb.is_textureMap)
		diffuse = dif * vec3(texture2D(TextureMap, frag_texcoord));
	else
		diffuse = dif * vec3(phong_shading.dif_color);

	// specular
	vec3 viewDir = normalize(frag_camPos - frag_pos);
	vec3 reflectDir = reflect(-lightDir, n);
	float spc = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = spc * phong_shading.spc_coe * vec3(phong_shading.spc_color);
	if(tex_enb.is_specularMap)
		specular *= vec3(texture2D(SpecularMap, frag_texcoord));

	// result
	vec3 res = (diffuse + ambient + specular);
	//if(tex_enb.is_textureMap)
	//	res *=  vec3(texture2D(TextureMap, frag_texcoord));
	
	outColor = vec4(res, 1.0);
	//outColor = texture2D(Texture, frag_texcoord);
}