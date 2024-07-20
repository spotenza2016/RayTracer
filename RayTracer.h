#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
using namespace std;
#pragma once
class RayTracer {
    // Class to hold color info about a material
    struct ColorPack {
        vector<unsigned char> ambientConstant;
        vector<unsigned char> diffuseConstant;
        vector<unsigned char> specularConstant;
        float phongExponent = 1.0f;
        bool reflect = false;
        ColorPack() = default;
        ColorPack(vector<unsigned char> ambientConstant, vector<unsigned char>
        diffuseConstant, vector<unsigned char> specularConstant, float phongExponent, bool
                  reflect = false);
    };
    // Abstract object class
    struct Object {
        ColorPack color;
        Object(ColorPack color);
        virtual ~Object() = default;
        // All objects check for intersection and can tell you their normal vector
        virtual float intersection(vector<float> p, vector<float> d) = 0;
        virtual vector<float> getNormal(const vector<float>& x) = 0;
    };
    // Sphere Class
    struct Sphere : public Object {
        vector<float> center;
        float radius;
        Sphere(vector<float> center, float radius, ColorPack color);
        float intersection(vector<float> p, vector<float> d) override;
        vector<float> getNormal(const vector<float>& x) override;
    };
    // Light Object Class
    struct LightObj : public Sphere {
        LightObj(vector<float> center, float radius, ColorPack color);
    };
    // Plane Class
    struct Plane : public Object {
        vector<float> a;
        vector<float> b;
        vector<float> c;
        Plane(vector<float> point1, vector<float> point2, vector<float> point3,
              ColorPack color);
        float intersection(vector<float> p, vector<float> d) override;
        vector<float> getNormal(const vector<float>& x) override;
    };
    // Triangle Class
    struct Triangle : public Object {
        vector<float> a;
        vector<float> b;
        vector<float> c;
        Triangle(vector<float> point1, vector<float> point2, vector<float> point3,
                 ColorPack color);
        float intersection(vector<float> p, vector<float> d) override;
        vector<float> getNormal(const vector<float> &x) override;
    };
    // Light Class
    struct Light {
        vector<float> location;
        float intensity;
        Light(vector<float> location, float intensity);
    };
    vector<float> findColor(vector<float> p, vector<float> d, int num, int limit);
public:
    // Saves an image to a ppm file (chose ppm because it's easy to write to)
    void takePicture(string fileName, unsigned char* image);
    // Functions for Vector Math
    static vector<float> addVec(const vector<float>& a, const vector<float>&
    b);
    static vector<float> scalarVec(float scalar, const vector<float>& a);
    static float dotVec(const vector<float>& a, const vector<float>& b);
    static vector<float> crossVec(const vector<float>& a, const vector<float>&
    b);
    static float vecMag(const vector<float>& a);
    static vector<float> normalizeVec(const vector<float>& a);
    static vector<float> multiplyVec(const vector<float>& a, const
    vector<float>& b);
    static vector<float> scaleColor(const vector<unsigned char>& a);
    // For transforming vectors
    static vector<float> transformVector(vector<float> vec, float pitch, float
    yaw, float roll);
    unsigned char* produceImage(vector<float> camera, vector<float> lookAtVec,
                                vector<float> upVec);
    ~RayTracer();
    unsigned char* image = nullptr;
    // Run Time Settings
    float distanceAwayConstant = 0.1f;
    float ambientIntensity = 0.5f;
    bool orthogonal = true;
    bool lightVisualization = false;
    int imgSizeX = 256;
    int imgSizeY = 256;
    float projectionDistance = 144.0f;
    // Object List
    vector<Object*> objects = {new Sphere({125, 50, -150}, 50, {{255, 128,
                                                                 255}, {255, 128, 255}, {255, 255, 255}, 16}),
                               new Triangle({175, 5, -75}, {200, 55, -100},
                                            {225, 5, -75}, {{128, 255, 255}, {128, 255, 255}, {255, 255, 255}, 16}),
                               new Triangle({225, 5, -75}, {200, 55, -100},
                                            {225, 5, -125}, {{128, 255, 255}, {128, 255, 255}, {255, 255, 255}, 16}),
                               new Triangle({225, 5, -125}, {200, 55, -100},
                                            {175, 5, -125}, {{128, 255, 255}, {128, 255, 255}, {255, 255, 255}, 16}),
                               new Triangle({175, 5, -125}, {200, 55, -100},
                                            {175, 5, -75}, {{128, 255, 255}, {128, 255, 255}, {255, 255, 255}, 16}),
                               new Triangle({225, 5, -125}, {175, 5, -125},
                                            {175, 5, -75}, {{128, 255, 255}, {128, 255, 255}, {255, 255, 255}, 16}),
                               new Triangle({225, 5, -75}, {225, 5, -125},
                                            {175, 5, -75}, {{128, 255, 255}, {128, 255, 255}, {255, 255, 255}, 16}),
                               new Sphere({128, 41, -62}, 20, {{255, 128, 128},
                                                               {255, 128, 128}, {255, 255, 255}, 16}),
                               new Plane({0, 0, 0}, {1, 0, 0}, {0, 0, 1},
                                         {{255, 255, 0}, {255, 255, 0}, {255, 255, 255}, 16, true}),
                               new LightObj({100, 100, -50}, 5, {{255, 255, 0},
                                                                 {255, 255, 0}, {255, 255, 255}, 16}),
                               new LightObj({231, 63, -127}, 5, {{255, 255, 0},
                                                                 {255, 255, 0}, {255, 255, 255}, 16})};
    vector<unsigned char> backgroundColor = {0, 0, 0};
    // Lights
    vector<Light*> lights = {new Light({100, 100, -50}, 0.2f),
                             new Light({231, 63, -127}, 0.2f)};
};