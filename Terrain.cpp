//  ========================================================================
//  COSC422: Computer Graphics (2023);  University of Canterbury.
//  FILE NAME: TerrainP.cpp
//  See Exer14.pdf for details.
//
//	The program generates and loads the mesh data for a terrain floor (100 verts, 81 elems).
//  Requires files  Terrain.vert, Terrain.frag
//                  Terrain.cont, Terrains.eval
//  ========================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "loadTGA.h" 
using namespace std;
bool drone = false;
GLuint vaoID;
GLuint theProgram;
GLuint mvpMatrixLoc, eyeLoc, gLoc, sLoc, wLoc, scaleLoc, lightLoc;
float wScale, gScale, sScale;
GLenum fill_line = GL_FILL;
float lightAngle = 0;
float eye_x = 0, eye_y = 20, eye_z = 30;      //Initial camera position
float look_x = 0, look_y = 0, look_z = -40;    //"Look-at" point along -z direction
float theta = 0;                              //Look angle
float toRad = 3.14159265/180.0;     //Conversion from degrees to rad

float verts[100*3];       //10x10 grid (100 vertices)
GLushort elems[81*4];     //Element array for 9x9 = 81 quad patches

glm::mat4 projView;

//Generate vertex and element data for the terrain floor
void generateData()
{
	int indx, start;
	//verts array
	for(int i = 0; i < 10; i++)   //100 vertices on a 10x10 grid
	{
		for(int j = 0; j < 10; j++)
		{
			indx = 10*i + j;
			verts[3*indx] = 10*i - 45;		//x
			verts[3*indx+1] = 0;			//y
			verts[3*indx+2] = -10*j;		//z
		}
	}

	//elems array
	for(int i = 0; i < 9; i++)
	{
		for(int j = 0; j < 9; j++)
		{
			indx = 9*i +j;
			start = 10*i + j;
			elems[4*indx] = start;
			elems[4*indx+1] = start + 10;
			elems[4*indx+2] = start + 11;
			elems[4*indx+3] = start + 1;			
		}
	}

}

//Loads height map
void loadTexture()
{
	GLuint texID[5];
    glGenTextures(5, texID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID[0]);
	loadTGA("Terrain_hm_01.tga");

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texID[1]);
	loadTGA("Terrain_hm_02.tga");

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texID[2]);
	loadTGA("Grass.tga");

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texID[3]);
	loadTGA("Rock2.tga");

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, texID[4]);
	loadTGA("Water.tga");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);


}

//Loads a shader file and returns the reference to a shader object
GLuint loadShader(GLenum shaderType, string filename)
{
	ifstream shaderFile(filename.c_str());
	if(!shaderFile.good()) cout << "Error opening shader file." << endl;
	stringstream shaderData;
	shaderData << shaderFile.rdbuf();
	shaderFile.close();
	string shaderStr = shaderData.str();
	const char* shaderTxt = shaderStr.c_str();

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderTxt, NULL);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
		const char *strShaderType = NULL;
		cerr <<  "Compile failure in shader: " << strInfoLog << endl;
		delete[] strInfoLog;
	}
	return shader;
}

void update(int _)
{	
	printf("drone: %d", drone);
	if (drone) {
		theta += 0.1;
		glutPostRedisplay();
	}

	glutTimerFunc(50, update, 0);
}


float limitScale(float s)
{
	if (s < 0) s = 0;
	if (s > 1) s = 1;
	return s;
}

void keyInput(unsigned char key, int x, int y)
{
	if (key == ' ') {
		if (fill_line == GL_FILL) fill_line = GL_LINE;
		else fill_line = GL_FILL;
	}
	else if (key == 's') {
		drone = !drone;
	}
	else if (key == 'a') {
		lightAngle -= 3.14159 / 90;
	}
	else if (key == 'd') {
		lightAngle += 3.14159 / 90;
	}
	else if (key == 'p') {
		sScale += 0.01;

	}
	else if (key == 'l') {
		sScale -= 0.01;
		if (gScale > sScale - 0.1) {
			if (sScale - 0.1 < wScale) gScale = wScale;
			else gScale = sScale - 0.1;
		}
	}
	else if (key == 'o') {
		wScale += 0.01;
	}
	else if (key == 'k') {
		wScale -= 0.01;
	}
	wScale = limitScale(wScale);
	gScale = limitScale(gScale);
	sScale = limitScale(sScale);

	glutPostRedisplay();
}

