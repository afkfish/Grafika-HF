//=============================================================================================
// Computer Graphics Sample Program: Ray-tracing-let
//=============================================================================================
#include "framework.h"

struct Material {
	vec3 ka, kd, ks;
	float  shininess;
	Material(const vec3 _kd, const vec3 _ks, const float _shininess) : ka(_kd * M_PI), kd(_kd), ks(_ks) { shininess = _shininess; }
};

struct Hit {
	float t;
	vec3 position, normal;
	Material * material = nullptr;
	Hit() { t = -1; }
};

struct Ray {
	vec3 start, dir;
	Ray(const vec3 _start, const vec3 _dir) { start = _start; dir = normalize(_dir); }
};

class Intersectable {
protected:
	Material * material = nullptr;
public:
	virtual Hit intersect(const Ray& ray) = 0;
};

struct Triangle : Intersectable {
	vec3 normal;
	vec3 point1, point2, point3;

	Triangle(const vec3& point1, const vec3& point2, const vec3& point3, Material* material) {
		this->point1 = point1;
		this->point2 = point2;
		this->point3 = point3;
		this->material = material;
		normal = normalize(cross(point2 - point1, point3 - point1));
	}

	Hit intersect(const Ray& ray) override {
		Hit hit;
		const float t = dot(point1 - ray.start, normal) / dot(ray.dir, normal);
		if (t < 0) return hit;
		const vec3 P = ray.start + ray.dir * t;

		vec3 C = cross(point2 - point1, P - point1);
		if (dot(C, normal) < 0) return hit;

		C = cross(point3 - point2, P - point2);
		if (dot(C, normal) < 0) return hit;

		C = cross(point1 - point3, P - point3);
		if (dot(C, normal) < 0) return hit;

		hit.t = t;
		hit.position = P;
		hit.normal = normal;
		hit.material = material;
		return hit;
	}
};

struct Square : Intersectable {
	vec3 normal;
	vec3 point1, point2, point3, point4;
	std::vector<Triangle> *triangles = new std::vector<Triangle>();

	Square(const vec3& point1, const vec3& point2, const vec3& point3, const vec3& point4, Material* material) {
		this->point1 = point1;
		this->point2 = point2;
		this->point3 = point3;
		this->point4 = point4;
		this->material = material;
		normal = normalize(cross(point2 - point1, point3 - point1));
		triangles->emplace_back(point1, point2, point3, material);
		triangles->emplace_back(point1, point3, point4, material);
	}

	Hit intersect(const Ray& ray) override {
		// check if the ray intersects with any of the triangles
		const Hit hit;
		for (auto &triangle : *triangles) {
			const Hit temp = triangle.intersect(ray);
			if (temp.t > 0 && (hit.t < 0 || temp.t < hit.t)) {
				return temp;
			}
		}
		return hit;
	}
};

struct Box : Intersectable {
	std::vector<Square> *squares = new std::vector<Square>();
	std::vector<vec3> points;

	explicit Box(Material* material) {
		points.emplace_back(-0.5, -0.5, -0.5); // bal also hatso 0
		points.emplace_back(-0.5, -0.5, 0.5); // bal also elso 1
		points.emplace_back(-0.5, 0.5, 0.5); // bal felso elso 2
		points.emplace_back(-0.5, 0.5, -0.5); // bal felso hatso 3
		points.emplace_back(0.5, -0.5, -0.5); // jobb also hatso 4
		points.emplace_back(0.5, -0.5, 0.5); // jobb also elso 5
		points.emplace_back(0.5, 0.5, -0.5); // jobb felso hatso 6
		points.emplace_back(0.5, 0.5, 0.5); // jobb felso elso 7
		squares->emplace_back(points[0], points[1], points[5], points[4], material);
		squares->emplace_back(points[0], points[3], points[6], points[4], material);
		squares->emplace_back(points[0], points[1], points[2], points[3], material);
		// squares.emplace_back(new Square(points[1], points[3], points[5], points[7], material));
		squares->emplace_back(points[3], points[6], points[7], points[2], material);
		// squares.emplace_back(new Square(points[4], points[5], points[6], points[7], material));
	}

	Hit intersect(const Ray& ray) override {
		const Hit hit;
		for (auto &square : *squares) {
			const Hit temp = square.intersect(ray);
			if (temp.t > 0 && (hit.t < 0 || temp.t < hit.t)) {
				return temp;
			}
		}
		return hit;
	}
};

