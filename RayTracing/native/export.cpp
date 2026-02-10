#include "rtweekend.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "color.h"
#include "camera.h"
#include <vector>
#include <cstring>
#include <memory>
#include <cstdint>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb_image_write.h"

using namespace std;

//from lecture - widoczne w ddl
#if defined(_WIN32) || defined(_WIN64)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

//part1 - creating a c-compatible abi to expose provided cpp core classes an utils

typedef void (*RenderCallback)(int samples, uint8_t* buffer);

//opaque handles a wiec csharp dostaje void*
using SceneHandle = void*;
using MaterialHandle = void*;
using ObjectHandle = void*;

//do rendera struct
//z camera.h
typedef struct CameraConfig {

	int    image_width;  // Rendered image width in pixel count
	int    samples_per_pixel;   // Count of random samples for each pixel
	int    max_depth;   // Maximum number of ray bounces into scene
	int    image_height;         // Rendered image height
	//dlatego ascpect_ratio = width/height
	//no i nie exposuje cpp layouts do kodu a wiec double nie point3
	double vfov;

	double lookfrom_x, lookfrom_y, lookfrom_z;
	double lookat_x, lookat_y, lookat_z;
	double vup_x, vup_y, vup_z;

	double defocus_angle;
	double focus_dist;
}CameraConfig;



//c compatible abi
//juz wczesniej mamy objects i scnee jako sharedptr a wiec tutaj rb to samo
extern "C" {

	//
	EXPORT SceneHandle CreateScene() {
		return new hittable_list();
	}

	EXPORT void DestroyScene(SceneHandle sc) {
		delete (hittable_list*)sc;
	}

	EXPORT void SceneClear(SceneHandle sc) {
		((hittable_list*)sc)->clear();
	}

	EXPORT MaterialHandle CreateMaterial(double r, double g, double b) {
		auto mt = make_shared<lambertian>(color(r, g, b));
		return new shared_ptr<material>(mt);
	}


	EXPORT void DestroyMaterial(MaterialHandle mt) {
		delete (shared_ptr<material>*)mt;
	}

	EXPORT ObjectHandle CreateSphere(double x, double y, double z, double radius, MaterialHandle mat) {
		auto* mt = (shared_ptr<material>*)mat;
		auto sph = make_shared<sphere>(point3(x, y, z), radius, *mt);
		return new shared_ptr<hittable>(sph);

	}

	EXPORT void DestroyObject(ObjectHandle obj) {
		delete (shared_ptr<hittable>*)obj;

	}
	//musimy jeszcze dodac do scenerii juz zrb obiekt 
	EXPORT void SceneAdd(SceneHandle sc, ObjectHandle obj)
	{
		auto* what = (hittable_list*)sc;
		auto* ebebe = (shared_ptr<hittable>*)obj;
		what->add(*ebebe);
	}

	//teraz musze zrb renderscene z parametrami cameraconfig i zrb local camera object 
	//przepisac parametry i wywolac render na tym i guesssss??

	EXPORT void RenderScene(CameraConfig c, SceneHandle scene, uint8_t* buff, RenderCallback call) {

		hittable_list* scena = (hittable_list*)scene;

		camera cam; //local camera

		cam.image_width = c.image_width;
		cam.samples_per_pixel = c.samples_per_pixel;
		cam.aspect_ratio = (double)c.image_width / (double)c.image_height;
		cam.vfov = c.vfov;
		//te glupie point3 z cpp
		cam.lookfrom = point3(c.lookfrom_x, c.lookfrom_y, c.lookfrom_z);
		cam.lookat = point3(c.lookat_x, c.lookat_y, c.lookat_z);
		cam.vup = vec3(c.vup_x, c.vup_y, c.vup_z);

		cam.defocus_angle = c.defocus_angle;
		cam.focus_dist = c.focus_dist;

		cam.render(*scena, buff, call);
	}

	EXPORT int SavePng(const char* path, int w, int h, const uint8_t* rgba)
	{
		//z STB_IMAGE_WRITE_IMPLEMENTATION
		return stbi_write_png(path, w, h, 4, rgba, w * 4);
	}

}
