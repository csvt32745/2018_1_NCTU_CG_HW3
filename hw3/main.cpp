/*
	Control Function:

	## HW Spec
	O	:	Switch Slice/Stack of Earth between 360/180 and 4/2
	P	:	Realtime pause
	
	## Additional features
	W	:	Draw axises X,Y,Z as R,G,B
	E/R	:	Adjust global var 'X'
	T/Y :	Adjust global var 'Y'
	A/D	:	Rotate camera around Y-axis
	Z/X :	Adjust camera height
	B	:	Move camera to default postion
	Right/Left Click:
			Adjust distance to Y-axis of camera 
*/
#include <Windows.h>
#include <iostream>
#include <glew.h>
#include <glut.h>
#include <FreeImage.h>
#include <string>
#include <MyVec3.h>
#include "shader.h"

using namespace std;

#define fzero 0.0f
#define pi 3.1415926f
#define ppi 6.2831852f

// Arugment Function
void display();
void reshape(int _width, int _height);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void mouseMotion(int x, int y) {}
void passiveMouseMotion(int x, int y) {}
void idle();

// General Function
void setShader();
GLuint setTexture(const char *filename);
void createAxises(int _range);
float rad_per_deg = pi / (float)180;
void mySphere(int slice, int stack);

// Shader Object
GLuint shaderProgram;
GLuint vertexBO;
GLuint tex;

// Shader Uniform
GLfloat projMtx[16];
GLfloat modMtx[16];
GLint projMtxLoc;
GLint modMtxLoc;
GLint lightsrcLoc;



struct VertexAttribute
{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat texcoord[2];
};

struct PhongShading
{
	GLfloat amb_color[4] = { 0.6f, 0.7f, 0.5f, 1.0f };
	GLfloat dif_color[4] = { 0.35f, 0.3f, 0.3f, 1.0f };
	GLfloat spc_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat amb_coe = 0.3f;
	GLfloat dif_coe = 1.0f;
	GLfloat spc_coe = 0.8f;
};

struct TextureEnable
{
	bool is_textureMap = true;
	bool is_normalMap = true;
	bool is_specularMap = true;
};

// Class 
class Planet {
public:
	GLint slice;
	GLint stack;
	GLfloat obliquity = fzero;
	GLfloat radius;
	GLfloat rotation;
	GLfloat deg_rot = fzero;
	GLfloat rot_axis;
	GLuint texture_map;
	GLuint normal_map;
	GLuint specular_map;

	PhongShading phong_shading_arg;
	GLfloat light_revolution;
	GLfloat light_deg_rev = pi;
	GLfloat cam_pos[3];
	TextureEnable tex_enb;

	Planet(GLint _slice, GLint _stack, GLfloat _rad, GLfloat _rot,
		GLuint _tex_id, GLuint _normal_map, GLuint _specular_map) {
		slice = _slice;
		stack = _stack;
		radius = _rad;
		rotation = _rot;
		texture_map = _tex_id;
		normal_map = _normal_map;
		specular_map = _specular_map;
		rot_axis = radius * 2;
		
		light_revolution = _rot / 10;
	}


	void setSliceStack(GLint _slice, GLint _stack) {
		slice = _slice;
		stack = _stack;
	}