struct Octahedron : Intersectable {
	std::vector<Triangle> *triangles = new std::vector<Triangle>();
	float size = 0.4;
	vec3 offset;

	explicit Octahedron(const vec3 _offset, Material* material) : offset(_offset) {
		triangles->emplace_back(size*(vec3(0, 0, 0.5)+offset), size*(vec3(0.5, 0, 0)+offset), size*(vec3(0, 0.5, 0)+offset), material);
		triangles->emplace_back(size*(vec3(0, 0, 0.5)+offset), size*(vec3(0, 0.5, 0)+offset), size*(vec3(-0.5, 0, 0)+offset), material);
		triangles->emplace_back(size*(vec3(0, 0, 0.5)+offset), size*(vec3(-0.5, 0, 0)+offset), size*(vec3(0, -0.5, 0)+offset), material);
		triangles->emplace_back(size*(vec3(0, 0, 0.5)+offset), size*(vec3(0, -0.5, 0)+offset), size*(vec3(0.5, 0, 0)+offset), material);
		triangles->emplace_back(size*(vec3(0, 0, -0.5)+offset), size*(vec3(0.5, 0, 0)+offset), size*(vec3(0, 0.5, 0)+offset), material);
		triangles->emplace_back(size*(vec3(0, 0, -0.5)+offset), size*(vec3(0, 0.5, 0)+offset), size*(vec3(-0.5, 0, 0)+offset), material);
		triangles->emplace_back(size*(vec3(0, 0, -0.5)+offset), size*(vec3(-0.5, 0, 0)+offset), size*(vec3(0, -0.5, 0)+offset), material);
		triangles->emplace_back(size*(vec3(0, 0, -0.5)+offset), size*(vec3(0, -0.5, 0)+offset), size*(vec3(0.5, 0, 0)+offset), material);
	}

	Hit intersect(const Ray& ray) override {
		Hit hit;
		for (auto &triangle : *triangles) {
			const Hit temp = triangle.intersect(ray);
			if (temp.t > 0 && (hit.t < 0 || temp.t < hit.t)) {
				hit = temp;
			}
		}
		return hit;
	}
};

struct Icosahedron: Intersectable {
	std::vector<Triangle> *triangles = new std::vector<Triangle>();
	std::vector<vec3> *a = new std::vector<vec3>();
	std::vector<vec3> *b = new std::vector<vec3>();
	std::vector<vec3> *c = new std::vector<vec3>();
	float size = 0.2;
	vec3 offset;

	explicit Icosahedron(const vec3 _offset, Material* material) : offset(_offset) {
		a->emplace_back(0.850651, 0, 0.525731); b->emplace_back(0.850651, 0, -0.525731); c->emplace_back(0.525731, 0.850651, 0);
		a->emplace_back(0.850651, 0, 0.525731); b->emplace_back(0.525731, -0.850651, 0); c->emplace_back(0.850651, 0, -0.525731);
		a->emplace_back(-0.850651, 0, -0.525731); b->emplace_back(-0.850651, 0, 0.525731); c->emplace_back(-0.525731, 0.850651, 0);
		a->emplace_back(-0.850651, 0, 0.525731); b->emplace_back(-0.850651, 0, -0.525731); c->emplace_back(-0.525731, -0.850651, 0);
		a->emplace_back(0.525731, 0.850651, 0); b->emplace_back(-0.525731, 0.850651, 0); c->emplace_back(0, 0.525731, 0.850651);
		a->emplace_back(-0.525731, 0.850651, 0); b->emplace_back(0.525731, 0.850651, 0); c->emplace_back(0, 0.525731, -0.850651);
		a->emplace_back(0, -0.525731, -0.850651); b->emplace_back(0, 0.525731, -0.850651); c->emplace_back(0.850651, 0, -0.525731);
		a->emplace_back(0, 0.525731, -0.850651); b->emplace_back(0, -0.525731, -0.850651); c->emplace_back(-0.850651, 0, -0.525731);
		a->emplace_back(0.525731, -0.850651, 0); b->emplace_back(-0.525731, -0.850651, 0); c->emplace_back(0, -0.525731, -0.850651);
		a->emplace_back(-0.525731, -0.850651, 0); b->emplace_back(0.525731, -0.850651, 0); c->emplace_back(0, -0.525731, 0.850651);
		a->emplace_back(0, 0.525731, 0.850651); b->emplace_back(0, -0.525731, 0.850651); c->emplace_back(0.850651, 0, 0.525731);
		a->emplace_back(0, -0.525731, 0.850651); b->emplace_back(0, 0.525731, 0.850651); c->emplace_back(-0.850651, 0, 0.525731);
		a->emplace_back(0.525731, 0.850651, 0); b->emplace_back(0.850651, 0, -0.525731); c->emplace_back(0, 0.525731, -0.850651);
		a->emplace_back(0.850651, 0, 0.525731); b->emplace_back(0.525731, 0.850651, 0); c->emplace_back(0, 0.525731, 0.850651);
		a->emplace_back(-0.850651, 0, -0.525731); b->emplace_back(-0.525731, 0.850651, 0); c->emplace_back(0, 0.525731, -0.850651);
		a->emplace_back(-0.525731, 0.850651, 0); b->emplace_back(-0.850651, 0, 0.525731); c->emplace_back(0, 0.525731, 0.850651);
		a->emplace_back(0.850651, 0, -0.525731); b->emplace_back(0.525731, -0.850651, 0); c->emplace_back(0, -0.525731, -0.850651);
		a->emplace_back(0.525731, -0.850651, 0); b->emplace_back(0.850651, 0, 0.525731); c->emplace_back(0, -0.525731, 0.850651);
		a->emplace_back(-0.850651, 0, -0.525731); b->emplace_back(0, -0.525731, -0.850651); c->emplace_back(-0.525731, -0.850651, 0);
		a->emplace_back(-0.850651, 0, 0.525731); b->emplace_back(-0.525731, -0.850651, 0); c->emplace_back(0, -0.525731, 0.850651);

		for (int i = 0; i < a->size(); ++i) {
			triangles->emplace_back(size*((*a)[i]+offset), size*((*b)[i]+offset), size*((*c)[i]+offset), material);
		}
	}