//Initialise the shader program, create and load buffer data
void initialise()
{
//--------Load terrain height map-----------
	loadTexture();
//--------Load shaders----------------------
	GLuint shaderv = loadShader(GL_VERTEX_SHADER, "Terrain.vert");
	GLuint shaderf = loadShader(GL_FRAGMENT_SHADER, "Terrain.frag");
	GLuint shaderc = loadShader(GL_TESS_CONTROL_SHADER, "Terrain.cont");
	GLuint shadere = loadShader(GL_TESS_EVALUATION_SHADER, "Terrains.eval");
	GLuint shaderg = loadShader(GL_GEOMETRY_SHADER, "Terrain.geom");

	GLuint program = glCreateProgram();
	glAttachShader(program, shaderv);
	glAttachShader(program, shaderf);
	glAttachShader(program, shaderc);
	glAttachShader(program, shadere);
	glAttachShader(program, shaderg);

	glLinkProgram(program);

	GLint status;
	glGetProgramiv (program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
	}
	glUseProgram(program);

	gLoc = glGetUniformLocation(program, "grass");
	glUniform1i(gLoc, 2);
	gScale = 0.6;

	sLoc = glGetUniformLocation(program, "snow");
	glUniform1i(sLoc, 3);
	sScale = 0.7;

	wLoc = glGetUniformLocation(program, "water");
	glUniform1i(wLoc, 4);
	wScale = 0.25;

	scaleLoc = glGetUniformLocation(program, "scale");
	
	lightLoc = glGetUniformLocation(program, "light");

	mvpMatrixLoc = glGetUniformLocation(program, "mvpMatrix");
	eyeLoc = glGetUniformLocation(program, "eyePos");

	GLuint texLoc = glGetUniformLocation(program, "heightMap");
	glUniform1i(texLoc, 0);

//---------Load buffer data-----------------------
	generateData();

	GLuint vboID[2];
	glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    glGenBuffers(2, vboID);

    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);  // Vertex position

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elems), elems, GL_STATIC_DRAW);

    glBindVertexArray(0);

	glPatchParameteri(GL_PATCH_VERTICES, 4);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glutKeyboardFunc(keyInput);
}

//Display function to compute uniform values based on transformation parameters and to draw the scene
void display()
{
	glm::vec4 cameraPosn = glm::vec4(eye_x, eye_y, eye_z, 1.0);
	glUniform4fv(eyeLoc, 1, &cameraPosn[0]);

	
	//--------Compute matrices----------------------
	glm::mat4 proj = glm::perspective(30.0f * toRad, 1.25f, 20.0f, 500.0f);  //perspective projection matrix
	glm::mat4 view = lookAt(glm::vec3(eye_x, eye_y, eye_z), glm::vec3(look_x, look_y, look_z), glm::vec3(0.0, 1.0, 0.0)); //view matri
	projView = proj * view;  //Product matrix
	glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, &projView[0][0]);
	
	glm::vec3 scale = glm::vec3(wScale, gScale, sScale);
	glUniform3fv(scaleLoc, 1, &scale[0]);

	glm::vec4 light = normalize(glm::vec4(50 * sin(lightAngle), 50 * cos(lightAngle), -50, 0));
	glUniform4fv(lightLoc, 1, &light[0]);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, fill_line);
	glBindVertexArray(vaoID);
	glDrawElements(GL_PATCHES, 81 * 4, GL_UNSIGNED_SHORT, NULL);
	glFlush();
}

void special(int key, int x, int y)
{
	int step = 0;
	float dir_x, dir_z;
	if (key == GLUT_KEY_LEFT) theta += 0.1;   //in radians
	else if (key == GLUT_KEY_RIGHT) theta -= 0.1;
	else if (key == GLUT_KEY_DOWN) step = -5;
	else if (key == GLUT_KEY_UP) step = 5;
	dir_x = -sin(theta);
	dir_z = -cos(theta);
	eye_x += step * 0.1 * dir_x;
	eye_z += step * 0.1 * dir_z;
	look_x = eye_x + 70 * dir_x;
	look_z = eye_z + 70 * dir_z;

	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
	glutInitWindowSize(1000, 800);
	glutCreateWindow("Terrain");
	glutInitContextVersion (4, 2);
	glutInitContextProfile ( GLUT_CORE_PROFILE );

	if(glewInit() == GLEW_OK)
	{
		cout << "GLEW initialization successful! " << endl;
		cout << " Using GLEW version " << glewGetString(GLEW_VERSION) << endl;
	}
	else
	{
		cerr << "Unable to initialize GLEW  ...exiting." << endl;
		exit(EXIT_FAILURE);
	}

	initialise();
	glutDisplayFunc(display); 
	glutSpecialFunc(special);
	glutTimerFunc(50, update, 0);
	glutMainLoop();
	return 0;
}

