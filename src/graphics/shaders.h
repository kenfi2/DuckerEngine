#ifndef SHADERS_H
#define SHADERS_H

#include <utils/include.h>

enum PrimitiveType : uint8_t {
    PrimitiveTypeTriangleList,
    PrimitiveTypeTriangleStrip,
    PrimitiveTypeLineList,
    PrimitiveTypeLineStrip,
    PrimitiveTypePointList,

    LastPrimitiveType
};

class Shaders {
    struct GPUShader {
        ~GPUShader();

        SDL_GPUShader* shader = nullptr;
        SDL_GPUDevice* device = nullptr;
    };

public:
    explicit Shaders() = default;
    Shaders(const uint8_t* data, size_t size, bool vertexShader, const std::string& device);
    ~Shaders();

    bool compile(const char* file, bool vertexShader, const std::string& device);

    bool bind(SDL_GPUDevice* device);

    void createPreCompiledShaderInfo(uint32_t uniformBuffer = 0);

    std::string getError() const { return m_error; }
    std::string getEntryPoint() const { return m_entryPoint; }
    SDL_GPUShader* getShader() const { return m_gpuShader.shader; }
    const uint8_t* getBuffer() const { return m_buffer; }
    size_t getSize() const { return m_size; }

    SDL_GPUShaderFormat getFormat() const;

    const std::vector<SDL_GPUVertexAttribute>& getVertexAttributes() const { return m_vertexAttributes; }

protected:
    bool compileD3D(const char* file, bool vertexShader, const char* profile);
    bool compileVulkan(const char* file, bool vertexShader, const char* profile);
    bool compileMetal(const char* file, bool vertexShader, const char* profile);

private:
    std::vector<SDL_GPUVertexAttribute> m_vertexAttributes;
    SDL_GPUShaderCreateInfo m_shaderCreateInfo;
    std::string m_error;
    std::string m_entryPoint;
    std::string m_device;
    GPUShader m_gpuShader;
    const uint8_t* m_buffer = nullptr;
    size_t m_size = 0;
    bool m_compiled = false;
    SDL_GPUShaderStage m_stage = SDL_GPU_SHADERSTAGE_VERTEX;
};

#endif
