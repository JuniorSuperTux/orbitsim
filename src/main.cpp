#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/gl/gl.h>
#include <cinder/Json.h>

#include <random>

// Forces the use of Nvidia display card
#include <windows.h>
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

using namespace ci;
using namespace ci::app;
using namespace std::this_thread;
using namespace std::chrono_literals;

// constexpr float G = 6.67430;
constexpr float G = 6.67430e-11f;

std::random_device rd;
std::mt19937 gen(rd());

float random_float(float floor, float ceiling) {
	std::uniform_real_distribution<float> dis(floor, ceiling);
	return dis(gen);
}

float random_float(float ceiling) {
	std::uniform_real_distribution<float> dis(0, ceiling);
	return dis(gen);
}

class Planet {
public:
	vec3 position, speed;
	float mass /* kg */, radius /* m */;

	Planet() = default;
	Planet(float mass, float radius, vec3 position, vec3 speed)
		: position{ position }, speed{ speed }, mass{ mass }, radius{ radius }
	{}

	void draw();
	void init_render(gl::GlslProgRef& shader);
	void update_speed(std::vector<Planet>&);

	vec3 eval_next_pos(float timestep);
	void set_pos(vec3);

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Planet, position, speed, mass, radius)
private:
	gl::BatchRef m_shape;
};

void Planet::init_render(gl::GlslProgRef& shader) {
	auto sphere = geom::Sphere()
		.radius(this->radius)
		.subdivisions(100);
	m_shape = gl::Batch::create(sphere, shader);
}

void Planet::update_speed(std::vector<Planet>& planets) {
//	vec3 acceleration = vec3(0, 0, 0); // m/s^2
//	for (auto& planet : planets) {
//		if (&planet == this)
//			continue;
//		float x_diff = (this->position.x - planet.position.x);
//		float y_diff = (this->position.y - planet.position.y);
//		float z_diff = (this->position.z - planet.position.z);
//		float denominator = pow(pow(x_diff, 2) + pow(y_diff, 2) + pow(z_diff, 2), 3 / 2);
//		float numerator = G * (planet.mass);
//		acceleration.x += ((numerator * x_diff) / denominator);
//		acceleration.y += ((numerator * y_diff) / denominator);
//		acceleration.z += ((numerator * z_diff) / denominator);
//	}
//	this->speed += acceleration;
	vec3 acceleration = vec3(0, 0, 0); // m/s^2
	for (const auto& planet : planets) {
		if (&planet == this)
			continue;
		vec3 direction = planet.position - this->position;
		float distance = glm::length(direction);
		if (distance > 0) {
			direction = glm::normalize(direction);
			float force = G * (this->mass * planet.mass) / (distance * distance);
			acceleration += direction * (force / this->mass);
		}
	}
	this->speed += acceleration;
}

vec3 Planet::eval_next_pos(float timestep) {
	float new_x = this->position.x + ((this->speed.x) * timestep);
	float new_y = this->position.y + ((this->speed.y) * timestep);
	float new_z = this->position.z + ((this->speed.z) * timestep);
	return vec3(new_x, new_y, new_z);
}


void Planet::draw() {
	gl::pushModelMatrix();
	vec3 translation = this->position;
	gl::translate(translation);
	m_shape->draw();
	gl::popModelMatrix();
}

void Planet::set_pos(vec3 target) {
	this->position = target;
}

class BasicApp : public App {
public:
	void draw() override;
	void setup() override;

	void mouseDrag(MouseEvent event) override;
	void mouseDown(MouseEvent event) override;
	void mouseWheel(MouseEvent event) override;

	void initialize_from_file(std::string filename);
	void dump_to_file(std::string filename);

	std::vector<Planet> m_planets;
private:
	// Settings
	const float ANGLE_SCALAR = 0.01, SCROLL_SCALAR = 100;
	const float CAM_DIST = 400;
	const float TIMESTEP = 0.00001;

	void update_camera();
	int half_rounds = 0;
	int next_half_rounds = 0;

	ivec2 m_mouse_pos;
	float m_phi = 0, m_theta = 0;
	float m_cam_dist = CAM_DIST;
	CameraPersp m_cam;
};

