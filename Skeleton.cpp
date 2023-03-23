//=============================================================================================
// Mintaprogram: Z�ld h�romsz�g. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Kis Benedek M.
// Neptun : JOYAXJ
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

// vertex shader in GLSL: It is a Raw string (C++11) since it contains new line characters
const char * const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;		// transform vp from modeling space to normalized device space
	}
)";

// fragment shader in GLSL
const char * const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers
	
	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";
// VAO and VBO handles
GLuint circleVAO;
GLuint circleVBO;

vec3 meroleges(vec3 v, vec3 p) {
	return cross(vec3(v.x, v.y, -v.z), vec3(p.x, p.y, -p.z));
}

class Circle {
	float ori_x, ori_y, radius;
	int fragments;
	std::vector<float> vertices;

	void computeVertecies() {
		float angle = 2.0f * 3.1415926f / (float)fragments;

		for (int i = 0; i < fragments; i++) {
			float x = radius * cos(angle * i);
			float y = radius * sin(angle * i);
			vertices.push_back(x + this->ori_x);
			vertices.push_back(y + this->ori_y);
		}
	}

public:
	Circle(float x, float y, float radius, int fragments = 60) {
		this->ori_x = x;
		this->ori_y = y;
		this->radius = radius;
		this->fragments = fragments;
	}

	void draw() {
		computeVertecies();
		glGenBuffers(1, &circleVBO);
		glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

		vertices.clear();

		glGenVertexArrays(1, &circleVAO);
		glBindVertexArray(circleVAO);
		glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glDrawArrays(GL_TRIANGLE_FAN, 0, fragments);

		glDeleteBuffers(1, &circleVBO);
		glDeleteVertexArrays(1, &circleVBO);
	}

	void move(float x, float y) {
		this->ori_x += x;
		this->ori_y += y;
	}
};

// Global variables
Circle* circle1 = new Circle(0.0f, 0.0f, 1.0f);
Circle* circle2 = new Circle(0.0f, 0.0f, 0.5f);
Circle* circle3 = new Circle(0.0f, 0.0f, 0.25f);

GPUProgram gpuProgram; // vertex and fragment shaders
//unsigned int vao;	   // virtual world on the GPU

// Initialization, create an OpenGL context
void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);

	// create program for the GPU
	gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

// Window has become invalid: Redraw
void onDisplay() {
	glClearColor(216.0f/255, 216.0f/255, 216.0f/255, 0);     // background color
	glClear(GL_COLOR_BUFFER_BIT); // clear frame buffer

	int color = glGetUniformLocation(gpuProgram.getId(), "color");
	glUniform3f(color, 36.0f/255, 89.0f/255, 83.0f/255); // 3 floats

	float MVPtransf[4][4] = { 1, 0, 0, 0,    // MVP matrix,
							  0, 1, 0, 0,    // row-major!
							  0, 0, 1, 0,
							  0, 0, 0, 1 };

	int location = glGetUniformLocation(gpuProgram.getId(), "MVP");	// Get the GPU location of uniform variable MVP
	glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);	// Load a 4x4 row-major float matrix to the specified location

	circle1->draw();
	glUniform3f(color, 64.0f/255, 142.0f/255, 145.0f/255);
	circle2->draw();
	glUniform3f(color, 36.0f/255, 89.0f/255, 83.0f/255);
	circle3->draw();

	glutSwapBuffers(); // exchange buffers for double buffering
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
	if (key == 27) { // ESCAPE key
		exit(0);
	} else if (key == 'w') {
		circle2->move(0.0f, 0.05f);
		glutPostRedisplay();
	} else if (key == 'a') {
		circle2->move(-0.05f, 0.0f);
		glutPostRedisplay();
	} else if (key == 's') {
		circle2->move(0.0f, -0.05f);
		glutPostRedisplay();
	} else if (key == 'd') {
		circle2->move(0.05f, 0.0f);
		glutPostRedisplay();
	}
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// Convert to normalized device space
//	float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
//	float cY = 1.0f - 2.0f * pY / windowHeight;
//	printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	// Convert to normalized device space
//	float cX = 2.0f * pX / windowWidth - 1;	// flip y axis
//	float cY = 1.0f - 2.0f * pY / windowHeight;
//
//	char * buttonStat;
//	switch (state) {
//	case GLUT_DOWN: buttonStat = "pressed"; break;
//	case GLUT_UP:   buttonStat = "released"; break;
//	}
//
//	switch (button) {
//	case GLUT_LEFT_BUTTON:   printf("Left button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);   break;
//	case GLUT_MIDDLE_BUTTON: printf("Middle button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY); break;
//	case GLUT_RIGHT_BUTTON:  printf("Right button %s at (%3.2f, %3.2f)\n", buttonStat, cX, cY);  break;
//	}
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
//	long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program
}
