#include "shaders.h"

#include <utils/rect.h>
#include <utils/point.h>
#include <utils/size.h>

Shaders::Shaders(const uint8_t *data, size_t size, bool vertexShader, const std::string& device)
{
    m_buffer = data;
    m_size = size;
    m_device = device;
    if(m_device == "direct3d12")
        m_entryPoint = vertexShader ? "VSMain" : "PSMain";
    else if(m_device == "vulkan")
        m_entryPoint = "main";
    else if(m_device == "metal")
        m_entryPoint = vertexShader ? "vs_main" : "ps_main";

    m_vertexAttributes.resize(2);
    m_vertexAttributes[0].buffer_slot = 0;
    m_vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    m_vertexAttributes[0].location = 0;
    m_vertexAttributes[0].offset = 0;

    m_vertexAttributes[1].buffer_slot = 0;
    m_vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
    m_vertexAttributes[1].location = 1;
    m_vertexAttributes[1].offset = sizeof(float) * 3;

    m_stage = vertexShader ? SDL_GPU_SHADERSTAGE_VERTEX : SDL_GPU_SHADERSTAGE_FRAGMENT;
}

Shaders::~Shaders()
{
    if(m_compiled && m_buffer)
        delete[] m_buffer;
}

bool Shaders::compile(const std::string& file, bool vertexShader, const std::string& device)
{
    std::string shaderFile = file;
    m_device = device;
    if(m_device == "direct3d12" && shaderFile.find(".hlsl") == std::string::npos)
        shaderFile += ".hlsl";
    else if(m_device == "vulkan" && shaderFile.find(".glsl") == std::string::npos)
        shaderFile += ".glsl";
    else if(m_device == "metal" && shaderFile.find(".metal") == std::string::npos)
        shaderFile += ".metal";

    FILE* f = fopen(shaderFile.c_str(), "r");
    if(!f) {
        std::cout << "Cannot open shaders file: " << shaderFile << "." << std::endl;
        return false;
    }
    fclose(f);

    bool ret = false;
    if(m_device == "direct3d12")
        ret = compileD3D(shaderFile.c_str(), vertexShader, vertexShader ? "vs_6_0" : "ps_6_0");
    else if(m_device == "vulkan")
        ret = compileVulkan(shaderFile.c_str(), vertexShader, NULL);
    else if(m_device == "metal")
        ret = compileMetal(shaderFile.c_str(), vertexShader, NULL);
    if(!ret)
        std::cout << m_error << std::endl;
    return ret;
}

bool Shaders::bind(SDL_GPUDevice* device)
{
    if(!m_gpuShader.shader) {
        m_gpuShader.shader = SDL_CreateGPUShader(device, &m_shaderCreateInfo);
        m_gpuShader.device = device;
    }
    return m_gpuShader.shader != nullptr;
}

Shaders::GPUShader::~GPUShader()
{
    SDL_ReleaseGPUShader(device, shader);
}

void Shaders::createPreCompiledShaderInfo(uint32_t uniformBuffer)
{
    SDL_zero(m_shaderCreateInfo);
    m_shaderCreateInfo.num_samplers = 0;
    m_shaderCreateInfo.num_storage_buffers = 0;
    m_shaderCreateInfo.num_storage_textures = 0;

    m_shaderCreateInfo.num_uniform_buffers = uniformBuffer;
    m_shaderCreateInfo.props = 0;

    m_shaderCreateInfo.format = getFormat();
    m_shaderCreateInfo.code = m_buffer;
    m_shaderCreateInfo.code_size = m_size;
    m_shaderCreateInfo.entrypoint = m_entryPoint.c_str();

    m_shaderCreateInfo.stage = m_stage;
}

SDL_GPUShaderFormat Shaders::getFormat() const
{
    if(m_device == "direct3d12")
        return SDL_GPU_SHADERFORMAT_DXIL;
    else if(m_device == "vulkan")
        return SDL_GPU_SHADERFORMAT_SPIRV;
    else if(m_device == "metal")
        return SDL_GPU_SHADERFORMAT_METALLIB; // May be SDL_GPU_SHADERFORMAT_MSL, search and find the reason why to use this
    return SDL_GPU_SHADERFORMAT_INVALID;
}

bool Shaders::compileMetal(const char* /* file */, bool /* vertexShader */, const char* /* profile */)
{
    return false;
}

size_t Shaders::addStruct(const std::string& typeName)
{
    size_t size = 0;
    if(typeName == "Rect" || typeName == "RectF")
        size = sizeof(RectF);
    else if(typeName == "RectI")
        size = sizeof(RectI);
    else if(typeName == "Point" || typeName == "PointF")
        size = sizeof(PointF);
    else if(typeName == "PointI")
        size = sizeof(PointI);
    else if(typeName == "Size" || typeName == "SizeF")
        size = sizeof(SizeF);
    else if(typeName == "SizeI")
        size = sizeof(SizeI);
    return size;
}

size_t Shaders::addPrimitive(int type)
{
    size_t size = 0;
    switch((UniformType)type) {
        case UniformTypeBool: {
            size = sizeof(bool);
            break;
        }
        case UniformTypeInt: {
            size = sizeof(int);
            break;
        }
        case UniformTypeFloat: {
            size = sizeof(float);
            break;
        }
        default: break;
    }
    return size;
}