	void draw() {
		setLight();
		setPSModel();
		setCamPos();
		setTex();
		setTexEnb();

		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "Projection"), 1, GL_FALSE, projMtx);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "ModelView"), 1, GL_FALSE, modMtx);
		glUniform1f(glGetUniformLocation(shaderProgram, "Ang"), deg_rot);
		glUniform1f(glGetUniformLocation(shaderProgram, "Obq"), obliquity);

		glUseProgram(shaderProgram);		
		
		glDrawArrays(GL_TRIANGLES, 0, slice*stack * 6);
		glUseProgram(0);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void drawAxis() {
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);
		glColor3f(1.0f, 1.0f, 0.5f);
		glVertex3f(0, rot_axis, 0);
		glVertex3f(0, -rot_axis, 0);
		glEnd();
		glEnable(GL_LIGHTING);
	}

	void updateDeg(GLfloat _time) {
		deg_rot += rotation * _time;
		while (deg_rot > 360.0f)
			deg_rot -= 360.0f;

		light_deg_rev += light_revolution * _time * rad_per_deg;
		while (light_deg_rev > ppi)
			light_deg_rev -= ppi;
	}

	void setLight() {
		GLfloat pos[3];
		pos[0] = cos(light_deg_rev)*3;
		pos[1] = fzero;
		pos[2] = -sin(light_deg_rev)*3;
		glUseProgram(shaderProgram);
		lightsrcLoc = glGetUniformLocation(shaderProgram, "LightSource");
		glUniform3fv(lightsrcLoc, 1, pos);
	}

	void setPSModel() {
		GLuint psLoc = glGetUniformLocation(shaderProgram, "phong_shading.amb_color");
		glUniform4fv(psLoc, 1, phong_shading_arg.amb_color);
		psLoc = glGetUniformLocation(shaderProgram, "phong_shading.dif_color");
		glUniform4fv(psLoc, 1, phong_shading_arg.dif_color);
		psLoc = glGetUniformLocation(shaderProgram, "phong_shading.spc_color");
		glUniform4fv(psLoc, 1, phong_shading_arg.spc_color);
		psLoc = glGetUniformLocation(shaderProgram, "phong_shading.amb_coe");
		glUniform1f(psLoc, phong_shading_arg.amb_coe);
		psLoc = glGetUniformLocation(shaderProgram, "phong_shading.dif_coe");
		glUniform1f(psLoc, phong_shading_arg.dif_coe);
		psLoc = glGetUniformLocation(shaderProgram, "phong_shading.spc_coe");
		glUniform1f(psLoc, phong_shading_arg.spc_coe);
	}

	void setTex() {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_map);
		glUniform1i(glGetUniformLocation(shaderProgram, "TextureMap"), 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normal_map);
		glUniform1i(glGetUniformLocation(shaderProgram, "NormalMap"), 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, specular_map);
		glUniform1i(glGetUniformLocation(shaderProgram, "SpecularMap"), 2);
	}

	void setCamPos() {
		glUniform3fv(glGetUniformLocation(shaderProgram, "CamPos"), 1, cam_pos);
	}

	void updateCamPos(const GLfloat campos[3]) {
		cam_pos[0] = campos[0];
		cam_pos[1] = campos[1];
		cam_pos[2] = campos[2];
		setCamPos();
	}
	void setPoints() {
		mySphere(slice, stack);
	}
	void setTexEnb() {
		GLuint Loc = glGetUniformLocation(shaderProgram, "tex_enb.is_textureMap");
		glUniform1i(Loc, tex_enb.is_textureMap);
		Loc = glGetUniformLocation(shaderProgram, "tex_enb.is_normalMap");
		glUniform1i(Loc, tex_enb.is_normalMap);
		Loc = glGetUniformLocation(shaderProgram, "tex_enb.is_specularMap");
		glUniform1i(Loc, tex_enb.is_specularMap);
	}
};

// General Argument
int width = 960, height = 540;

MyVec3 cam_pos(0.0f, 0.0f, 3.0f);
float cam_rad = 3.0f;
float cam_deg = pi / (float)2;	//default camera deg = 90 deg
float cam_delta_deg = pi / (float)18;	// 10 deg
float cam_move_speed = 1.0f;

int time_start = 0, time_end = 0, time_delta = 0;
float X = 0.1f; // related to degree
Planet *earth = NULL;
bool is_pause = false, is_axises = false;

int main(int argc, char **argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("^q^");
	

	/*
		GLUT Event
					*/
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutPassiveMotionFunc(passiveMouseMotion);
	glutIdleFunc(idle);



	/*
		Initialization
						*/
	glewInit();
	setShader();
	glShadeModel(GL_SMOOTH);
	earth = new Planet(360, 180, 1, X,
		setTexture("earth_texture_map.jpg"), setTexture("earth_normal_map.tif"),
		setTexture("earth_specular_map.tif"));
	earth->obliquity = -23.5f;
	earth->updateCamPos(cam_pos.getArr());
	earth->setPoints();
	/*
		Main Loop
					*/
	glutMainLoop();
	return 0;
}


