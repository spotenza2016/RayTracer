#include "RayTracer.h"
vector<float> RayTracer::addVec(const vector<float> &a, const vector<float> &b) {
    return {a[0] + b[0], a[1] + b[1], a[2] + b[2]};
}
vector<float> RayTracer::scalarVec(float scalar, const vector<float> &a) {
    return {a[0] * scalar, a[1] * scalar, a[2] * scalar};
}
float RayTracer::dotVec(const vector<float> &a, const vector<float> &b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
vector<float> RayTracer::crossVec(const vector<float> &a, const vector<float> &b) {
    return {a[1]*b[2] - a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]};
}
float RayTracer::vecMag(const vector<float> &a) {
    return sqrt(pow(a[0], 2) + pow(a[1], 2) + pow(a[2], 2));
}
vector<float> RayTracer::normalizeVec(const vector<float> &a) {
    float mag = vecMag(a);
    return {a[0] / mag, a[1] / mag, a[2] / mag};
}
vector<float> RayTracer::multiplyVec(const vector<float> &a, const vector<float>
&b) {
    return {a[0] * b[0], a[1] * b[1], a[2] * b[2]};
}
vector<float> RayTracer::scaleColor(const vector<unsigned char> &a) {
    return {a[0] / 255.0f, a[1] / 255.0f, a[2] / 255.0f};
}
vector<float> RayTracer::findColor(vector<float> p, vector<float> d, int num, int
limit) {
    // Iterate through all objects, finding closest one the ray hits
    float minT = -1;
    Object* hitObject;
    for (int k = 0; k < objects.size(); k++) {
        if (dynamic_cast<LightObj*>(objects.at(k)) != nullptr) {
            if (!lightVisualization || num > 0) {
                continue;
            }
        }
        float t = objects.at(k)->intersection(p, d);
        if (t == -1) {
            continue;
        }
        if (minT == -1 || t < minT) {
            minT = t;
            hitObject = objects.at(k);
        }
    }
    // If an object was found
    if (minT != -1) {
        // Hit location
        vector<float> x = addVec(p, scalarVec(minT, d));
        // Color and normal of surface
        vector<float> ambientColor = scaleColor(hitObject->color.ambientConstant);
        vector<float> diffuseColor = scaleColor(hitObject->color.diffuseConstant);
        vector<float> specularColor = scaleColor(hitObject->color.specularConstant);
        vector<float> normal = hitObject->getNormal(x);
        // For lights, just give full ambient color
        if (dynamic_cast<LightObj*>(hitObject) != nullptr) {
            return ambientColor;
        }
        vector<float> totalLight = {0, 0, 0};
        // Ambient Lighting
        // Adds Ambient Intensity * Ambient Color Constant
        totalLight = scalarVec(ambientIntensity, ambientColor);
        // Vector from surface to current light
        for (int lightNum = 0; lightNum < lights.size(); lightNum++) {
            // Vector from surface to light
            vector<float> lightVec = normalizeVec(addVec(lights.at(lightNum)->location, scalarVec(-1, x)));
            vector<float> rayPoint = addVec(x, scalarVec(distanceAwayConstant,
                                                         lightVec));
            float lightT = vecMag(addVec(lights.at(lightNum)->location, scalarVec(-
                                                                                          1, rayPoint)));
            // Ray trace to find any objects blocking light
            bool inShadow = false;
            for (int k = 0; k < objects.size(); k++) {
                if (dynamic_cast<LightObj*>(objects.at(k)) != nullptr) {
                    continue;
                }
                float foundT = objects.at(k)->intersection(rayPoint, lightVec);
                // If found and not past the light
                if (foundT != -1 && foundT < lightT) {
                    inShadow = true;
                    break;
                }
            }
            // If not in shadow, add contributing light
            if (!inShadow) {
                // Diffuse
                // Adds Intensity * max(0, n dot l) * Diffuse Constant
                totalLight = addVec(scalarVec(lights.at(lightNum)->intensity,scalarVec(max(0.0f, dotVec(normal, lightVec)), diffuseColor)),
                                    totalLight);
                // Specular
                vector<float> eyeVec = normalizeVec(addVec(p, scalarVec(-1, x)));
                vector<float> h = normalizeVec(addVec(eyeVec, lightVec));
                // Adds Intensity * max(0, (n dot h)^p) * Specular Constant
                totalLight = addVec(scalarVec(lights.at(lightNum)->intensity,scalarVec(pow(max(0.0f, dotVec(normal, h)), hitObject->color.phongExponent), specularColor)), totalLight);
            }
            // Reflective
            if (hitObject->color.reflect && num < limit) {
                vector<float> r = addVec(d, scalarVec(-2 * dotVec(d, normal),
                                                      normal));
                vector<float> reflectRayPoint = addVec(x,
                                                       scalarVec(distanceAwayConstant, r));
                // Adds Intensity * findColor(reflectRayPoint, r, num + 1, limit) * specularColor
                        totalLight = addVec(scalarVec(lights.at(lightNum)->intensity,multiplyVec(findColor(reflectRayPoint, r, num + 1, limit),specularColor)), totalLight);
            }
        }
        // Clamp any values higher than 1 to 1
        return {min(totalLight[0], 1.0f), min(totalLight[1], 1.0f),
                min(totalLight[2], 1.0f)};
    }
    // Return background color for no objects found
    return scaleColor(backgroundColor);
}
unsigned char * RayTracer::produceImage(vector<float> camera, vector<float>
lookAtVec, vector<float> upVec) {
    if (image != nullptr) {
        delete image;
    }
    image = new unsigned char[imgSizeX * imgSizeY * 3];
    // Define Camera Basis
    vector<float> w = normalizeVec(scalarVec(-1, lookAtVec));
    vector<float> u = normalizeVec(crossVec(upVec, w));
    vector<float> v = normalizeVec(crossVec(w, u));
    // Loop for every pixel in the image
    for (int i = 0; i < imgSizeX; i++) {
        for (int j = 0; j < imgSizeY; j++) {
            // Adjusts u and v to be -1 to 1
            float uScale = 2.0f * (((float) i + 0.5f) / (float) imgSizeX) - 1.0f;
            float vScale = 2.0f * (((float) j + 0.5f) / (float) imgSizeY) - 1.0f;
            // p = camera
            vector<float> p = camera;
            if (orthogonal) {
                // p = camera + uScale * u + vScale * v
                p = addVec(scalarVec(uScale, scalarVec((float) imgSizeX / 2.0f,
                                                       u)), p);
                p = addVec(scalarVec(vScale, scalarVec((float) imgSizeY / 2.0f,
                                                       v)), p);
            }
            vector<float> d;
            if (orthogonal) {
                // d = -w
                d = scalarVec(-1, w);
            }
            else {
                // d = -w * projectionDistance + uScale * u + vScale * v
                d = scalarVec(-1 * projectionDistance, w);
                d = addVec(scalarVec(uScale, scalarVec((float) imgSizeX / 2.0f,
                                                       u)), d);
                d = addVec(scalarVec(vScale, scalarVec((float) imgSizeY / 2.0f,
                                                       v)), d);
                d = normalizeVec(d);
            }
            // Get the color
            vector<float> scaleColor = findColor(p, d, 0, 1);
            vector<unsigned char> color;
            for (int k = 0; k < scaleColor.size(); k++) {
                color.push_back(scaleColor.at(k)*255);
            }
            // Set pixel value
            int index = (j * imgSizeX + i) * 3;
            image[index] = color[0];
            image[index + 1] = color[1];
            image[index + 2] = color[2];
        }
    }
    return image;
}
RayTracer::~RayTracer() {
    for (int i = 0; i < objects.size(); i++) {
        delete objects.at(i);
    }
    for (int i = 0; i < lights.size(); i++) {
        delete lights.at(i);
    }
    if (image != nullptr) {
        delete image;
    }
}
vector<float> RayTracer::transformVector(vector<float> vec, float pitch, float yaw,
                                         float roll) {
    // Roll is disabled but figured I'd include the code anyway
    float a = 0.0f * (float)M_PI / 180.0f;
    float b = pitch * (float)M_PI / 180.0f;
    float c = yaw * (float)M_PI / 180.0f;
    // Roll Matrix
    vector<float> rolledVec = {cos(a) * vec[0] + sin(a) * vec[1], -1 * sin(a) *
                                                                  vec[0] + cos(a) * vec[1], vec[2]};
    vec = rolledVec;
    // Pitch Matrix
    vector<float> pitchedVec = {vec[0], cos(b) * vec[1] + sin(b) * vec[2], -1 *
                                                                           sin(b) * vec[1] + cos(b) * vec[2]};
    vec = pitchedVec;
    // Yaw Matrix
    vector<float> yawedVec = {cos(c) * vec[0] + -1 * sin(c) * vec[2], vec[1],
                              sin(c) * vec[0] + cos(c) * vec[2]};
    vec = yawedVec;
    return vec;
}
void RayTracer::takePicture(string fileName, unsigned char* image) {
    ofstream file(fileName, ios::out | ios::binary | ios::trunc);
    // PPM Magic Number
    char magicNumber[3] = {'P', '6', '\n'};
    file.write(magicNumber, 3);
    // Width
    string widthString = to_string(imgSizeX);
    file.write(widthString.c_str(), widthString.size());
    char newLine = '\n';
    // Height
    file.write(&newLine, sizeof(newLine));
    string heightString = to_string(imgSizeY);
    file.write(heightString.c_str(), heightString.size());
    file.write(&newLine, sizeof(newLine));
    // Max Color
    char maxColor[4] = {'2', '5', '5', '\n'};
    file.write(maxColor, 4);
    // Write the pixels
    for (int i = imgSizeY - 1; i >= 0; i--) {
        for (int j = 0; j < imgSizeX; j++) {
            int index = (i * imgSizeX + j) * 3;
            unsigned char currChar[3] = {image[index], image[index + 1],
                                         image[index + 2]};
            file.write((char*)currChar, 3);
        }
    }
}
RayTracer::ColorPack::ColorPack(vector<unsigned char> ambientConstant,
                                vector<unsigned char> diffuseConstant,
                                vector<unsigned char> specularConstant, float
                                phongExponent, bool reflect) {
    this->ambientConstant = ambientConstant;
    this->diffuseConstant = diffuseConstant;
    this->specularConstant = specularConstant;
    this->phongExponent = phongExponent;
    this->reflect = reflect;
}
RayTracer::Object::Object(RayTracer::ColorPack color) {
    this->color = color;
}
RayTracer::Sphere::Sphere(vector<float> center, float radius, RayTracer::ColorPack
color) : Object(color) {
    this->center = center;
    this->radius = radius;
}
// Calculates ray sphere intersection using -d dot x +- sqrt((d dot x)^2 - x dot x + R^2)
float RayTracer::Sphere::intersection(vector<float> p, vector<float> d) {
    // Vector from center to point
    vector<float> x = addVec(p, scalarVec(-1, center));
    // (d dot x)^2 - x dot x + R^2
    float underRoot = pow(dotVec(d, x), 2) - dotVec(x, x) + pow(radius, 2);
    // If value is negative, doesn't exist
    if (underRoot < 0) {
        return -1;
    }
    // sqrt of underRoot
    float root = sqrt(underRoot);
    // Solve for both solutions of quadratic formula
    float tStart = -1.0f * dotVec(d, x);
    float t1 = tStart + root;
    float t2 = tStart - root;
    // Return lowest non-negative value
    if (t1 > 0 && t2 > 0) {
        return min(t1, t2);
    }
    else if (t1 > 0) {
        return t1;
    }
    else if (t2 > 0) {
        return t2;
    }
    else {
        return -1;
    }
}
vector<float> RayTracer::Sphere::getNormal(const vector<float> &x) {
    return normalizeVec(addVec(x, scalarVec(-1, center)));
}
RayTracer::Plane::Plane(vector<float> point1, vector<float> point2, vector<float>
point3, RayTracer::ColorPack color) : Object(color) {
    this->a = point1;
    this->b = point2;
    this->c = point3;
}
// Calculates plane ray intersection using ((a - p) dot n) / (d dot n)
float RayTracer::Plane::intersection(vector<float> p, vector<float> d) {
    vector<float> normalVec = getNormal(p);
    vector<float> aMinusP = addVec(a, scalarVec(-1, p));
    // Top and bottom of formula
    float topT = dotVec(aMinusP, normalVec);
    float bottomT = dotVec(d, normalVec);
    // Divide by zero means doesn't exist
    if (bottomT == 0) {
        return -1;
    }
    // Return value if greater than zero
    float t = topT / bottomT;
    if (t < 0) {
        return -1;
    }
    return t;
}
vector<float> RayTracer::Plane::getNormal(const vector<float> &x) {
    vector<float> subPoint2 = scalarVec(-1, b);
    return normalizeVec(crossVec(addVec(a, subPoint2), addVec(c, subPoint2)));
}
RayTracer::Triangle::Triangle(vector<float> point1, vector<float> point2,
                              vector<float> point3, RayTracer::ColorPack color) : Object(color) {
    this->a = point1;
    this->b = point2;
    this->c = point3;
}
// Calculates ray triangle intersection using plane intersection and checking edge vectors
float RayTracer::Triangle::intersection(vector<float> p, vector<float> d) {
    vector<float> normalVec = getNormal(p);
    // Plane intersection method
    vector<float> aMinusP = addVec(a, scalarVec(-1, p));
    float topT = dotVec(aMinusP, normalVec);
    float bottomT = dotVec(d, normalVec);
    if (bottomT == 0) {
        return -1;
    }
    float t = topT / bottomT;
    if (t < 0) {
        return -1;
    }
    // If on plane, get that point
    vector<float> x = addVec(p, scalarVec(t, d));
    // Edge vectors
    vector<float> bMinusA = addVec(b, scalarVec(-1, a));
    vector<float> cMinusB = addVec(c, scalarVec(-1, b));
    vector<float> aMinusC = addVec(a, scalarVec(-1, c));
    vector<float> xMinusA = addVec(x, scalarVec(-1, a));
    vector<float> xMinusB = addVec(x, scalarVec(-1, b));
    vector<float> xMinusC = addVec(x, scalarVec(-1, c));
    // Traverse, if inside all 3 then inside triangle
    if (dotVec(crossVec(bMinusA, xMinusA), normalVec) > 0 ||
        dotVec(crossVec(cMinusB, xMinusB), normalVec) > 0 ||
        dotVec(crossVec(aMinusC, xMinusC), normalVec) > 0) {
        return -1;
    } else {
        return t;
    }
}
vector<float> RayTracer::Triangle::getNormal(const vector<float> &x) {
    vector<float> subPoint2 = scalarVec(-1, b);
    return normalizeVec(crossVec(addVec(a, subPoint2), addVec(c, subPoint2)));
}
RayTracer::Light::Light(vector<float> location, float intensity) {
    this->location = location;
    this->intensity = intensity;
}
RayTracer::LightObj::LightObj(vector<float> center, float radius,
                              RayTracer::ColorPack color) : Sphere(center, radius, color) {
    // Just calls parent constructor
}