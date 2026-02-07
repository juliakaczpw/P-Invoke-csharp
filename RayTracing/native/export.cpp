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
		delete(shared_ptr<material>*)mt;
	}

	EXPORT ObjectHandle CreateSphere(double x, double y, double y, double radius, MaterialHandle mat) {
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
}