	Hit intersect(const Ray& ray) override {
		Hit hit;
		for (auto &triangle : *triangles) {
			const Hit temp = triangle.intersect(ray);
			if (temp.t > 0 && (hit.t < 0 || temp.t < hit.t)) {
				hit = temp;
			}
		}
		return hit;
	}
};

class Camera {
	vec3 eye, lookat, right, up;
public:
	void set(const vec3 _eye, const vec3 _lookat, const vec3 vup, const float fov) {
		eye = _eye;
		lookat = _lookat;
		const vec3 w = eye - lookat;
		const float focus = length(w);
		right = normalize(cross(vup, w)) * focus * tanf(fov / 2);
		up = normalize(cross(w, right)) * focus * tanf(fov / 2);
	}
	Ray getRay(const int X, const int Y) {
		vec3 dir = lookat + right * (2.0f * (X + 0.5f) / windowWidth - 1) + up * (2.0f * (Y + 0.5f) / windowHeight - 1) - eye;
		return {eye, dir};
	}
};

struct Light {
	vec3 position;
	vec3 Le;
	Light(const vec3 _position, const vec3 _Le) {
		position = _position;
		Le = _Le;
	}
};

struct Cone : Intersectable {
	vec3 center;
	vec3 normal;
	vec3 color;
	float alfa = 10 * M_PI/180, height = 0.15f;
	Material* material;
	Light *light;

	Cone(const vec3 _center, const vec3 _dir, const vec3 _color, Material* _material) : center(_center), normal(normalize(_dir)),
	                                                                                    color(_color), material(_material) {
		light = new Light(center + normal*0.001, color);
	}

	Hit intersect(const Ray& ray) override {
		Hit hit;
		const float a = powf(dot(normal, ray.dir), 2) - (dot(ray.dir, ray.dir) * powf(cosf(alfa), 2));
		const float b = 2 * dot(normal, ray.dir) * dot((ray.start - center), normal) - 2 * dot(ray.dir, ray.start - center) * powf(cosf(alfa), 2);
		const float c = powf(dot((ray.start - center), normal), 2) - dot(ray.start - center, ray.start - center) * powf(cosf(alfa), 2);
		const float D = b * b - 4 * a * c;
		const float t1 = (-b + sqrtf(D)) / (2 * a);
		const float t2 = (-b - sqrtf(D)) / (2 * a);
		vec3 r;
		const vec3 r1 = ray.start + ray.dir * t1;
		const vec3 r2 = ray.start + ray.dir * t2;
		if (D < 0) return hit;
		if (dot(r1-center, normal) < 0 || dot(r1-center, normal) > height) {
			if (dot(r2-center, normal) < 0 || dot(r2-center, normal) > height)
				return hit;
			hit.t = t2;
			r = r2;
		}
		else {
			if (dot(r2-center, normal) < 0 || dot(r2-center, normal) > height) {
				hit.t = t1;
				r = r1;
			}
			else {
				hit.t = fminf(t1, t2);
				if (hit.t == t1) {
					r = r1;
				} else {
					r = r2;
				}
			}
		}
		hit.position = ray.start + hit.t * ray.dir;
		hit.normal = 2 * dot((r-center), normal) * normal - 2 * (r - center) * cosf(alfa);
		hit.material = material;
		return hit;
	}

