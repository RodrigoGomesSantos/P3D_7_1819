#include "scene.hpp"

#include "plane.hpp"
#include "sphere.hpp"
#include "triangle.hpp"
#include "area_light.hpp"

#include <iostream>
#include <iterator>
#include <sstream>
#include <fstream>
#include <string>


p3d::scene::scene() {}

p3d::scene::~scene() {
	for (auto l : lights()) delete l;
	for (auto o : objs()) delete o;
}

p3d::camera p3d::scene::cam() const {
	return _cam;
}

math::vec3 p3d::scene::b_color() const {
	return _b_color;
}

std::vector<p3d::light*> p3d::scene::lights() const {
	return _lights;
}

std::vector<p3d::scene_obj*> p3d::scene::objs() const {
	return _objs;
}

p3d::hit p3d::scene::calculate_hit(const ray &ray) {
    if (_grid_on)
        return _grid.traverse(ray);

    hit closest;
    for (auto obj : _objs) {
        hit hit = obj->calculate_intersection(ray);
        if (hit.collided() && hit.distance() < closest.distance())
            closest = hit;
    }
    return closest;
}

/*
Taken from the interwebs
It returns a string array that each element contains a subset of the original string str up to the next separator sep segment
*/
std::vector<std::string> split(const std::string str, const std::string sep) {
	std::istringstream in(str);
	std::istream_iterator<std::string> begin(in), end;
	return std::vector<std::string>(begin, end);
}

