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

float length_h(const vec3& v) {
	return sqrtf(hyperDot(v, v));
}

vec3 hyperNormalize(const vec3& v) {
	return v * (1 / length_h(v));
}

vec3 perpendicular(vec3 v, vec3 p) {
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
	return v*cosf(angle) + (perpendicular(v, p) * sinf(angle));
}

vec3 movePointOnHyper(vec3 p, vec3 v, float t) {
	return p * coshf(t) + v * sinhf(t);
}

vec3 castToPoincareDisk(vec3 p1) {
	p1 = pointPatchToHyperboloid(p1);
	float t = 1 + p1.z;
	return {p1.x/t, p1.y/t, 0};
}

class HyperbolicCircle {
	vec3 origin;
	vec3 vector;
	float radius;
	int fragments;
	unsigned int VAO{}, VBO{};
	std::vector<float> vertices;
public:
	void computeVertecies() {
		float angle = M_PI / ((float) fragments/2);
		if (!vertices.empty()) {
			vertices.clear();
		}
		vec3 v = vector;
		v = vectorPatchToPoint(origin, v);
		v = hyperNormalize(v);

		for (int i = 0; i < fragments; i++) {
			v = hyperNormalize(vectorPatchToPoint(origin, rotateVector(origin, v, angle)));
			vec3 p = movePointOnHyper(origin, v, radius);
			p = pointPatchToHyperboloid(p);

			p = castToPoincareDisk(p);
			vertices.push_back(p.x);
			vertices.push_back(p.y);
		}
	}

	HyperbolicCircle(vec2 origin, float radius, int fragments = 60) {
		this->origin = pointPatchToHyperboloid(vec3(origin.x, origin.y, 0));
		this->radius = radius;
		this->fragments = fragments;
		this->vector = hyperNormalize(vectorPatchToPoint(this->origin, vec3(0, 1, 0)));
		this->vertices = std::vector<float>();
	}

	HyperbolicCircle(vec3 origin, vec3 direction, float radius, int fragments = 60) {
		this->origin = pointPatchToHyperboloid(origin);
		this->radius = radius;
		this->fragments = fragments;
		this->vector = hyperNormalize(vectorPatchToPoint(this->origin, direction));
		this->vertices = std::vector<float>();
	}

	void draw(int color, vec3 colorVec) {
		glUniform3f(color, colorVec.x, colorVec.y, colorVec.z);
		computeVertecies();

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

		glBufferData(GL_ARRAY_BUFFER, fragments * 2 * sizeof(float), vertices.data(), GL_STATIC_DRAW);
		glDrawArrays(GL_TRIANGLE_FAN, 0, fragments);

		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
	}

	void setOrigin(vec3 point, vec3 direction) {
		this->origin = pointPatchToHyperboloid(point);
		this->vector = hyperNormalize(vectorPatchToPoint(this->origin, direction));
	}
};

class Trail {
	std::vector<float> trail;
	unsigned VAO{}, VBO{};

public:
	void addPoint(vec3 pt) {
		vec3 p = pointPatchToHyperboloid(pt);
		p = castToPoincareDisk(p);
		trail.push_back(p.x);
		trail.push_back(p.y);
	}

	void draw() {
		if (!trail.empty()){
			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

			glBufferData(GL_ARRAY_BUFFER, trail.size() * sizeof(float), trail.data(), GL_STATIC_DRAW);
			glDrawArrays(GL_LINE_STRIP, 0, (int) trail.size() / 2);

			glDeleteBuffers(1, &VBO);
			glDeleteVertexArrays(1, &VBO);
		}
	}
};

class Hami {
	HyperbolicCircle* body;
	HyperbolicCircle* eye1{};
	HyperbolicCircle* eye2{};
	HyperbolicCircle* mouth{};
	Trail* trail{};
	float radius;
	vec3 position;
	vec3 direction;