	void move(const vec3 pos, const vec3 surfaceNormal) {
		center = pos;
		normal = surfaceNormal;
		light->position = pos;
	}
};

constexpr float epsilon = 0.0001f;

class Scene {
	std::vector<Light *> lights;
	vec3 La;
public:
	Camera camera;
	void build() {
		const vec3 eye = vec3(1.5, 0, 1.5);
		const vec3 vup = vec3(0, 1, 0);
		const vec3 lookat = vec3(0, 0, 0);
		constexpr float fov = 45 * M_PI / 180;
		camera.set(eye, lookat, vup, fov);

		La = vec3(1.f, 1.f, 1.f);

		const vec3 kd(0.5f, 0.5f, 0.5f);
		const vec3 ks(2, 2, 2);
		auto * material = new Material(kd, ks, 5);
		objects.emplace_back(new Box(material));

		objects.emplace_back(new Octahedron(vec3(0, -0.5, 1), material));
		objects.emplace_back(new Icosahedron(vec3(2.5, -1, 1), material));

		// green cone
		Cone *gCone = new Cone(vec3(-0.49, 0, -0.49), vec3(1, 0, 1), vec3(0, 0.2, 0), material);
		lights.emplace_back(gCone->light);
		objects.emplace_back(gCone);
		cones.emplace_back(gCone);

		// red cone
		Cone *rCone = new Cone(vec3(-0.4, -0.4, -0.4), vec3(1, 0, 1), vec3(0.2, 0, 0), material);
		lights.emplace_back(rCone->light);
		objects.emplace_back(rCone);
		cones.emplace_back(rCone);

		// blue cone
		Cone *bCone = new Cone(vec3(0.5, 0, 0.5), vec3(-0.5, 0, -0.5), vec3(0, 0, 0.2), material);
		lights.emplace_back(bCone->light);
		objects.emplace_back(bCone);
		cones.emplace_back(bCone);
	}

	void render(std::vector<vec4>& image) {
		for (int Y = 0; Y < windowHeight; Y++) {
#pragma omp parallel for
			for (int X = 0; X < windowWidth; X++) {
				const vec3 color = trace(camera.getRay(X, Y));
				image[Y * windowWidth + X] = vec4(color.x, color.y, color.z, 1);
			}
		}
	}

	Hit firstIntersect(const Ray&ray) const {
		Hit bestHit;
		for (Intersectable * object : objects) {
			const Hit hit = object->intersect(ray); //  hit.t < 0 if no intersection
			if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t)) {
				bestHit = hit;
			}
		}
		if (dot(ray.dir, bestHit.normal) > 0) bestHit.normal = bestHit.normal * (-1);
		return bestHit;
	}

	bool shadowIntersect(const Ray&ray) const {	// for directional lights
		for (Intersectable * object : objects) if (object->intersect(ray).t > 0) return true;
		return false;
	}

	vec3 trace(const Ray&ray, int depth = 0) const {
		const Hit hit = firstIntersect(ray);
		if (hit.t < 0) return {0, 0, 0};
		const float theta = cosh(dot( ray.dir, hit.normal));
		vec3 Ka = (hit.material->ka / 2);
		vec3 outRadiance = 0.2f * (1 + theta) * La;
		for (const Light * light : lights) {
			vec3 lightDir = light->position - hit.position;
			Ray shadowRay(hit.position + hit.normal * epsilon, lightDir);
			const float cosTheta = dot(hit.normal, lightDir);
			Hit shadowHit = firstIntersect(shadowRay);
			if (cosTheta > 0 && (shadowHit.t < 0 || shadowHit.t > length(shadowHit.position - light->position))) {	// shadow computation
				outRadiance = outRadiance + light->Le * hit.material->kd * cosTheta;
				vec3 halfway = normalize(-ray.dir + lightDir);
				const float cosDelta = dot(hit.normal, halfway);
				if (cosDelta > 0) outRadiance = outRadiance + light->Le * hit.material->ks * powf(cosDelta, hit.material->shininess);
			}
		}
		return outRadiance;
	}

	std::vector<Cone *> cones;
	std::vector<Intersectable *> objects;
};