void BasicApp::setup() {
//	setWindowSize(1080, 1080);
	console() << __argc << std::endl;
	if (__argc > 1) {
		console() << "Loading previous saved state " << __argv[1] << std::endl;
		this->initialize_from_file(__argv[1]);
	}
	else {
		getWindow()->getSignalClose().connect([this]() {
			auto t = time(nullptr);
			std::ostringstream oss;
			oss << std::put_time(localtime(&t), "%Y-%m-%d %H-%M-%S");
			oss << ".json";
			this->dump_to_file(oss.str());
		});
	}
	setFrameRate(144.0f);

	gl::enableDepth();
	auto lambert = gl::ShaderDef().lambert().color();
	gl::GlslProgRef shader = gl::getStockShader(lambert);

	for (int i = 0; i < 100; ++i) {
		m_planets.emplace_back(random_float(1000000000000000), random_float(5, 7),
			vec3(random_float(-25, 25), random_float(-25, 25), random_float(-25, 25)),
			vec3(random_float(-20, 20), random_float(-20, 20), random_float(-20, 20)));
	}
	for (auto& planet : m_planets)
		planet.init_render(shader);

	this->update_camera();
}

void BasicApp::draw() {
	gl::clear();
	this->update_camera();
	gl::setMatrices(m_cam);
	std::vector<vec3> new_positions(m_planets.size());
	for (size_t it = 0; it < m_planets.size(); ++it) {
		m_planets.at(it).draw();
		m_planets.at(it).update_speed(m_planets);
		new_positions.at(it) = m_planets.at(it).eval_next_pos(TIMESTEP);
	}
	for (size_t it = 0; it < m_planets.size(); ++it)
		m_planets.at(it).set_pos(new_positions.at(it));


	gl::setMatricesWindow(getWindowSize());
	TextBox m_tbox = TextBox()
		.alignment(TextBox::LEFT)
		.font(Font("Arial", 30))
		.size(vec2(300, 30))
		.text("INIT")
		.backgroundColor(ColorA(0.5f, 0.5f, 0.5f, 0.5f));
	std::ostringstream ss;
	ss << "phi: ";
	ss << roundf(m_phi * 100) / 100;
	ss << " theta: ";
	ss << roundf(m_theta * 100) / 100;
	m_tbox.setText(ss.str());
	gl::TextureRef m_tbox_texture = gl::Texture::create(m_tbox.render());

	gl::draw(m_tbox_texture, vec2(0, 0));

}


void BasicApp::update_camera() {
	if (bool((next_half_rounds - half_rounds) % 2))
		m_cam.setWorldUp(m_cam.getWorldUp() * vec3(1, -1, 1));
	m_cam.lookAt(vec3(
		m_cam_dist * sinf(m_phi) * cosf(m_theta),
		m_cam_dist * cosf(m_phi),
		m_cam_dist * sinf(m_phi) * sinf(m_theta)
	), vec3(0));
	half_rounds = next_half_rounds;
}

void BasicApp::mouseDrag(MouseEvent event) {
	ivec2 diff = event.getPos() - m_mouse_pos;
//  float theta = -atan2(diff.x, diff.y) + M_PI;
//  float phi = acosf(clamp(float(diff.y) / getWindowSize().y, -1.0f, 1.0f));
//	m_theta += (diff.x * ANGLE_SCALAR * (m_cam_dist / CAM_DIST));
	m_theta += (diff.x * ANGLE_SCALAR);
//	m_phi += (diff.y * ANGLE_SCALAR * (m_cam_dist / CAM_DIST));
	m_phi += (diff.y * ANGLE_SCALAR);

	next_half_rounds = int(m_phi / M_PI);
	m_mouse_pos = event.getPos();
}

void BasicApp::mouseDown(MouseEvent event) {
	m_mouse_pos = event.getPos();
}

void BasicApp::mouseWheel(MouseEvent event) {
	float diff = event.getWheelIncrement() * SCROLL_SCALAR;
	if (m_cam_dist - diff > 0)
		m_cam_dist -= diff;
}

void BasicApp::dump_to_file(std::string filename) {
	std::ofstream file(filename);
	Json planets_array = m_planets;
	file << std::setw(4) << planets_array;
}

void BasicApp::initialize_from_file(std::string filename) {
	std::ifstream file(filename);
	Json planets_array = Json::parse(file);
	m_planets = planets_array.template get<decltype(m_planets)>();
}

CINDER_APP(BasicApp, RendererGl)

void to_json(Json& target, const vec3& source) {
	target = Json{
		{ "x", source.x },
		{ "y", source.y },
		{ "z", source.z },
	};
}

void from_json(const Json& source, vec3& target) {
	source.at("x").get_to(target.x);
	source.at("y").get_to(target.y);
	source.at("z").get_to(target.z);
}