bool p3d::scene::load_nff(const std::string name) {
	std::ifstream myfile(name);
	if (myfile.is_open()) {
		std::string line;
		std::vector<std::string> arr;

		material mat;
		bool mat_flag = false;

		math::vec3 from, at, up;
		float angle, hither;
		int resX, resY;

		bool b_v, b_at, b_up, b_from,
			b_angle, b_hither, b_resolution;

		b_v = b_at = b_up = b_from =
			b_angle = b_hither = b_resolution = false;

		while (std::getline(myfile, line)) {
			arr = split(line, " ");

			if (arr[0] == "v" && !b_v) { //viewing vectors and angles
				std::cout << "viewing vectors and angles" << std::endl;
				b_v = true;
			} else if (arr[0] == "from" && b_v) {
				std::cout << "from" << std::endl;
				from = math::vec3(std::atof(arr[1].c_str()),
					std::atof(arr[2].c_str()),
					std::atof(arr[3].c_str()));
				b_from = true;
			} else if (arr[0] == "at" && b_v) {
				std::cout << "at" << std::endl;
				at = math::vec3(std::atof(arr[1].c_str()),
					std::atof(arr[2].c_str()),
					std::atof(arr[3].c_str()));
				b_at = true;
			} else if (arr[0] == "up" && b_v) {
				std::cout << "up" << std::endl;
				up = math::vec3(std::atof(arr[1].c_str()),
					std::atof(arr[2].c_str()),
					std::atof(arr[3].c_str()));
				b_up = true;
			} else if (arr[0] == "angle" && b_v) {
				std::cout << "angle" << std::endl;
				angle = std::atof(arr[1].c_str());
				b_angle = true;
			} else if (arr[0] == "hither" && b_v) {
				std::cout << "hither" << std::endl;
				hither = std::atof(arr[1].c_str());
				b_hither = true;
			} else if (arr[0] == "resolution" && b_v) {
				std::cout << "resolution" << std::endl;
				resX = std::atoi(arr[1].c_str());
				resY = std::atoi(arr[2].c_str());
				b_resolution = true;
			} else if (arr[0] == "b") { //background color
				std::cout << "background color" << std::endl;
				_b_color = math::vec3(std::atof(arr[1].c_str()),
					std::atof(arr[2].c_str()),
					std::atof(arr[3].c_str()));
			} else if (arr[0] == "l") { //positional light location
				std::cout << "creating positional light" << std::endl;
				// position
				math::vec3 pos = math::vec3(std::atof(arr[1].c_str()),
					std::atof(arr[2].c_str()),
					std::atof(arr[3].c_str()));
				// color
				math::vec3 col = math::vec3(std::atof(arr[4].c_str()),
					std::atof(arr[5].c_str()),
					std::atof(arr[6].c_str()));

				std::cout << "adding light to lights vector" << std::endl;
				light *l = new p3d::light(pos, col);
				_lights.push_back(l);
			} else if (arr[0] == "al") { //area light location
				std::cout << "creating area light" << std::endl;
				// position
				math::vec3 pos = math::vec3(std::atof(arr[1].c_str()),
					std::atof(arr[2].c_str()),
					std::atof(arr[3].c_str()));
				// a
				math::vec3 a = math::vec3(std::atof(arr[4].c_str()),
					std::atof(arr[5].c_str()),
					std::atof(arr[6].c_str()));			
				// b
				math::vec3 b = math::vec3(std::atof(arr[7].c_str()),
					std::atof(arr[8].c_str()),
					std::atof(arr[9].c_str()));
				// color
				math::vec3 color = math::vec3(std::atof(arr[10].c_str()),
					std::atof(arr[11].c_str()),
					std::atof(arr[12].c_str()));

				std::cout << "adding area light to lights vector" << std::endl;
				p3d::area_light *al = new p3d::area_light(pos, a, b, color);
				_lights.push_back(al);
			} else if (arr[0] == "f") { //object material properties
				std::cout << "object material properties" << std::endl;
				if (arr.size() == 9) {
					math::vec3 color = math::vec3(std::atof(arr[1].c_str()),
						std::atof(arr[2].c_str()),
						std::atof(arr[3].c_str()));

					mat = material(color, std::atof(arr[4].c_str()),
						std::atof(arr[5].c_str()),
						std::atof(arr[6].c_str()),
						std::atof(arr[7].c_str()),
						std::atof(arr[8].c_str()));
					mat_flag = true;
				} else {
					std::cout <<
						"material doesn\'t have the right amount of atributes to be set"
						<< std::endl;
				}
			} else if (arr[0] == "c" && mat_flag) { //cone or cylinder primitive
				std::cout << "not implemented" << std::endl;
			} else if (arr[0] == "s" && mat_flag) { //sphere primitive
				std::cout << "creating sphere" << std::endl;
				math::vec3 pos(std::atof(arr[1].c_str()),
					std::atof(arr[2].c_str()),
					std::atof(arr[3].c_str()));

				float rad = std::atof(arr[4].c_str());
				sphere *sphe = new sphere(pos, rad, mat);

				std::cout << "adding sphere to scene objects" << std::endl;
				_objs.push_back(sphe);
			} else if (arr[0] == "p" && mat_flag) { //polygon primitive
                int vert = std::atoi(arr[1].c_str());
                std::vector<math::vec3> vertices;
                for (int i = vert; i > 0; i--) {
                    std::getline(myfile, line);
                    arr = split(line, " ");
                    math::vec3 v(std::atof(arr[0].c_str()),
                        std::atof(arr[1].c_str()),
                        std::atof(arr[2].c_str()));
                    vertices.push_back(v);
                }
                if (vert == 3) {
                    std::cout << "creating triangle" << std::endl;
                    triangle *tri = new triangle(vertices[0],
                        vertices[1], vertices[2], mat);
                    _objs.push_back(tri);
                } else
                    std::cout << "not implemented" << std::endl;
			} else if (arr[0] == "pp" && mat_flag) { //polygonal patch primitive
				std::cout << "not implemented" << std::endl;
			} else if (arr[0] == "pl" && mat_flag) { //plane
				std::cout << "creating plane" << std::endl;
				math::vec3 a(std::atof(arr[1].c_str()),
					std::atof(arr[2].c_str()),
					std::atof(arr[3].c_str()));
				math::vec3 b(std::atof(arr[4].c_str()),
					std::atof(arr[5].c_str()),
					std::atof(arr[6].c_str()));
				math::vec3 c(std::atof(arr[7].c_str()),
					std::atof(arr[8].c_str()),
					std::atof(arr[9].c_str()));

				plane *pl = new plane(a, b, c, mat);

				std::cout << "adding plane to scene objects" << std::endl;
				_objs.push_back(pl);
			}

			//closes the viewing vectors and angles section and creates the camera
			if (b_v && b_from && b_at && b_up && b_angle && b_hither && b_resolution) {
				_cam = camera(from, at, up, hither, angle, resX, resY);

				b_v = false;
				b_from = false;
				b_at = false;
				b_up = false;
				b_angle = false;
				b_hither = false;
				b_resolution = false;
			}

		}
		myfile.close();
	} else {
		std::cout << "Unable to open file" << std::endl;
		return false;
	}
	return true;
}

void p3d::scene::setup_grid() {
    _grid.setup_cells(_objs);
    _grid_on = true;
}


