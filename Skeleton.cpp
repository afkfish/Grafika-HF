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

float hyperbolicDot(vec3 v1, vec3 v2) {
	return (v1.x * v2.x) + (v1.y*v2.y) - (v1.z*v2.z);
}

float hyperbolicLength(const vec3& v) {
	return sqrtf(hyperbolicDot(v, v));
}

vec3 hyperbolicNormalize(const vec3& v) {
	return v * (1 / hyperbolicLength(v));
}

vec3 perpendicular(vec3 v, vec3 p) {
	return cross(vec3(v.x, v.y, -v.z), vec3(p.x, p.y, -p.z));
}

vec3 vectorPatchToPoint(vec3 p, vec3 v) {
	if (abs(hyperbolicDot(p, v)) < 1e-7)
		return v;
	float lambda = (-hyperbolicDot(p, v)) / (hyperbolicDot(p, p));
	return v + (p * lambda);
}

vec3 pointPatchToHyperboloid(vec3 p) {
	return {p.x, p.y, sqrtf(p.x*p.x + p.y*p.y + 1)};
}

vec3 rotateVector(vec3 p, vec3 v, float angle) {
	return v*cosf(angle) + (perpendicular(v, p) * sinf(angle));
}

vec3 movePointOnHyperboloid(vec3 p, vec3 v, float t) {
	return p * coshf(t) + v * sinhf(t);
}

vec3 castToPoincareDisk(vec3 p1) {
	p1 = pointPatchToHyperboloid(p1);
	return {p1.x/(1 + p1.z), p1.y/(1 + p1.z), 0};
}

vec3 pointDirection(vec3 p, vec3 q) {
    return -(q - p*coshf(1)) / sinhf(1);
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
		vertices.clear();

		vec3 v = vectorPatchToPoint(origin, vector);
		v = hyperbolicNormalize(v);

		for (int i = 0; i < fragments; i++) {
			v = hyperbolicNormalize(vectorPatchToPoint(origin, rotateVector(origin, v, angle)));
			vec3 p = pointPatchToHyperboloid(movePointOnHyperboloid(origin, v, radius));

			p = castToPoincareDisk(p);
			vertices.push_back(p.x);
			vertices.push_back(p.y);
		}
	}

	HyperbolicCircle(vec2 origin, float radius, int fragments = 60) {
		this->origin = pointPatchToHyperboloid(vec3(origin.x, origin.y, 0));
		this->radius = radius;
		this->fragments = fragments;
		this->vector = hyperbolicNormalize(vectorPatchToPoint(this->origin, vec3(0, 1, 0)));
		this->vertices = std::vector<float>();
	}

	HyperbolicCircle(vec3 origin, vec3 direction, float radius, int fragments = 60) {
		this->origin = pointPatchToHyperboloid(origin);
		this->radius = radius;
		this->fragments = fragments;
		this->vector = hyperbolicNormalize(vectorPatchToPoint(this->origin, direction));
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
		this->vector = hyperbolicNormalize(vectorPatchToPoint(this->origin, direction));
	}
};

class Trail {
	std::vector<float> trail;
	unsigned int VAO{}, VBO{};

public:
	void addPoint(vec3 pt) {
		vec3 p = castToPoincareDisk(pointPatchToHyperboloid(pt));
		trail.push_back(p.x);
		trail.push_back(p.y);
	}

