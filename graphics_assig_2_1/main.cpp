// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Co-Authors:
//            Jeremy Hart, University of Calgary
//            John Hall, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <array>
#include <string>
#include <vector>
#include <iterator>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "texture.h"

using namespace std;
using namespace glm;
// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
void addVertices(MyTexture incomingTexture);
void resetLuminance();

MyTexture myTexture;
vector<vec2> vertices;
vector<vec3> colours;
vector<vec2> textureCoords;

string image_path = "res/image5-pattern.png";
bool leftClicked = false;
bool rightClicked = false;
bool isScroll = false;
double xpos, ypos;
double prevx, prevy;
float translateSpeed = 2.0f;
int zoomLevel = 0;
bool zoomTooMuch = true;
float initImageWidth;
float initImageHeight;

float kernelSize = 9.0f;
mat4 transformVertice = mat4(1.0f);
float adjustBrightness = 0;
float doSobel = 0;
float horSobel = 0;
float doUnSharp = 0;
float doGauss = 0;
float gaussVal = 0;

struct LuminanceValues
{
    float r;
    float g;
    float b;
};
LuminanceValues luminanceValues;


// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

// load, compile, and link shaders, returning true if successful
GLuint InitializeShaders()
{
    // load shader source from files
    string vertexSource = LoadSource("shaders/vertex.glsl");
    string fragmentSource = LoadSource("shaders/fragment.glsl");
    if (vertexSource.empty() || fragmentSource.empty()) return false;
    
    // compile shader source into shader objects
    GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    // link shader program
    GLuint program = LinkProgram(vertex, fragment);
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    // check for OpenGL errors and return false if error occurred
    return program;
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

struct Geometry
{
    // OpenGL names for array buffer objects, vertex array object
    GLuint  vertexBuffer;
    GLuint  textureBuffer;
    GLuint  colourBuffer;
    GLuint  vertexArray;
    GLsizei elementCount;
    
    // initialize object names to zero (OpenGL reserved value)
    Geometry() : vertexBuffer(0), colourBuffer(0), vertexArray(0), elementCount(0)
    {}
};

bool InitializeVAO(Geometry *geometry){
    
    const GLuint VERTEX_INDEX = 0;
    const GLuint COLOUR_INDEX = 1;
    const GLuint TEXTURE_INDEX = 2;
    
    //Generate Vertex Buffer Objects
    // create an array buffer object for storing our vertices
    glGenBuffers(1, &geometry->vertexBuffer);
    
    // create another one for storing our colours
    glGenBuffers(1, &geometry->colourBuffer);
    
    // create another one for storing our texture coords
    glGenBuffers(1, &geometry->textureBuffer);
    
    //Set up Vertex Array Object
    // create a vertex array object encapsulating all our vertex attributes
    glGenVertexArrays(1, &geometry->vertexArray);
    glBindVertexArray(geometry->vertexArray);
    
    // associate the position array with the vertex array object
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
    glVertexAttribPointer(
                          VERTEX_INDEX,        //Attribute index
                          2,                     //# of components
                          GL_FLOAT,             //Type of component
                          GL_FALSE,             //Should be normalized?
                          sizeof(vec2),        //Stride - can use 0 if tightly packed
                          0);                    //Offset to first element
    glEnableVertexAttribArray(VERTEX_INDEX);
    
    // associate the colour array with the vertex array object
    glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
    glVertexAttribPointer(
                          COLOUR_INDEX,        //Attribute index
                          3,                     //# of components
                          GL_FLOAT,             //Type of component
                          GL_FALSE,             //Should be normalized?
                          sizeof(vec3),         //Stride - can use 0 if tightly packed
                          0);                    //Offset to first element
    glEnableVertexAttribArray(COLOUR_INDEX);
    
    // associate the texture array with the vertex array object
    glBindBuffer(GL_ARRAY_BUFFER, geometry->textureBuffer);
    glVertexAttribPointer(
                          TEXTURE_INDEX,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(vec2),
                          0);
    glEnableVertexAttribArray(TEXTURE_INDEX);
    
    // unbind our buffers, resetting to default state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    return !CheckGLErrors();
}

// create buffers and fill with geometry data, returning true if successful
bool LoadGeometry(Geometry *geometry, int elementCount)
{
    geometry->elementCount = elementCount;
    
    // create an array buffer object for storing our vertices
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
    
    // create another one for storing our colours
    glBindBuffer(GL_ARRAY_BUFFER, geometry->colourBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*colours.size(), &colours[0], GL_STATIC_DRAW);
    
    // create another one for storing our textures
    glBindBuffer(GL_ARRAY_BUFFER, geometry->textureBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*textureCoords.size(), &textureCoords[0], GL_STATIC_DRAW);
    
    //Unbind buffer to reset to default state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // check for OpenGL errors and return false if error occurred
    return !CheckGLErrors();
}

// deallocate geometry-related objects
void DestroyGeometry(Geometry *geometry)
{
    // unbind and destroy our vertex array object and associated buffers
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &geometry->vertexArray);
    glDeleteBuffers(1, &geometry->vertexBuffer);
    glDeleteBuffers(1, &geometry->colourBuffer);
}

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void RenderScene(Geometry *geometry, MyTexture *texture, GLuint program)
{
    // clear screen to a dark grey colour
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // transformations
    if (leftClicked) {
        // translate the image
        transformVertice = translate(transformVertice, vec3(( translateSpeed*((float)xpos - (float)prevx)/(float)myTexture.width), -translateSpeed*(((float)ypos - (float)prevy)/(float)myTexture.height) , 0.0f));
    } else if (rightClicked) {
        // rotate the image
        transformVertice = rotate(transformVertice, (((float)xpos - (float)prevx) /(float)myTexture.width), vec3(0.0f, 0.0f, 1.0f));
    }
    prevx = xpos;
    prevy = ypos;
    
    // bind our shader program and the vertex array object containing our
    // scene geometry, then tell OpenGL to draw our geometry
    glUseProgram(program);
    
    // transformation
    unsigned int transformLoc = glGetUniformLocation(program, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(transformVertice));
    
    // luminance
    unsigned int luminanceOfTexture = glGetUniformLocation(program, "luminanceValues");
    glUniform3f(luminanceOfTexture, luminanceValues.r, luminanceValues.g, luminanceValues.b);
    
    // brightness
    unsigned int brightnessOfTexture = glGetUniformLocation(program, "adjustBrightness");
    glUniform1f(brightnessOfTexture, adjustBrightness);
    
    // sobel
    unsigned int sobelTexture = glGetUniformLocation(program, "doSobel");
    glUniform1f(sobelTexture, doSobel);
    
    // sobal orientation
    unsigned int sobelOrientation = glGetUniformLocation(program, "horSobel");
    glUniform1f(sobelOrientation, horSobel);
    
    // image height and width
    unsigned int textureWidth = glGetUniformLocation(program, "imageWidth");
    glUniform1f(textureWidth, myTexture.width);
    unsigned int textureHeight = glGetUniformLocation(program, "imageHeight");
    glUniform1f(textureHeight, myTexture.height);
    
    // sharpen
    unsigned int sharpen = glGetUniformLocation(program, "doUnSharp");
    glUniform1f(sharpen, doUnSharp);
    
    // gaus
    unsigned int gauss = glGetUniformLocation(program, "doGauss");
    glUniform1f(gauss, doGauss);
    unsigned int gaussValue = glGetUniformLocation(program, "gaussVal");
    glUniform1f(gaussValue, gaussVal);
    
    glBindVertexArray(geometry->vertexArray);
    glBindTexture(texture->target, texture->textureID);
    glDrawArrays(GL_TRIANGLES, 0, geometry->elementCount);
    
    // reset state to default (no shader or geometry bound)
    glBindTexture(texture->target, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    
    // check for an report any OpenGL errors
    CheckGLErrors();
}

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
    cout << "GLFW ERROR " << error << ":" << endl;
    cout << description << endl;
}

