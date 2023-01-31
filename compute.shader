#version 430

layout(local_size_x = 8, local_size_y = 8) in;
layout(rgba32f, binding = 0) uniform image2D img_out;
layout(std430, binding = 1) buffer
buffer_InPalette
{
	vec4 RGB_Palette[];
};

struct PayLoad {
	float t;
	vec3 RayColor;
	vec3 N;
};

struct Ray {
	vec3 O;
	vec3 d;

	float tmin, tmax;
};


struct Sphere {
	vec3 C;
	float R;
	vec3 color;
};
uniform vec3 eye, cameraU, cameraV, w;
uniform vec2 dim; //(width, height) width  = aspect x height , height = 2x tan(fov) Calculated outside 
uniform vec2 resolution; // (cols, rows)
PayLoad payload;

vec3 pointOnRay(Ray ray, float t) {
	return ray.O + t * ray.d;
}

Ray generateRay(vec2 pixel) {
	Ray returnRay;
	// Origin of the ray is the eye for now
	returnRay.O = eye;
	// Direction is the fun part
	vec2 NDC = 2. * pixel / resolution - 1.;
	returnRay.d = normalize( - w
				  + NDC.y * cameraV
				  + NDC.x * cameraU);
	returnRay.tmin = 0.;
	returnRay.tmax = 1000000.;
	return returnRay;
}

bool intersectSphere(Ray ray, Sphere obj) {
	float a = dot(ray.d, ray.d);
	vec3 o_c = ray.O - obj.C;
	float b = 2.0 * dot(ray.d, o_c);
	float c = dot(o_c, o_c) - (obj.R * obj.R);
	float disc = b * b - 4.0 * a * c;
	if (disc < 0.0)
		return false;

	disc = sqrt(disc);
	float t = (-b - disc) / (2.0 * a);
	if (t <= ray.tmin)
		t = (-b + disc) / (2.0 * a);
	if (t <= ray.tmin || t >= ray.tmax)
		return false;
	
	payload.t = t;
	return true;
	// t is the ray parameter of the intersection, should do somethign with that
}

bool traverseScene(Ray ray) {
	bool intersection = false;
	// For now we will define the objects just hardcoded
	Sphere scene[2];
	scene[0].C = vec3(1, 0, -1);
	scene[0].R = 1.;
	scene[0].color = vec3(0, 1, 0);
	scene[1].C = vec3(-1, 0, -1);
	scene[1].R = 1.;
	scene[1].color = vec3(1, 0, 0);

	for (int i = 0; i < scene.length(); i++) {
		if (intersectSphere(ray, scene[i])) {
			intersection = true;
			ray.tmax = payload.t;
			payload.RayColor = scene[i].color;
			payload.N = (normalize(pointOnRay(ray, ray.tmax) - scene[i].C));
		}
	}
	return intersection;
}

vec3 miss(PayLoad payload) {
	vec2 vector2 = vec2(gl_GlobalInvocationID.xy) / vec2(resolution);
	return vec3(vector2.x,vector2.y,0);
}

vec3 closestHit(Ray ray, PayLoad payload) {
	vec3 L = vec3(0, 0, 2);
	return payload.RayColor * clamp(dot(payload.N, normalize(L-pointOnRay(ray,payload.t))), 0., 1.);
}

vec3 traceRay(in Ray ray) {

	if (traverseScene(ray))
		return closestHit(ray, payload);
	else
		return miss(payload);
}

vec3 pixelColor(vec2 pixel) {
	Ray ray = generateRay(pixel);
	return traceRay(ray);
}


void main() {
	vec2 pixel_xy = gl_GlobalInvocationID.xy; // pixel coordinates
	vec4 color = vec4(0, 0, 0, 1);
	color.rgb = pixelColor(pixel_xy);

	imageStore(img_out, ivec2(pixel_xy), color); // stores the pixel, at its location and color based on previous
}