void display() 
{
	time_end = glutGet(GLUT_ELAPSED_TIME);
	if (is_pause)
		time_delta = fzero;
	else
		time_delta = time_end - time_start;

	// ModelView Matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	
	gluLookAt(cam_pos.x, cam_pos.y, cam_pos.z,
		fzero, fzero, fzero,
		fzero, 1.0f, fzero);

	// Projection Matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, width / (GLfloat)height, 0.1, 1000);

	// Viewport Matrix
	glViewport(0, 0, width, height);

	// General Setting
	glMatrixMode(GL_MODELVIEW);
	glClearColor(0.1f, 0.1f, 0.1f, fzero);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClear(GL_DEPTH_BUFFER_BIT);


	if (is_axises)
		createAxises(100);
	// Drawing Process

	
	glGetFloatv(GL_PROJECTION_MATRIX, projMtx);
	glGetFloatv(GL_MODELVIEW_MATRIX, modMtx);
	
	earth->updateDeg(time_delta);
	glPushMatrix();
	// Draw Earth
	glRotatef(earth->obliquity, 0, 0, 1);
	earth->draw();
	earth->drawAxis();
	glPopMatrix();

	

	//glDrawArrays(GL_TRIANGLES, 0, 4);
	
	time_start = time_end;
	glutSwapBuffers();
}

void reshape(int _w, int _h)
{
	width = _w;
	height = _h;
}

void keyboard(unsigned char key, int x, int y) {
	printf("Pressed : %u\n", key);
	bool is_updateX = false;
	bool is_updateCam = false;
	switch (key)
	{
	// HW Spec
	case '1':
		earth->tex_enb.is_textureMap = !earth->tex_enb.is_textureMap;
		if(earth->tex_enb.is_textureMap)
			cout << "Texture ON" << endl;
		else
			cout << "Texture OFF" << endl;
		break;

	case '2':
		earth->tex_enb.is_normalMap = !earth->tex_enb.is_normalMap;
		if (earth->tex_enb.is_normalMap)
			cout << "Normal ON" << endl;
		else
			cout << "Normal OFF" << endl;
		break;

	case '3':
		earth->tex_enb.is_specularMap = !earth->tex_enb.is_specularMap;
		if (earth->tex_enb.is_specularMap)
			cout << "Specular ON" << endl;
		else
			cout << "Specular OFF" << endl;
		break;

	case 'P':
	case 'p':
		is_pause = !is_pause;
		break;

	// Additional features
	
	case 'W':
	case 'w':
		is_axises = !is_axises;
		break;

	case 'E':
	case 'e':
		X -= 0.01f;
		if (X < fzero)
			X = fzero;
		is_updateX = true;
		break;

	case 'R':
	case 'r':
		X += 0.01f;
		if (X > 0.8f)
			X = 0.8f;
		is_updateX = true;
		break;

	case 'T':
	case 't':
		break;

	case 'Y':
	case 'y':
		break;

	case 'A':
	case 'a':
		cam_deg -= cam_delta_deg;
		is_updateCam = true;
		break;

	case 'D':
	case 'd':
		cam_deg += cam_delta_deg;
		is_updateCam = true;
		break;

	case 'Z':
	case 'z':
		cam_pos.y += cam_move_speed;
		is_updateCam = true;
		break;

	case 'X':
	case 'x':
		cam_pos.y -= cam_move_speed;
		is_updateCam = true;
		break;

	case 'B':
	case 'b':
		cam_rad = 3.0f;
		cam_deg = pi / 2.0f;
		cam_pos.y = 0.0f;
		is_updateCam = true;
		break;

	default:
		break;
	}

	

	if (is_updateCam) {
		if (cam_deg < fzero) {
			cam_deg += ppi;
		}
		else if (cam_deg > ppi) {
			cam_deg -= ppi;
		}
		cam_pos.x = cam_rad * cos(cam_deg);
		cam_pos.z = cam_rad * sin(cam_deg);
		earth->updateCamPos(cam_pos.getArr());
	}

	if (is_updateX) {
		earth->rotation = X;
		earth->light_revolution = X / 10;
	}
}