// handles mouse input events
static void cursorPositionCallback(GLFWwindow *window, double xpos, double ypos)
{
    //std::cout << xpos << " : " << ypos << std::endl;
}
void cursorEnterCallback(GLFWwindow *window, int entered)
{
    if (entered) {
        //std::cout << "Entered Window" << std::endl;
    } else {
        //std::cout << "Left window" << std::endl;
    }
}
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        //std::cout << "Right button pressed" << std::endl;
        rightClicked = true;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        //std::cout << "Right button released" << std::endl;
        rightClicked = false;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        //std::cout << "Left button pressed" << std::endl;
        leftClicked = true;
        prevx = xpos;
        prevy = ypos;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        //std::cout << "Left button released" << std::endl;
        leftClicked = false;
    }
}
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (yoffset < 0) {
        // zoom out
        zoomLevel -= 1;
        if (zoomLevel < 0) {
            translateSpeed *= 2;
        }
        
        transformVertice = scale(transformVertice, vec3( 0.5f ));
    } else {
        // zoom in
        zoomLevel += 1;
        if (zoomLevel == 0) {
            translateSpeed = 2.0f;
        } else {
            if (translateSpeed / 2.0f > 2) {
                translateSpeed /= 2.0f;
            } else {
                translateSpeed = 2.0f;
            }
        }
        
        transformVertice = scale(transformVertice, vec3( 2.0f ));
    }
    
    isScroll = false;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
        
    // for changing image
    } else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        image_path = "res/image1-mandrill.png";
        if (!InitializeTexture(&myTexture, image_path.c_str())) {
            cout << "Program failed to initialize texture!" << endl;
        }
        addVertices(myTexture);
    } else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        image_path = "res/image2-uclogo.png";
        if (!InitializeTexture(&myTexture, image_path.c_str())) {
            cout << "Program failed to initialize texture!" << endl;
        }
        addVertices(myTexture);
    } else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        image_path = "res/image3-aerial.jpg";
        if (!InitializeTexture(&myTexture, image_path.c_str())) {
            cout << "Program failed to initialize texture!" << endl;
        }
        addVertices(myTexture);
    } else if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        image_path = "res/image4-thirsk.jpg";
        if (!InitializeTexture(&myTexture, image_path.c_str())) {
            cout << "Program failed to initialize texture!" << endl;
        }
        addVertices(myTexture);
    } else if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
        image_path = "res/image5-pattern.png";
        if (!InitializeTexture(&myTexture, image_path.c_str())) {
            cout << "Program failed to initialize texture!" << endl;
        }
        addVertices(myTexture);
    } else if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
        image_path = "res/image6-Banff.jpg";
        if (!InitializeTexture(&myTexture, image_path.c_str())) {
            cout << "Program failed to initialize texture!" << endl;
        }
        addVertices(myTexture);
    }
    
    // for luminance
    else if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        doSobel = 0;
        horSobel = 0;
        adjustBrightness = 0;
        doUnSharp = 0;
        doGauss = 0.0f;
        gaussVal = 0;
        
        luminanceValues.r = 0.333;
        luminanceValues.g = 0.333;
        luminanceValues.b = 0.333;
    } else if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        doSobel = 0;
        horSobel = 0;
        adjustBrightness = 0;
        doUnSharp = 0;
        doGauss = 0.0f;
        gaussVal = 0;
        
        luminanceValues.r = 0.299;
        luminanceValues.g = 0.587;
        luminanceValues.b = 0.114;
    } else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        doSobel = 0;
        horSobel = 0;
        adjustBrightness = 0;
        doUnSharp = 0;
        doGauss = 0.0f;
        gaussVal = 0;
        
        luminanceValues.r = 0.213;
        luminanceValues.g = 0.715;
        luminanceValues.b = 0.072;
    
    // for brightness filter
    } else if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        doSobel = 0;
        horSobel = 0;
        adjustBrightness = 1.0f;
        doUnSharp = 0.0f;
        doGauss = 0.0f;
        gaussVal = 0;
        
        resetLuminance();
        
    // reset back to original
    } else if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        doSobel = 0;
        horSobel = 0;
        adjustBrightness = 0;
        doUnSharp = 0.0f;
        doGauss = 0.0f;
        gaussVal = 0;
        
        resetLuminance();
    
    // horizontal sobel filter
    } else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        doSobel = 1.0;
        horSobel = 1.0;
        adjustBrightness = 0;
        doUnSharp = 0.0f;
        doGauss = 0.0f;
        gaussVal = 0;
        
        resetLuminance();
    
    // vertical sobel filter
    } else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        doSobel = 1.0f;
        horSobel = 0.0;
        adjustBrightness = 0;
        doUnSharp = 0.0f;
        doGauss = 0.0f;
        gaussVal = 0;
        
        resetLuminance();
    
    // sharpen filter
    } else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        doSobel = 0;
        horSobel = 0;
        adjustBrightness = 0;
        doUnSharp = 1.0f;
        doGauss = 0.0f;
        gaussVal = 0;
        
        resetLuminance();
    
    // gauss 3x3
    } else if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        doSobel = 0;
        horSobel = 0;
        adjustBrightness = 0;
        doUnSharp = 0.0f;
        doGauss = 1.0f;
        gaussVal = 3.0f;
        
        resetLuminance();
    
    // gauss 5x5
    } else if (key == GLFW_KEY_K && action == GLFW_PRESS) {
        doSobel = 0;
        horSobel = 0;
        adjustBrightness = 0;
        doUnSharp = 0.0f;
        doGauss = 1.0f;
        gaussVal = 5.0f;
        
        resetLuminance();
    
    // gauss 7x7
    } else if (key == GLFW_KEY_J && action == GLFW_PRESS) {
        doSobel = 0;
        horSobel = 0;
        adjustBrightness = 0;
        doUnSharp = 0.0f;
        doGauss = 1.0f;
        gaussVal = 7.0f;
        
        resetLuminance();
    }
}

