#include <orbitsim/trajectory.hpp>
#include <fstream>

// void exportToBinarySTL(const std::string& filename, const std::vector<Pixel>& data) {
// 	std::ofstream out(filename, std::ios::binary);
// 
// 	if (!out) {
// 		std::cerr << "Error opening file for writing: " << filename << std::endl;
// 		return;
// 	}
// 
// 	// Write 80-byte header
// 	char header[80] = {};
// 	std::strncpy(header, "Binary STL with Color", 80);
// 	out.write(header, 80);
// 
// 	// Each point will be treated as an individual 'triangle' for simplicity, even though it's just a point
// 	uint32_t numTriangles = data.size(); // Treat each point as a 'triangle'
// 	out.write(reinterpret_cast<const char*>(&numTriangles), sizeof(numTriangles));
// 
// 	for (const auto& pixel : data) {
// 		// Dummy normal vector (since it's not really a surface)
// 		float normal[3] = { 0.0f, 0.0f, 0.0f };
// 		out.write(reinterpret_cast<const char*>(normal), sizeof(normal));
// 
// 		// Write the same position three times to fit the STL triangle format
// 		for (int j = 0; j < 3; ++j) {
// 			float vertexArray[3] = { pixel.position.x, pixel.position.y, pixel.position.z };
// 			out.write(reinterpret_cast<const char*>(vertexArray), sizeof(vertexArray));
// 
// 		//	// Write color (non-standard extension, might not be supported by all STL viewers)
// 		//	uint16_t color = 0;
// 		//	const Color& col = pixel.color;
// 		//	color |= (uint16_t(col.r * 31) & 0x1F) << 10; // Red
// 		//	color |= (uint16_t(col.g * 31) & 0x1F) << 5;  // Green
// 		//	color |= (uint16_t(col.b * 31) & 0x1F);       // Blue
// 		//	color |= 0x8000; // Set the color bit
// 		//	out.write(reinterpret_cast<const char*>(&color), sizeof(color));
// 		}
// 	}


// 
// 	out.close();
// }

// void exportToBinarySTL(const std::string& filename, const std::vector<Pixel>& data) {
// 	std::ofstream out(filename, std::ios::binary);
// 
// 	if (!out) {
// 		std::cerr << "Error opening file for writing: " << filename << std::endl;
// 		return;
// 	}
// 
// 	// Write 80-byte header
// 	char header[80] = {};
// 	std::strncpy(header, "Binary STL", sizeof(header));
// 	out.write(header, sizeof(header));
// 
// 	// Calculate the number of triangles
// 	uint32_t numTriangles = data.size() / 3;
// 	out.write(reinterpret_cast<const char*>(&numTriangles), sizeof(numTriangles));
// 
// 	for (size_t i = 0; i < data.size(); i += 3) {
// 		if (i + 2 >= data.size()) break; // Ensure we have a complete triangle
// 
// 		// Calculate a dummy normal vector (pointing in the z-direction)
// 		float normal[3] = { 0.0f, 0.0f, 1.0f };
// 		out.write(reinterpret_cast<const char*>(normal), sizeof(normal));
// 
// 		// Write the vertices
// 		for (int j = 0; j < 3; ++j) {
// 			const auto& pixel = data[i + j];
// 			float vertexArray[3] = { pixel.position.x, pixel.position.y, pixel.position.z };
// 			out.write(reinterpret_cast<const char*>(vertexArray), sizeof(vertexArray));
// 		}
// 
// 		// Write the attribute byte count (set to 0 since we don't use attributes)
// 		uint16_t attributeByteCount = 0;
// 		out.write(reinterpret_cast<const char*>(&attributeByteCount), sizeof(attributeByteCount));
// 	}
// 
// 	out.close();
// }

void exportToPLY(const std::string& filename, const std::vector<Pixel>& data) {
	std::ofstream out(filename, std::ios::binary);

	if (!out) {
		std::cerr << "Error opening file for writing: " << filename << std::endl;
		return;
	}

	// Write PLY header (ascii format)
	out << "ply\n";
	out << "format ascii 1.0\n";
	out << "element vertex " << data.size() << "\n";
	out << "property float x\n";
	out << "property float y\n";
	out << "property float z\n";
//	out << "property uchar red\n";
//	out << "property uchar green\n";
//	out << "property uchar blue\n";
	out << "end_header\n";

	// Prepare string stream for efficient writing (optional)
	std::stringstream dataString;

	// Write vertex data
	for (const auto& pixel : data) {
		int red = std::max(0, std::min(255, static_cast<int>(std::round(pixel.color.r * 255))));
		int green = std::max(0, std::min(255, static_cast<int>(std::round(pixel.color.g * 255))));
		int blue = std::max(0, std::min(255, static_cast<int>(std::round(pixel.color.b * 255))));

		// Use string stream for efficient writing (optional)
		dataString << pixel.position.x << " " << pixel.position.y << " " << pixel.position.z << " "
			<< red << " " << green << " " << blue << "\r\n";

		// Alternatively, write directly to file (less efficient)
		// out << pixel.position.x << " " << pixel.position.y << " " << pixel.position.z << " "
		//    << red << " " << green << " " << blue << "\n";
	}

	// Write data string to file (optional)
	out << dataString.str();

	out.close();
}