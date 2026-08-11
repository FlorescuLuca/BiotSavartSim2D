#include "config.h"
#include <cmath>

std::random_device rd;
std::mt19937 gen(rd());

std::uniform_real_distribution<float> dis(-3.0f, 3.0f);

const float k = 1e-7f * 100000.0f; // miu0 / 4pi scaled up


struct wire{
    glm::vec2 position;
    float current;

    wire(float x, float y, float current){
        this->position = glm::vec2(x, y);
        this->current = current;
    }
};

struct gridPoint {
    glm::vec2 position;
    float B = 0.0f;
    float magnitude = 0.0f;
    glm::vec2 dir = glm::vec2(0.0f);
};

struct Engine{

    GLFWwindow* window;
    int windowWidth = 1920;
    int windowHeight = 1080;
    const char* windowTitle = "BiotSavartSim";

    std::vector<wire> wires;
    std::vector<gridPoint> grid;

    bool mouseWasPressed = false;


    Engine(){
        if(!glfwInit()){
        std::cout<<"glfw failed to initialze"<<std::endl;
        exit(-1);
        }

        window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
    
        if (!window) {
            glfwTerminate();
            std::cout<<"window failed to create"<<std::endl;
            exit(-1);
        }

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout<<"glad failed to initialize"<<std::endl;
            exit(-1);
        }

    }

    void handleInput(){
        glfwPollEvents();

        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
            glfwSetWindowShouldClose(window, true);
        }

        int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

        if(mouseState == GLFW_PRESS && !mouseWasPressed){
            mouseWasPressed = true;

            double x, y;
            glfwGetCursorPos(window, &x, &y);

            float X = (2.0f * x) / windowWidth - 1.0f;
            float Y = 1.0f - (2.0f * y) / windowHeight;

            float random = dis(gen);

            while(random <= 0.2f && random >= -0.2f){
                random = dis(gen);
            }

            wires.push_back(wire(X, Y, random));

        }
        else if (mouseState == GLFW_RELEASE){
            mouseWasPressed = false;
        }
    }

    void draw_triangles(unsigned int biotSavartShader){
        std::vector<float> verticies;

        const float headLength = 0.01f;
        const float headWidth = 0.005f;
        const float shaftLength = 0.04f;
        const float shaftWidth = 0.001f;

        for(const auto& gridPoint: grid){

            glm::vec2 W = glm::vec2(-gridPoint.dir.y, gridPoint.dir.x);

            if(glm::length(gridPoint.dir) < 0.001f) continue;

            glm::vec2 tip = gridPoint.position + gridPoint.dir * headLength;
            glm::vec2 left = gridPoint.position - gridPoint.dir * headLength + W * headWidth;
            glm::vec2 right = gridPoint.position - gridPoint.dir * headLength - W * headWidth;

            glm::vec2 shaftTopLeft = gridPoint.position + W * shaftWidth;
            glm::vec2 shaftTopRight = gridPoint.position - W * shaftWidth;
            glm::vec2 shaftBottomLeft = gridPoint.position - gridPoint.dir * shaftLength + W * shaftWidth;
            glm::vec2 shaftBottomRight = gridPoint.position - gridPoint.dir * shaftLength - W * shaftWidth;

            float intensity = tanh(gridPoint.magnitude * 7.0f);

            verticies.push_back(tip.x);
            verticies.push_back(tip.y);
            verticies.push_back(intensity);

            verticies.push_back(left.x);
            verticies.push_back(left.y);
            verticies.push_back(intensity);

            verticies.push_back(right.x);
            verticies.push_back(right.y);
            verticies.push_back(intensity);


            verticies.push_back(shaftBottomRight.x);
            verticies.push_back(shaftBottomRight.y);
            verticies.push_back(intensity);

            verticies.push_back(shaftTopLeft.x);
            verticies.push_back(shaftTopLeft.y);
            verticies.push_back(intensity);

            verticies.push_back(shaftBottomLeft.x);
            verticies.push_back(shaftBottomLeft.y);
            verticies.push_back(intensity);


            verticies.push_back(shaftBottomRight.x);
            verticies.push_back(shaftBottomRight.y);
            verticies.push_back(intensity);

            verticies.push_back(shaftTopRight.x);
            verticies.push_back(shaftTopRight.y);
            verticies.push_back(intensity);
            
            verticies.push_back(shaftTopLeft.x);
            verticies.push_back(shaftTopLeft.y);
            verticies.push_back(intensity);


        }

        if (verticies.empty()) return;

        unsigned int VAO, VBO;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, verticies.size() * sizeof(float), verticies.data(), GL_STREAM_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        GLsizei vertexCount = static_cast<GLsizei>(verticies.size() / 3);

        unsigned int isWireLoc;
        isWireLoc = glGetUniformLocation(biotSavartShader, "isWire");
        glUniform1i(isWireLoc, GL_FALSE);

        glDrawArrays(GL_TRIANGLES, 0, vertexCount);

        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void draw_wires(unsigned int biotSavartShader){
        if(wires.empty()) return;
        
        std::vector<float> verticies;
        for(const auto& wire : wires){
            verticies.push_back(wire.position.x);
            verticies.push_back(wire.position.y);
            verticies.push_back(wire.current);
        }

        unsigned int VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, verticies.size() * sizeof(float), verticies.data(), GL_STREAM_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        unsigned int isWireLoc;
        isWireLoc = glGetUniformLocation(biotSavartShader, "isWire");
        glUniform1i(isWireLoc, GL_TRUE);

        glPointSize(12.0f);

        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(wires.size()));

        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

};