GPUProgram gpuProgram; // vertex and fragment shaders
Scene scene;

// vertex shader in GLSL
const char *vertexSource = R"(
	#version 330
    precision highp float;

	layout(location = 0) in vec2 cVertexPosition;	// Attrib Array 0
	out vec2 texcoord;

	void main() {
		texcoord = (cVertexPosition + vec2(1, 1))/2;							// -1,1 to 0,1
		gl_Position = vec4(cVertexPosition.x, cVertexPosition.y, 0, 1); 		// transform to clipping space
	}
)";

// fragment shader in GLSL
const char *fragmentSource = R"(
	#version 330
    precision highp float;

	uniform sampler2D textureUnit;
	in  vec2 texcoord;			// interpolated texture coordinates
	out vec4 fragmentColor;		// output that goes to the raster memory as told by glBindFragDataLocation

	void main() {
		fragmentColor = texture(textureUnit, texcoord);
	}
)";

class FullScreenTexturedQuad {
	unsigned int vao{};	// vertex array object id and texture id
	Texture texture;
public:
	FullScreenTexturedQuad(const int windowWidth, const int windowHeight, const std::vector<vec4>& image)
			: texture(windowWidth, windowHeight, image)
	{
		glGenVertexArrays(1, &vao);	// create 1 vertex array object
		glBindVertexArray(vao);		// make it active

		unsigned int vbo;		// vertex buffer objects
		glGenBuffers(1, &vbo);	// Generate 1 vertex buffer objects

		// vertex coordinates: vbo0 -> Attrib Array 0 -> vertexPosition of the vertex shader
		glBindBuffer(GL_ARRAY_BUFFER, vbo); // make it active, it is an array
		constexpr float vertexCoords[] = { -1, -1,  1, -1,  1, 1,  -1, 1 };	// two triangles forming a quad
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_STATIC_DRAW);	   // copy to that part of the memory which is not modified
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);     // stride and offset: it is tightly packed
	}

	void Draw() const {
		glBindVertexArray(vao);	// make the vao and its vbos active playing the role of the data source
		gpuProgram.setUniform(texture, "textureUnit");
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);	// draw two triangles forming a quad
	}
};

FullScreenTexturedQuad * fullScreenTexturedQuad;

// Initialization, create an OpenGL context
void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	scene.build();

	std::vector<vec4> image(windowWidth * windowHeight);
	const long timeStart = glutGet(GLUT_ELAPSED_TIME);
	scene.render(image);
	const long timeEnd = glutGet(GLUT_ELAPSED_TIME);
	printf("Rendering time: %ld milliseconds\n", timeEnd - timeStart);

	fullScreenTexturedQuad = new FullScreenTexturedQuad(windowWidth, windowHeight, image);

	gpuProgram.create(vertexSource, fragmentSource, "fragmentColor");
}

void onDisplay() {
	fullScreenTexturedQuad->Draw();
	glutSwapBuffers();									// exchange the two buffers
}

void onKeyboard(unsigned char key, int pX, int pY) {
}

void onKeyboardUp(unsigned char key, int pX, int pY) {

}

void onMouse(const int button, const int state, const int pX, const int pY) {
	if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

	const Hit hit = scene.firstIntersect(scene.camera.getRay(pX, windowHeight - pY));
	printf("x: %f, y: %f, z: %f\n", hit.position.x, hit.position.y, hit.position.z);

	float t = 50;
	Cone *c;
	for (Cone *cone : scene.cones) {
		// get the cone which has the closest center to the point in 3D
		const float t1 = length(hit.position - cone->center);
		if (t1 < t || t == 0) {
			t = t1;
			c = cone;
		}
	}
	c->move(hit.position, hit.normal);

	std::vector<vec4> image(windowWidth * windowHeight);
	scene.render(image);
	fullScreenTexturedQuad = new FullScreenTexturedQuad(windowWidth, windowHeight, image);
	glutPostRedisplay();
}

void onMouseMotion(int pX, int pY) {
}

void onIdle() {
}