void resetLuminance()
{
    luminanceValues.r = 1.0;
    luminanceValues.g = 1.0;
    luminanceValues.b = 1.0;
}

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
    // initialize the GLFW windowing system
    if (!glfwInit()) {
        cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
        return -1;
    }
    glfwSetErrorCallback(ErrorCallback);
    
    // attempt to create a window with an OpenGL 4.1 core profile context
    GLFWwindow *window = 0;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    int width = 512, height = 512;
    window = glfwCreateWindow(width, height, "CPSC 453 OpenGL Boilerplate", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return -1;
    }
    
    // set mouse callback functions
    glfwSetCursorPosCallback( window, cursorPositionCallback );
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
    glfwSetCursorEnterCallback( window, cursorEnterCallback );
    glfwSetMouseButtonCallback( window, mouseButtonCallback );
    glfwSetInputMode( window, GLFW_STICKY_MOUSE_BUTTONS, 1 );
    glfwSetScrollCallback( window, scrollCallback );
    
    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, KeyCallback);
    glfwMakeContextCurrent(window);
    
    //Intialize GLAD
    if (!gladLoadGL())
    {
        cout << "GLAD init failed" << endl;
        return -1;
    }
    
    // query and print out information about our OpenGL environment
    QueryGLVersion();
    
    // call function to load and compile shader programs
    GLuint program = InitializeShaders();
    if (program == 0) {
        cout << "Program could not initialize shaders, TERMINATING" << endl;
        return -1;
    }
    
    textureCoords.push_back(vec2(0.0f, 1.0f));
    textureCoords.push_back(vec2(1.0f, 1.0f));
    textureCoords.push_back(vec2(0.0f, 0.0f));
    textureCoords.push_back(vec2(1.0f, 0.0f));
    textureCoords.push_back(vec2(1.0f, 1.0f));
    textureCoords.push_back(vec2(0.0f, 0.0f));
    
    colours.push_back(vec3( 1.0f, 1.0f, 1.0f ));
    colours.push_back(vec3( 1.0f, 1.0f, 1.0f ));
    colours.push_back(vec3( 1.0f, 1.0f, 1.0f ));
    colours.push_back(vec3( 1.0f, 1.0f, 1.0f ));
    colours.push_back(vec3( 1.0f, 1.0f, 1.0f ));
    colours.push_back(vec3( 1.0f, 1.0f, 1.0f ));
    
    // call function to create and fill buffers with geometry data
    Geometry geometry;
    if (!InitializeVAO(&geometry)) {
        cout << "Program failed to intialize geometry!" << endl;
    }
    
    // set the initial values of the luminance to 1
    luminanceValues.r = 1;
    luminanceValues.g = 1;
    luminanceValues.b = 1;

    const char* tmp_image_path = image_path.c_str();
    if (!InitializeTexture(&myTexture, tmp_image_path)) {
        cout << "Program failed to initialize texture!" << endl;
    }
    
    addVertices(myTexture);
    
    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window)) {
        glUseProgram(program);
        
        if(!LoadGeometry(&geometry, 6)) {
            cout << "Failed to load geometry" << endl;
        }
    
        // call function to draw our scene
        RenderScene(&geometry, &myTexture, program);
        glfwGetCursorPos( window, &xpos, &ypos );
        
        
        glfwSwapBuffers(window);
        
        glfwPollEvents();
    }
    
    // clean up allocated resources before exit
    DestroyGeometry(&geometry);
    glUseProgram(0);
    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();
    
    cout << "Goodbye!" << endl;
    return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
    // query opengl version and renderer information
    string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
    
    cout << "OpenGL [ " << version << " ] "
    << "with GLSL [ " << glslver << " ] "
    << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
    bool error = false;
    for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
    {
        cout << "OpenGL ERROR:  ";
        switch (flag) {
            case GL_INVALID_ENUM:
            cout << "GL_INVALID_ENUM" << endl; break;
            case GL_INVALID_VALUE:
            cout << "GL_INVALID_VALUE" << endl; break;
            case GL_INVALID_OPERATION:
            cout << "GL_INVALID_OPERATION" << endl; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
            cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
            case GL_OUT_OF_MEMORY:
            cout << "GL_OUT_OF_MEMORY" << endl; break;
            default:
            cout << "[unknown error code]" << endl;
        }
        error = true;
    }
    return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
    string source;
    
    ifstream input(filename.c_str());
    if (input) {
        copy(istreambuf_iterator<char>(input),
             istreambuf_iterator<char>(),
             back_inserter(source));
        input.close();
    }
    else {
        cout << "ERROR: Could not load shader source from file "
        << filename << endl;
    }
    
    return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
    // allocate shader object name
    GLuint shaderObject = glCreateShader(shaderType);
    
    // try compiling the source as a shader of the given type
    const GLchar *source_ptr = source.c_str();
    glShaderSource(shaderObject, 1, &source_ptr, 0);
    glCompileShader(shaderObject);
    
    // retrieve compile status
    GLint status;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
        cout << "ERROR compiling shader:" << endl << endl;
        cout << source << endl;
        cout << info << endl;
    }
    
    return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    // allocate program object name
    GLuint programObject = glCreateProgram();
    
    // attach provided shader objects to this program
    if (vertexShader)   glAttachShader(programObject, vertexShader);
    if (fragmentShader) glAttachShader(programObject, fragmentShader);
    
    // try linking the program with given attachments
    glLinkProgram(programObject);
    
    // retrieve link status
    GLint status;
    glGetProgramiv(programObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
        cout << "ERROR linking shader program:" << endl;
        cout << info << endl;
    }
    
    return programObject;
}

void addVertices(MyTexture incomingTexture)
{
    vertices.clear();
    
    if (incomingTexture.width/incomingTexture.height < 1) {
        initImageWidth = (float) incomingTexture.width/incomingTexture.height;
        initImageHeight = 1;
    } else if (incomingTexture.height/incomingTexture.width < 1) {
        initImageHeight = (float) incomingTexture.height/incomingTexture.width;
        initImageWidth = 1;
    }
    
    vertices.push_back(vec2(-initImageWidth, initImageHeight));
    vertices.push_back(vec2(initImageWidth, initImageHeight));
    vertices.push_back(vec2(-initImageWidth, -initImageHeight));
    
    vertices.push_back(vec2(initImageWidth, -initImageHeight));
    vertices.push_back(vec2(initImageWidth, initImageHeight));
    vertices.push_back(vec2(-initImageWidth, -initImageHeight));
}