	void recalculateBody() {
		vec3 pos = position;
		vec3 tempDir = hyperNormalize(rotateVector(pos, this->direction, M_PI / 4));
		vec3 eye1Pos = pointPatchToHyperboloid(movePointOnHyper(pos, tempDir, radius));
		if (this->eye1 == nullptr)
			this->eye1 = new HyperbolicCircle(vec2(eye1Pos.x, eye1Pos.y), this->direction, radius/4);
		else
			this->eye1->setOrigin(eye1Pos, this->direction);
		pos = position;
		tempDir = hyperNormalize(rotateVector(pos, this->direction, -M_PI / 4));
		vec3 eye2Pos = pointPatchToHyperboloid(movePointOnHyper(pos, tempDir, radius));
		if (this->eye2 == nullptr)
			this->eye2 = new HyperbolicCircle(vec2(eye2Pos.x, eye2Pos.y), this->direction, radius/4);
		else
			this->eye2->setOrigin(eye2Pos, this->direction);
		pos = position;
		vec3 mouthPos = pointPatchToHyperboloid(movePointOnHyper(pos, this->direction, radius));
		if (this->mouth == nullptr)
			this->mouth = new HyperbolicCircle(vec2(mouthPos.x, mouthPos.y), this->direction, radius/3);
		else
			this->mouth->setOrigin(mouthPos, this->direction);
		if (this->trail == nullptr)
			trail = new Trail();
	}

public:
	Hami(vec2 position, float radius) {
		this->radius = radius;
		this->position = pointPatchToHyperboloid(vec3(position.x, position.y, 0));
		this->direction = hyperNormalize(vectorPatchToPoint(this->position, vec3(0, 1, 0)));
		this->body = new HyperbolicCircle(position, direction, radius, 60);
		this->recalculateBody();
	}

	void drawTrail(int color) {
		glUniform3f(color, 1.0f, 1.0f, 1.0f);
		if (trail != nullptr)
			trail->draw();
	}

	void draw(int color, vec3 colorVec) {
		body->draw(color, colorVec);
		glUniform3f(color, 1.0f, 1.0f, 1.0f);
		if (eye1 != nullptr && eye2 != nullptr && mouth != nullptr) {
			eye1->draw(color, vec3(1, 1, 1));
			eye2->draw(color, vec3(1, 1, 1));
			glUniform3f(color, 0.0f, 0.0f, 0.0f);
			mouth->draw(color, vec3(0, 0, 0));
			glUniform3f(color, 1.0f, 1.0f, 1.0f);
		}
	}

	void move(float distance) {
		this->position = pointPatchToHyperboloid(movePointOnHyper(position, direction, distance));
		this->direction = hyperNormalize(vectorPatchToPoint(position, direction));
		this->body->setOrigin(this->position, this->direction);
		this->recalculateBody();
		trail->addPoint(position);
	}

	void rotate(float angle) {
		this->direction = hyperNormalize(vectorPatchToPoint(position, rotateVector(position, direction, angle)));
		this->body->setOrigin(this->position, this->direction);
		this->recalculateBody();
	}
};

HyperbolicCircle* poincareDisk = new HyperbolicCircle(vec2(0.0f, 0.0f), 20.0f, 60);

Hami* hami1 = new Hami(vec2(0.0f, 0.0f), 0.5f);
Hami* hami2 = new Hami(vec2(0.0f, 0.0f), 0.5f);

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
	glClearColor(147.0f/255, 147.0f/255, 147.0f/255, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	int color = glGetUniformLocation(gpuProgram.getId(), "color");
	poincareDisk->draw(color, vec3(0, 0, 0));

	hami2->drawTrail(color);
	hami1->drawTrail(color);

	hami2->draw(color, vec3(0, 1, 0));
	hami1->draw(color, vec3(1, 0, 0));

	glutSwapBuffers();
}

bool s = false;
bool e = false;
bool f = false;

void onKeyboard(unsigned char key, int pX, int pY) {
	switch (key) {
		case 's':
			s = true;
			break;
		case 'e':
			e = true;
			break;
		case 'f':
			f = true;
			break;
		case 27:
			exit(0);
		default:
			break;
	}
}

void onKeyboardUp(unsigned char key, int pX, int pY) {
	switch (key) {
		case 's':
			s = false;
			break;
		case 'e':
			e = false;
			break;
		case 'f':
			f = false;
			break;
		default:
			break;
	}
}

void onMouseMotion(int pX, int pY) {}

void onMouse(int button, int state, int pX, int pY) {}

void onIdle() {
	if (s) {
		hami1->rotate(-M_PI/60);
	}
	if (e) {
		hami1->move(0.04f);
	}
	if (f) {
		hami1->rotate(M_PI/60);
	}
	hami2->move(0.04f);
	hami2->rotate(-M_PI/60);
	glutPostRedisplay();
}