	void draw() {
		if (!trail.empty()) {
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
    HyperbolicCircle* pupil1{};
	HyperbolicCircle* eye2{};
    HyperbolicCircle* pupil2{};
	HyperbolicCircle* mouth{};
	Trail* trail{};
	int i = 0;
	float radius;
	float mouthOffset;
    vec3 direction;
    vec3 otherHamiPos;

	void recalculateBody() {
		vec3 pos = position;
		vec3 tempDir = hyperbolicNormalize(rotateVector(pos, this->direction, M_PI / 4));
		vec3 eye1Pos = pointPatchToHyperboloid(movePointOnHyperboloid(pos, tempDir, radius));
        vec3 pupil1Dir = hyperbolicNormalize(vectorPatchToPoint(eye1Pos, pointDirection(otherHamiPos, eye1Pos)));
        vec3 pupil1pos = pointPatchToHyperboloid(movePointOnHyperboloid(eye1Pos, pupil1Dir, radius / 8));
		if (this->eye1 == nullptr || this->pupil1 == nullptr) {
            this->eye1 = new HyperbolicCircle(vec2(eye1Pos.x, eye1Pos.y), this->direction, radius / 4);
            this->pupil1 = new HyperbolicCircle(vec2(pupil1pos.x, pupil1pos.y), this->direction, radius / 8);
        } else {
            this->eye1->setOrigin(eye1Pos, this->direction);
            this->pupil1->setOrigin(pupil1pos, this->direction);
        }
		pos = position;
		tempDir = hyperbolicNormalize(rotateVector(pos, this->direction, -M_PI / 4));
		vec3 eye2Pos = pointPatchToHyperboloid(movePointOnHyperboloid(pos, tempDir, radius));
        vec3 pupil2Dir = hyperbolicNormalize(vectorPatchToPoint(eye2Pos, pointDirection(otherHamiPos, eye2Pos)));
        vec3 pupil2pos = pointPatchToHyperboloid(movePointOnHyperboloid(eye2Pos, pupil2Dir, radius / 8));
		if (this->eye2 == nullptr || this->pupil2 == nullptr) {
            this->eye2 = new HyperbolicCircle(vec2(eye2Pos.x, eye2Pos.y), this->direction, radius / 4);
            this->pupil2 = new HyperbolicCircle(vec2(pupil2pos.x, pupil2pos.y), this->direction, radius / 8);
        } else {
            this->eye2->setOrigin(eye2Pos, this->direction);
            this->pupil2->setOrigin(pupil2pos, this->direction);
		}
		pos = position;
		vec3 mouthPos = pointPatchToHyperboloid(
				movePointOnHyperboloid(pos, this->direction, radius * 1.25f + (mouthOffset * radius / 3)));
		if (this->mouth == nullptr)
			this->mouth = new HyperbolicCircle(vec2(mouthPos.x, mouthPos.y), this->direction, radius/3);
		else
			this->mouth->setOrigin(mouthPos, this->direction);
		if (this->trail == nullptr)
			trail = new Trail();
	}

public:
    vec3 position;

	Hami(vec2 position, float radius, vec3 otherHamiPos) {
		this->radius = radius;
		this->position = pointPatchToHyperboloid(vec3(position.x, position.y, 0));
		this->direction = hyperbolicNormalize(vectorPatchToPoint(this->position, vec3(0, 1, 0)));
		this->body = new HyperbolicCircle(position, direction, radius);
        this->otherHamiPos = otherHamiPos;
		this->recalculateBody();
	}

	void drawTrail(int color) {
		glUniform3f(color, 1.0f, 1.0f, 1.0f);
		trail->draw();
	}

	void draw(int color, vec3 colorVec) {
		body->draw(color, colorVec);
		glUniform3f(color, 1.0f, 1.0f, 1.0f);
		eye1->draw(color, vec3(1, 1, 1));
		pupil1->draw(color, vec3(0, 0, 1));
		eye2->draw(color, vec3(1, 1, 1));
		pupil2->draw(color, vec3(0, 0, 1));
		glUniform3f(color, 0.0f, 0.0f, 0.0f);
		mouth->draw(color, vec3(0, 0, 0));
		glUniform3f(color, 1.0f, 1.0f, 1.0f);
	}

	void move(float distance) {
		this->position = pointPatchToHyperboloid(movePointOnHyperboloid(position, direction, distance));
		this->direction = hyperbolicNormalize(vectorPatchToPoint(position, direction));
		this->body->setOrigin(this->position, this->direction);
		this->recalculateBody();
		trail->addPoint(position);
	}

	void rotate(float angle) {
		this->direction = hyperbolicNormalize(vectorPatchToPoint(position, rotateVector(position, direction, angle)));
		this->body->setOrigin(this->position, this->direction);
		this->recalculateBody();
	}

    void update(vec3 otherPos) {
        this->otherHamiPos = otherPos;
		this->i ++;
		if (i >= 120) i = 0;
		this->mouthOffset = sinf(0 - i*M_PI/120);
        recalculateBody();
    }
};

HyperbolicCircle* poincareDisk = new HyperbolicCircle(vec2(0.0f, 0.0f), 20.0f);

Hami* hami1 = new Hami(vec2(0.0f, 0.0f), 0.5f, vec3(0, 0, 0));
Hami* hami2 = new Hami(vec2(0.0f, 0.0f), 0.5f, vec3(0, 0, 0));

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

bool s, e, f;

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

	hami1->update(hami2->position);
	hami2->update(hami1->position);
	glutPostRedisplay();
}
