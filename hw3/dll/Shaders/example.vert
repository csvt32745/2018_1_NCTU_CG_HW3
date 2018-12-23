#version 450

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;


uniform mat4 Projection;
uniform mat4 ModelView;
uniform float Ang;
uniform float Obq;
uniform vec3 LightSource;
uniform vec3 CamPos;

out vec3 geom_pos;
out vec3 geom_n;
out vec2 geom_texcoord;
out vec3 geom_lightPos;
out vec3 geom_camPos;

void main() {
	float cosAng = cos(radians(Ang));
	float sinAng = -sin(radians(Ang));
	float cosObq = cos(radians(Obq));
	float sinObq = -sin(radians(Obq));
	mat3 rotAng = mat3(
		cosAng, 0, sinAng,
		0, 1, 0,
		-sinAng, 0, cosAng
	);

	mat3 rotObq = mat3(
		cosObq, -sinObq, 0,
		sinObq, cosObq, 0,
		0, 0, 1
	);

	vec4 new_pos = vec4(rotObq * rotAng * pos, 1.0);
	
	mat4 M = Projection * ModelView;
	gl_Position = M * new_pos;
	
	geom_n = rotObq * rotAng * normal;
	geom_texcoord = texcoord;
	geom_pos = vec3(ModelView * new_pos);
	//geom_pos = vec3(new_pos);
	geom_lightPos = vec3(ModelView * vec4(LightSource, 1.0));
	//geom_lightPos = LightSource;

	geom_camPos = vec3(ModelView * vec4(CamPos, 1.0));
	//geom_camPos = CamPos;
}