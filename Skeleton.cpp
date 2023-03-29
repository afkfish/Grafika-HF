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

const char * const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;		// transform vp from modeling space to normalized device space
	}
)";

const char * const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers
	
	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

float hyperDot(vec3 v1, vec3 v2) {
	return (v1.x * v2.x) + (v1.y*v2.y) - (v1.z*v2.z);
}

vec3 perpendicular(vec3 p, vec3 v) {
	return cross(vec3(v.x, v.y, -v.z), vec3(p.x, p.y, -p.z));
}

vec3 vectorPatchToPoint(vec3 p, vec3 v) {
	if (abs(hyperDot(p, v)) < 1e-7) {
		return v;
	}
	float lambda = (-hyperDot(p, v)) / (hyperDot(p, p));
	return v + (p * lambda);
}

vec3 pointPatchToHyperboloid(vec3 p) {
	return {p.x, p.y, sqrtf(p.x*p.x + p.y*p.y + 1)};
}

vec3 rotateVector(vec3 p, vec3 v, float angle) {
	return v*cos(angle) + (perpendicular(p, v) * sin(angle));
}

vec3 movePointOnHyper(vec3 p, vec3 v, float t) {
	return p * cosh(t) + v * sinh(t);
}

vec3 castToPoincareDisk(vec3 p1) {
	float t = 1 / (1 + p1.z);
	return {p1.x*t, p1.y*t, 0};
}

class HyperbolicCircle {
	vec3 origin;
	vec3 vector;
	float radius;
	int fragments;
	unsigned int VAO{}, VBO{};
	std::vector<float> vertices;

	void computeVertecies() {
		float angle = M_PI / ((float) fragments/2);
		if (!vertices.empty()) {
			vertices.clear();
		}
		vec3 v = this->vector;
		vec3 p = this->origin;

		pointPatchToHyperboloid(p);
		v = vectorPatchToPoint(p, v);

		for (int i = 0; i < fragments; i++) {
			v = rotateVector(origin, v, angle);
			v = vectorPatchToPoint(origin, v);
			p = movePointOnHyper(origin, v, radius);
			p = castToPoincareDisk(p);
			vertices.push_back(p.x);
			vertices.push_back(p.y);
		}
	}

public:
	HyperbolicCircle(vec2 origin, float radius, int fragments = 60) {
		this->origin = pointPatchToHyperboloid(vec3(origin.x, origin.y, 0));
		this->radius = radius;
		this->fragments = fragments;
		this->vector = vectorPatchToPoint(this->origin, vec3(0, 1, 0));
		this->vertices = std::vector<float>();
	}

	void draw() {
		computeVertecies();

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		glBufferData(GL_ARRAY_BUFFER, fragments * 2 * sizeof(float), vertices.data(), GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLE_FAN, 0, fragments);

		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
	}

	void move(float distance) {
		this->origin = pointPatchToHyperboloid(movePointOnHyper(origin, vector, distance));
		this->vector = vectorPatchToPoint(origin, vector);
	}

	void rotate(float angle) {
		this->vector = vectorPatchToPoint(origin, rotateVector(origin, vector, angle));
	}
};

HyperbolicCircle* poincareDisk = new HyperbolicCircle(vec2(0.0f, 0.0f), 20.0f, 60);

HyperbolicCircle* circle1 = new HyperbolicCircle(vec2(0.0f, 0.0f), 0.5f, 60);
HyperbolicCircle* circle2 = new HyperbolicCircle(vec2(0.0f, 0.0f), 0.5f, 60);

GPUProgram gpuProgram;

void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	gpuProgram.create(vertexSource, fragmentSource, "outColor");

	float MVPtransf[4][4] = { 1, 0, 0, 0,
							  0, 1, 0, 0,
							  0, 0, 1, 0,
							  0, 0, 0, 1 };

	int location = glGetUniformLocation(gpuProgram.getId(), "MVP");
	glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);
}

void onDisplay() {
	glClearColor(216.0f/255, 216.0f/255, 216.0f/255, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	int color = glGetUniformLocation(gpuProgram.getId(), "color");
	glUniform3f(color, 147.0f/255, 147.0f/255, 147.0f/255);
	poincareDisk->draw();

	glUniform3f(color, 36.0f/255, 89.0f/255, 83.0f/255);
	circle1->draw();
	glUniform3f(color, 64.0f/255, 142.0f/255, 145.0f/255);
	circle2->draw();

	glutSwapBuffers();
}

void onKeyboard(unsigned char key, int pX, int pY) {
	if (key == 27) {
		exit(0);
	} else if (key == 'w') {
		circle2->move(0.05f);
		glutPostRedisplay();
	} else if (key == 'a') {
		circle2->rotate(M_PI/15);
		glutPostRedisplay();
	} else if (key == 's') {
		circle2->move(-0.05f);
		glutPostRedisplay();
	} else if (key == 'd') {
		circle2->rotate(-M_PI/15);
		glutPostRedisplay();
	}
}

void onKeyboardUp(unsigned char key, int pX, int pY) {}

void onMouseMotion(int pX, int pY) {}

void onMouse(int button, int state, int pX, int pY) {}

void onIdle() {
	circle1->rotate(-M_PI/180);
	circle1->move(0.01f);
	glutPostRedisplay();
}
