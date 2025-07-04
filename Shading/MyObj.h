#ifndef MY_OBJ_H
#define MY_OBJ_H

#include <vgl.h>
#include <vec.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdio>

struct Surface {
    int v1, v2, v3;
    vec3 normal;   

    Surface(int vertex1, int vertex2, int vertex3) : v1(vertex1), v2(vertex2), v3(vertex3), normal(vec3(0, 0, 0)) {}
};

class MyObj {
private:
    std::vector<vec3> vertices;
    std::vector<Surface> surfaces;
    std::vector<vec3> vertexNormals;

    GLuint vao;
    GLuint buffer;
    bool bInitialized;

    bool useFaceNormals;

public:
    MyObj() : bInitialized(false), useFaceNormals(false) {}
    ~MyObj() {}

    vec3 min; 
    vec3 max;

    bool loadFile(const std::string& filename) {
        std::ifstream file(filename.c_str());
        if (!file.is_open()) {
            std::cout << "Couldn't open file " << filename << std::endl;
            return false;
        }

        std::string line;
        bool firstVertex = true;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            if (line.size() > 1 && line[0] == 'v') {
                float x, y, z;
                if (sscanf_s(line.c_str(), "v %f %f %f", &x, &y, &z) == 3) {
                    vec3 vertex(x, y, z);
                    vertices.push_back(vertex);

                    if (firstVertex) {
                        min = vertex;
                        max = vertex;
                        firstVertex = false;
                    }
                    else {
                        min.x = std::min(min.x, vertex.x);
                        min.y = std::min(min.y, vertex.y);
                        min.z = std::min(min.z, vertex.z);

                        max.x = std::max(max.x, vertex.x);
                        max.y = std::max(max.y, vertex.y);
                        max.z = std::max(max.z, vertex.z);
                    }
                }
            }
            else if (line.size() > 1 && line[0] == 'f') {
                int v1, v2, v3;
                if (sscanf_s(line.c_str(), "f %d %d %d", &v1, &v2, &v3) == 3) {
                    surfaces.push_back(Surface(v1-1, v2-1, v3-1));
                }
            }
        }

        file.close();
        getNormals();
        return true;
    }
    void getNormals() {
        vertexNormals.resize(vertices.size(), vec3(0, 0, 0));

        for (size_t i = 0; i < surfaces.size(); i++) {
            
            vec3 v1 = vertices[surfaces[i].v1];
            vec3 v2 = vertices[surfaces[i].v2];
            vec3 v3 = vertices[surfaces[i].v3];

            vec3 vector1 = v2 - v1;
            vec3 vector2 = v3 - v1;

            vec3 normal = normalize(cross(vector1, vector2));
            surfaces[i].normal = normal;

            vertexNormals[surfaces[i].v1] = vertexNormals[surfaces[i].v1] + normal;
            vertexNormals[surfaces[i].v2] = vertexNormals[surfaces[i].v2] + normal;
            vertexNormals[surfaces[i].v3] = vertexNormals[surfaces[i].v3] + normal;
        }

        for (size_t i = 0; i < vertexNormals.size(); i++) {
            vertexNormals[i] = normalize(vertexNormals[i]);
        }
    }

    void normalMode(bool faceNormal) {
        if (faceNormal) useFaceNormals = true;
        else useFaceNormals = false;
        std::cout << (useFaceNormals ? "Surface Normal Mode" : "Vertex Normal Mode") << std::endl;
    }

    GLuint Init() {
        if (bInitialized) return vao;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);

        bInitialized = true;
        return vao;
    }

    void updateGpu() {
        struct Vertex {
            vec3 position;
            vec3 normal;
        };

        std::vector<Vertex> normalData;

        for (size_t i = 0; i < surfaces.size(); i++) {
            Surface face = surfaces[i];

            if (useFaceNormals) {
                normalData.push_back({ vertices[face.v1], face.normal });
                normalData.push_back({ vertices[face.v2], face.normal });
                normalData.push_back({ vertices[face.v3], face.normal });
            }
            else {
                normalData.push_back({ vertices[face.v1], vertexNormals[face.v1] });
                normalData.push_back({ vertices[face.v2], vertexNormals[face.v2] });
                normalData.push_back({ vertices[face.v3], vertexNormals[face.v3] });
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, normalData.size() * sizeof(Vertex), &normalData[0], GL_STATIC_DRAW);
    }

    void SetAttributes(GLuint program) {
        GLuint vPosition = glGetAttribLocation(program, "vPosition");
        glEnableVertexAttribArray(vPosition);
        glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (void*)0);

        GLuint vNormal = glGetAttribLocation(program, "vNormal");
        glEnableVertexAttribArray(vNormal);
        glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (void*)(sizeof(vec3)));
    }

    void Draw(GLuint program) {
        if (!bInitialized) return;

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        SetAttributes(program);

        glDrawArrays(GL_TRIANGLES, 0, surfaces.size() * 3);
    }
};

#endif // MY_OBJ_H