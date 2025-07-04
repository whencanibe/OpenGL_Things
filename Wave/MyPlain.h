#ifndef MY_PLAIN_H
#define MY_PLAIN_H

#include <vgl.h>
#include <vec.h>

class MyPlain {
public:
    int NUM_VERTEX;
    int DIVISION;
    vec4* points;
    vec4* colors;
    GLuint vao;
    GLuint vbo;
    bool bInit;

    MyPlain() {
        DIVISION = 0;
        NUM_VERTEX = 0;  
        points = nullptr;
        colors = nullptr;
        bInit = false;
    }

    ~MyPlain() {
        delete[] points;  
        delete[] colors;
    }
   

    void init(int num) {
        if (num <= 2) num = 2;
        DIVISION = num;
        NUM_VERTEX = DIVISION * DIVISION * 2 * 3;
        
        if (points) {
            delete[] points;
        }
        if (colors) {
            delete[] colors;
        }
        points = new vec4[NUM_VERTEX];
        colors = new vec4[NUM_VERTEX];
       
        generatePlain();

     
        if (bInit == false) {
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
        }
        else {
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
        }

        bInit = true;
        
        glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(vec4) * NUM_VERTEX, NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * NUM_VERTEX, points);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * NUM_VERTEX, sizeof(vec4) * NUM_VERTEX, colors);
    }

    void draw(GLuint program) {
        glBindVertexArray(vao);
        connectShader(program);
        glDrawArrays(GL_TRIANGLES, 0, NUM_VERTEX);
    }

    void connectShader(GLuint program) {
        GLuint vPosition = glGetAttribLocation(program, "vPosition");
        glEnableVertexAttribArray(vPosition);
        glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

        GLuint vColor = glGetAttribLocation(program, "vColor");
        glEnableVertexAttribArray(vColor);
        glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vec4) * NUM_VERTEX));
    }

    void generatePlain() {
        vec4 vertex_pos[4];  
        vec4 vertex_color;  
        float step = 2.0f / DIVISION;  
        int vindex = 0;
        for (int i = 0; i < DIVISION; ++i) {
            for (int j = 0; j < DIVISION; ++j) {

                float x0 = -1.0f + j * step;
                float x1 = -1.0f + (j + 1) * step;
                float y0 = -1.0f + i * step;
                float y1 = -1.0f + (i + 1) * step;

             
                vertex_pos[0] = vec4(x0, y0, 0.0f, 1.0f);  
                vertex_pos[1] = vec4(x1, y0, 0.0f, 1.0f);  
                vertex_pos[2] = vec4(x1, y1, 0.0f, 1.0f);  
                vertex_pos[3] = vec4(x0, y1, 0.0f, 1.0f);  

                
                if ((i + j) % 2 == 0) {
                    vertex_color = vec4(0.7f, 0.7f, 0.7f, 1.0f);  // 밝은 회색
                }
                else {
                    vertex_color = vec4(0.3f, 0.3f, 0.3f, 1.0f);  // 진한 회색
                }

                points[vindex] = vertex_pos[0];    colors[vindex] = vertex_color;  vindex++;
                points[vindex] = vertex_pos[1];    colors[vindex] = vertex_color;  vindex++;
                points[vindex] = vertex_pos[2];    colors[vindex] = vertex_color;  vindex++;

                points[vindex] = vertex_pos[2];    colors[vindex] = vertex_color;  vindex++;
                points[vindex] = vertex_pos[3];    colors[vindex] = vertex_color;  vindex++;
                points[vindex] = vertex_pos[0];    colors[vindex] = vertex_color;  vindex++;
                
            }
        }
    }

    void increase() {
        int n = DIVISION + 1;
        init(n);
        printf("Division: %d, Number of Triangles: %d, Number of Vertices: %d\n",DIVISION,DIVISION*DIVISION*2,NUM_VERTEX);
        
    }

    void decrease() {
        int n = DIVISION - 1;
        init(n);
        printf("Division: %d, Number of Triangles: %d, Number of Vertices: %d\n", DIVISION, DIVISION*DIVISION * 2, NUM_VERTEX);
        
    }
};

#endif