unsigned int load_shader(const std::string& vertex_file_path, const std::string fragment_file_path){
    
    std::ifstream file;
    const char* vertexShader;
    const char* fragmentShader;
    std::string line;
    std::stringstream bufferedLines;

    file.open(vertex_file_path);
    while(std::getline(file, line)) {
        bufferedLines<<line<<std::endl;
    }
    file.close();
    file.clear();

    std::string vertexSource = bufferedLines.str();
    vertexShader = vertexSource.c_str();

    bufferedLines.str("");
    bufferedLines.clear();

    file.open(fragment_file_path);
    while(std::getline(file, line)) {
        bufferedLines<<line<<std::endl;
    }
    file.close();
    file.clear();

    std::string fragmentSource = bufferedLines.str();
    fragmentShader = fragmentSource.c_str();

    int result;
    int infoLogLength;

    unsigned int vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderID, 1, &vertexShader, NULL);
    glCompileShader(vertexShaderID);

    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
    if(!result) {
        glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> shaderErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, &shaderErrorMessage[0]);
        std::cout<<"Vertex Shader Error: "<<&shaderErrorMessage[0]<<std::endl;
    }

    unsigned int fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderID, 1, &fragmentShader, NULL);
    glCompileShader(fragmentShaderID);

    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
    if(!result){
        glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> shaderErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(fragmentShaderID, infoLogLength, NULL, &shaderErrorMessage[0]);
        std::cout<<"Fragment Shader Error: "<<&shaderErrorMessage[0]<<std::endl;
    }

    unsigned int programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    if(!result){
        glGetShaderiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> programErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(programID, infoLogLength, NULL, &programErrorMessage[0]);
        std::cout<<"Shader Linker Error: "<<&programErrorMessage[0]<<std::endl;
    }

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    return programID;
}

void calculateTotalB(std::vector<wire>& wires, std::vector<gridPoint>& grid){
    for(auto& gridPoint : grid){

        glm::vec2 bDir = glm::vec2(0.0f);

        for(const auto& wire : wires){
            glm::vec2 r_vec = gridPoint.position - wire.position;
            float r = glm::length(r_vec);

            if(r < 0.04f) continue;

            glm::vec2 tangent = glm::vec2(-r_vec.y, r_vec.x);
            bDir += (tangent / (r * r)) * wire.current;
        }

        bDir *= k;

        gridPoint.B = glm::length(bDir);
        gridPoint.magnitude = abs(gridPoint.B);

        if(gridPoint.magnitude  > 0.000f){
            gridPoint.dir = glm::normalize(bDir);
        }
        else{
            gridPoint.dir = glm::vec2(0.0f);
        }

    }
}

int main(){

    Engine engine;

    const int gridWidth = 50;
    const int gridHeight = 50;

    engine.grid.resize(gridWidth * gridHeight);

    for(int i = 0; i < gridHeight; i ++){
        for(int j = 0; j < gridWidth; j ++){
            int index = i * gridWidth + j;
            float xPos = static_cast<float>(i) / (gridWidth - 1) * 2.0f - 1.0f;
            float yPos = static_cast<float>(j) / (gridHeight - 1) * 2.0f - 1.0f; 
            engine.grid[index].position = glm::vec2(xPos, yPos);
        }
    }

    unsigned int biotSavartShader = load_shader("sim.vert", "sim.frag");

    if(biotSavartShader == 0){
        std::cout<<"Shader failed to compile"<<std::endl;
        return -1;
    }

    engine.wires.push_back(wire(0.0f, 0.0f, -1.5f));

    while(!glfwWindowShouldClose(engine.window)){
        
        engine.handleInput();

        calculateTotalB(engine.wires, engine.grid);

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(biotSavartShader);

        engine.draw_triangles(biotSavartShader);
        engine.draw_wires(biotSavartShader);

        glfwSwapBuffers(engine.window);
    }

    glDeleteProgram(biotSavartShader);

    glfwTerminate();
    
    return 0;
}