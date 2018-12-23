#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;
 
struct TextureEnable
{
	bool is_textureMap;
	bool is_normalMap;
	bool is_specularMap;
};

in vec3 geom_pos[3];
in vec3 geom_n[3];
in vec2 geom_texcoord[3];
in vec3 geom_lightPos[3];
in vec3 geom_camPos[3];

out vec2 frag_texcoord;
out vec3 frag_n;
out vec3 frag_pos;
out vec3 frag_lightPos;
out vec3 frag_camPos;
out mat3 frag_TBN;

uniform TextureEnable tex_enb;

void main()
{
	//vec3 edge1 = gl_in[2].gl_Position - gl_in[1].gl_Position;
	//vec3 edge2 = gl_in[3].gl_Position - gl_in[1].gl_Position;
	//vec2 dUV1 = geom_texcoord[2] - geom_texcoord[1];
	//vec2 dUV2 = geom_texcoord[3] - geom_texcoord[1];
	mat2x3 TB;
	if(tex_enb.is_normalMap){
		mat2 dUV = mat2( 
				geom_texcoord[1] - geom_texcoord[0],
				geom_texcoord[2] - geom_texcoord[0] 
			);
		mat2x3 E = mat2x3(
				gl_in[1].gl_Position - gl_in[0].gl_Position,
				gl_in[2].gl_Position - gl_in[0].gl_Position
			);
		/*
		*	T(E) = T(dUV) o TB
		*	inv(T(dUV)) o T(E) = TB
		*	T(E o inv(dUV)) = TB
		*/
		TB = E*inverse(dUV);
	}

	for(int i = 0; i < gl_in.length(); i++)
	{
		gl_Position = gl_in[i].gl_Position;
		frag_texcoord = geom_texcoord[i];
		
		// apply TBN
		

		frag_n = geom_n[i];
		frag_pos = geom_pos[i];
		frag_lightPos = geom_lightPos[i];
		frag_camPos = geom_camPos[i];
		if(tex_enb.is_normalMap){
			mat3 TBN = mat3(
				normalize(TB[0]),
				normalize(TB[1]), 
				normalize(geom_n[0]));
			frag_TBN = TBN;
		}
		EmitVertex();
	}
	EndPrimitive();
}