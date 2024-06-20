#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/gl/gl.h>
#include <glm/fwd.hpp>

using namespace ci;
using namespace ci::app;
struct Pixel {
	ColorT<float> color;
	vec3 position = vec3();
	Pixel(ColorT<float>& color, vec3& position)
		: color{ color }, position{ position } {}
};

void exportToBinarySTL(const std::string& filename, const std::vector<Pixel>& data);
void exportToPLY(const std::string& filename, const std::vector<Pixel>& data);
