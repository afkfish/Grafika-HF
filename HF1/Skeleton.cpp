#include "framework.h"

const char * const vertexSource = R"(
	#version 330
	precision highp float;

	uniform mat4 MVP;
	layout(location = 0) in vec2 vp;

	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;
	}
)";

const char * const fragmentSource = R"(
	#version 330
	precision highp float;

	uniform vec3 color;
	out vec4 outColor;

	void main() {
		outColor = vec4(color, 1);
	}
)";

float hyperbolicDot(const vec3 v1, const vec3 v2) {
	return v1.x * v2.x + v1.y*v2.y - v1.z*v2.z;
}

float hyperbolicLength(const vec3& v) {
	return sqrtf(hyperbolicDot(v, v));
}

vec3 hyperbolicNormalize(const vec3& v) {
	return v * (1 / hyperbolicLength(v));
}

vec3 perpendicular(const vec3 v, const vec3 p) {
	return cross(vec3(v.x, v.y, -v.z), vec3(p.x, p.y, -p.z));
}

vec3 vectorPatchToPoint(const vec3 p, const vec3 v) {
	if (abs(hyperbolicDot(p, v)) < 1e-7)
		return v;
	const float lambda = -hyperbolicDot(p, v) / hyperbolicDot(p, p);
	return v + p * lambda;
}

vec3 pointPatchToHyperboloid(vec3 p) {
	return {p.x, p.y, sqrtf(p.x*p.x + p.y*p.y + 1)};
}

vec3 rotateVector(const vec3 p, const vec3 v, const float angle) {
	return v*cosf(angle) + perpendicular(v, p) * sinf(angle);
}

vec3 movePointOnHyperboloid(const vec3 p, const vec3 v, const float t) {
	return p * coshf(t) + v * sinhf(t);
}

vec3 castToPoincareDisk(vec3 p1) {
	p1 = pointPatchToHyperboloid(p1);
	return {p1.x/(1 + p1.z), p1.y/(1 + p1.z), 0};
}