void mouse(int button, int state, int x, int y) {
	// Adjust the camera distance to Y-axis
	if (button == 2 && state == 0) {
		// Right button
		cam_rad += 1.0f;
		if (cam_rad > 100.0f)
			cam_rad = 100.0f;
	}
	else if (button == 0 && state == 0) {
		// Left button
		cam_rad -= 1.0f;
		if (cam_rad <= fzero)
			cam_rad = 0.1f;
	}
}

void idle() {
	glutPostRedisplay();
}


void setShader() {
	GLuint vert = createShader("Shaders/example.vert", "vertex");
	GLuint geom = createShader("Shaders/example.geom", "geometry");
	GLuint frag = createShader("Shaders/example.frag", "fragment");
	shaderProgram = createProgram(vert, geom, frag);

	glGenBuffers(1, &vertexBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBO);

	//glBufferData(GL_ARRAY_BUFFER, sizeof(VertexAttribute) * 6, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)(offsetof(VertexAttribute, position)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)(offsetof(VertexAttribute, normal)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)(offsetof(VertexAttribute, texcoord)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vertexBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBO);
}



GLuint setTexture(const char *filename) 
{
	glEnable(GL_TEXTURE_2D);
	FIBITMAP* bm = FreeImage_Load(FreeImage_GetFileType(filename, 0), filename);
	FIBITMAP* bm32 = FreeImage_ConvertTo32Bits(bm);
	GLint width = FreeImage_GetWidth(bm32);
	GLint height = FreeImage_GetHeight(bm32);
	GLuint tex_id;
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(bm32));
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 5);
	//glGenerateTextureMipmap(tex_id);

	glBindTexture(GL_TEXTURE_2D, 0);
	return tex_id;
}

void createAxises(int _range) {
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);

	// X axis
	glColor3f(1.0f, fzero, fzero);
	glVertex3i(_range, 0, 0);
	glVertex3i(-_range, 0, 0);
	// Y axis
	glColor3f(fzero, 1.0f, fzero);
	glVertex3i(0, _range, 0);
	glVertex3i(0, -_range, 0);
	// Z axis
	glColor3f(fzero, fzero, 1.0f);
	glVertex3i(0, 0, _range);
	glVertex3i(0, 0, -_range);

	glEnd();
	glEnable(GL_LIGHTING);
}

void mySphere(int slice, int stack) {
	int size = slice * stack * 6;
	VertexAttribute vertices[4];
	VertexAttribute *buf_vtx = new VertexAttribute[size];
	glBindBuffer(GL_ARRAY_BUFFER, vertexBO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)(offsetof(VertexAttribute, position)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)(offsetof(VertexAttribute, normal)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribute), (void*)(offsetof(VertexAttribute, texcoord)));

	int direction[4][2] = { { 0, 0 },{ 1, 0 },{ 0, 1 },{ 1, 1 } };
	int index[6] = { 2, 1, 0, 2, 3, 1 };
	float slice_step = 2 * pi / slice, stack_step = pi / stack;
	int count = 0;
	for (int i = 0; i < slice; i++) {
		//glBegin(GL_TRIANGLES);
		for (int j = 0; j < stack; j++) {
			for (int k = 0; k < 4; k++) {
				vertices[k].position[0] = vertices[k].normal[0] = sin((j + direction[k][1]) * stack_step) * cos((i + direction[k][0])*slice_step);
				vertices[k].position[1] = vertices[k].normal[1] = cos((j + direction[k][1]) * stack_step);
				vertices[k].position[2] = vertices[k].normal[2] = sin((j + direction[k][1]) * stack_step) * sin((i + direction[k][0])*slice_step);
				vertices[k].texcoord[0] = 1 - float(i + direction[k][0]) / slice;
				vertices[k].texcoord[1] = 1 - float(j + direction[k][1]) / stack;
			}
			for (int k = 0; k < 6; k++) {
				buf_vtx[count++] = vertices[index[k]];
				//glNormal3fv(vertices[index[k]].normal);
				//glTexCoord2fv(vertices[index[k]].texcoord);
				//glVertex3fv(vertices[index[k]].position);
			}

		}
		//glEnd();
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexAttribute) * size, buf_vtx, GL_STATIC_DRAW);
	
	delete[] buf_vtx;
}