vec3 pointDirection(const vec3 p, const vec3 q) {
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
		const float angle = M_PI / (static_cast<float>(fragments)/2);
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

	HyperbolicCircle(const vec2 origin, const float radius, const int fragments = 60) {
		this->origin = pointPatchToHyperboloid(vec3(origin.x, origin.y, 0));
		this->radius = radius;
		this->fragments = fragments;
		this->vector = hyperbolicNormalize(vectorPatchToPoint(this->origin, vec3(0, 1, 0)));
		this->vertices = std::vector<float>();
	}

	HyperbolicCircle(const vec3 origin, const vec3 direction, const float radius, const int fragments = 60) {
		this->origin = pointPatchToHyperboloid(origin);
		this->radius = radius;
		this->fragments = fragments;
		this->vector = hyperbolicNormalize(vectorPatchToPoint(this->origin, direction));
		this->vertices = std::vector<float>();
	}

	void draw(const int color, const vec3 colorVec) {
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

	void setOrigin(const vec3 point, const vec3 direction) {
		this->origin = pointPatchToHyperboloid(point);
		this->vector = hyperbolicNormalize(vectorPatchToPoint(this->origin, direction));
	}
};

class Trail {
	std::vector<float> trail;
	unsigned int VAO{}, VBO{};

public:
	void addPoint(const vec3 pt) {
		const vec3 p = castToPoincareDisk(pointPatchToHyperboloid(pt));
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
			glDrawArrays(GL_LINE_STRIP, 0, static_cast<int>(trail.size()) / 2);

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
	float mouthOffset = 0;
    vec3 direction;
    vec3 otherHamiPos;

	void recalculateBody() {
		vec3 pos = position;
		vec3 tempDir = hyperbolicNormalize(rotateVector(pos, this->direction, M_PI / 4));
		const vec3 eye1Pos = pointPatchToHyperboloid(movePointOnHyperboloid(pos, tempDir, radius));
		const vec3 pupil1Dir = hyperbolicNormalize(vectorPatchToPoint(eye1Pos, pointDirection(otherHamiPos, eye1Pos)));
		const vec3 pupil1pos = pointPatchToHyperboloid(movePointOnHyperboloid(eye1Pos, pupil1Dir, radius / 8));
		if (this->eye1 == nullptr || this->pupil1 == nullptr) {
            this->eye1 = new HyperbolicCircle(vec2(eye1Pos.x, eye1Pos.y), this->direction, radius / 4);
            this->pupil1 = new HyperbolicCircle(vec2(pupil1pos.x, pupil1pos.y), this->direction, radius / 8);
        } else {
            this->eye1->setOrigin(eye1Pos, this->direction);
            this->pupil1->setOrigin(pupil1pos, this->direction);
        }
		pos = position;
		tempDir = hyperbolicNormalize(rotateVector(pos, this->direction, -M_PI / 4));
		const vec3 eye2Pos = pointPatchToHyperboloid(movePointOnHyperboloid(pos, tempDir, radius));
		const vec3 pupil2Dir = hyperbolicNormalize(vectorPatchToPoint(eye2Pos, pointDirection(otherHamiPos, eye2Pos)));
		const vec3 pupil2pos = pointPatchToHyperboloid(movePointOnHyperboloid(eye2Pos, pupil2Dir, radius / 8));
		if (this->eye2 == nullptr || this->pupil2 == nullptr) {
            this->eye2 = new HyperbolicCircle(vec2(eye2Pos.x, eye2Pos.y), this->direction, radius / 4);
            this->pupil2 = new HyperbolicCircle(vec2(pupil2pos.x, pupil2pos.y), this->direction, radius / 8);
        } else {
            this->eye2->setOrigin(eye2Pos, this->direction);
            this->pupil2->setOrigin(pupil2pos, this->direction);
		}
		pos = position;
		const vec3 mouthPos = pointPatchToHyperboloid(
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

	Hami(const vec2 position, const float radius, const vec3 otherHamiPos) {
		this->radius = radius;
		this->position = pointPatchToHyperboloid(vec3(position.x, position.y, 0));
		this->direction = hyperbolicNormalize(vectorPatchToPoint(this->position, vec3(0, 1, 0)));
		this->body = new HyperbolicCircle(position, direction, radius);
        this->otherHamiPos = otherHamiPos;
		this->recalculateBody();
	}

	void drawTrail(const int color) const {
		glUniform3f(color, 1.0f, 1.0f, 1.0f);
		trail->draw();
	}

	void draw(const int color, const vec3 colorVec) const {
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

	void move(const float distance) {
		this->position = pointPatchToHyperboloid(movePointOnHyperboloid(position, direction, distance));
		this->direction = hyperbolicNormalize(vectorPatchToPoint(position, direction));
		this->body->setOrigin(this->position, this->direction);
		this->recalculateBody();
		trail->addPoint(position);
	}

	void rotate(const float angle) {
		this->direction = hyperbolicNormalize(vectorPatchToPoint(position, rotateVector(position, direction, angle)));
		this->body->setOrigin(this->position, this->direction);
		this->recalculateBody();
	}

    void update(const vec3 otherPos) {
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

	const float MVPtransf[4][4] = { 1, 0, 0, 0,
							  0, 1, 0, 0,
							  0, 0, 1, 0,
							  0, 0, 0, 1 };

	const int location = glGetUniformLocation(gpuProgram.getId(), "MVP");
	glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);
}

void onDisplay() {
	glClearColor(147.0f/255, 147.0f/255, 147.0f/255, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	const int color = glGetUniformLocation(gpuProgram.getId(), "color");
	poincareDisk->draw(color, vec3(0, 0, 0));

	hami2->drawTrail(color);
	hami1->drawTrail(color);

	hami2->draw(color, vec3(0, 1, 0));
	hami1->draw(color, vec3(1, 0, 0));

	glutSwapBuffers();
}

bool s, e, f;

void onKeyboard(const unsigned char key, int pX, int pY) {
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

void onKeyboardUp(const unsigned char key, int pX, int pY) {
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
