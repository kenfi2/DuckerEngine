#include "program.h"

#include "engine.h"

Programs g_programs;

bool Program::createPipeline(const std::unique_ptr<Shaders> &vertexShader, const std::unique_ptr<Shaders> &fragmentShader, PrimitiveType primitiveType, uint32_t pitch)
{
    SDL_GPUDevice* gpuDevice = g_painter->getDevice();
    if(!vertexShader->bind(gpuDevice))
    {
        SDL_Log("A shader has failed. Error check: %s", SDL_GetError());
        return false;
    }

    if(!fragmentShader->bind(gpuDevice))
    {
        SDL_Log("A shader has failed. Error check: %s", SDL_GetError());
        return false;
    }

    SDL_GPUColorTargetDescription tempColor;
    SDL_zero(tempColor);

    tempColor.format = SDL_GetGPUSwapchainTextureFormat(gpuDevice, g_window->getSDLWindow());

    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo;
    SDL_zero(pipelineInfo);

    pipelineInfo.target_info.num_color_targets = 1;
    pipelineInfo.target_info.color_target_descriptions = &tempColor;

    pipelineInfo.multisample_state.sample_count = m_sampleCount;

    pipelineInfo.primitive_type = (SDL_GPUPrimitiveType)primitiveType;

    pipelineInfo.vertex_shader = vertexShader->getShader();
    pipelineInfo.fragment_shader = fragmentShader->getShader();
    if(primitiveType == SDL_GPU_PRIMITIVETYPE_LINELIST || primitiveType == SDL_GPU_PRIMITIVETYPE_LINESTRIP)
        pipelineInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE;

    SDL_GPUVertexBufferDescription vertexBufferDescription;
    SDL_zero(vertexBufferDescription);

    vertexBufferDescription.slot = 0;
    vertexBufferDescription.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertexBufferDescription.instance_step_rate = 0;
    vertexBufferDescription.pitch = pitch;

    const std::vector<SDL_GPUVertexAttribute>& vertexAttributes = vertexShader->getVertexAttributes();

    pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
    pipelineInfo.vertex_input_state.vertex_buffer_descriptions = &vertexBufferDescription;
    pipelineInfo.vertex_input_state.num_vertex_attributes = (uint32_t)vertexAttributes.size();
    pipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes.data();

    pipelineInfo.props = 0;

    m_uniforms[VertexShader] = vertexShader->getUniforms();
    m_uniforms[FragmentShader] = fragmentShader->getUniforms();

    m_pipeline = SDL_CreateGPUGraphicsPipeline(gpuDevice, &pipelineInfo);
    if(!m_pipeline)
        SDL_Log("Error creating pipeline: %s", SDL_GetError());

    return m_pipeline != nullptr;
}

void Program::destroy()
{
    if(m_pipeline)
        SDL_ReleaseGPUGraphicsPipeline(g_painter->getDevice(), m_pipeline);
}

void Program::bind(SDL_GPURenderPass* renderPass)
{
    if(m_pipeline)
        SDL_BindGPUGraphicsPipeline(renderPass, m_pipeline);
}

void Program::pushData(SDL_GPUCommandBuffer *commandBuffer)
{
    for(const auto& it : m_uniforms[VertexShader])
        SDL_PushGPUVertexUniformData(commandBuffer, it->getSlot(), it->getData(), it->getSize());

    for(const auto& it : m_uniforms[FragmentShader])
        SDL_PushGPUVertexUniformData(commandBuffer, it->getSlot(), it->getData(), it->getSize());
}

std::string getPrimitiveType(PrimitiveType primitiveType)
{
    switch(primitiveType) {
        case PrimitiveTypeTriangleList:
            return "TriangleList";
        case PrimitiveTypeTriangleStrip:
            return "TriangleStrip";
        case PrimitiveTypeLineList:
            return "LineList";
        case PrimitiveTypeLineStrip:
            return "LineStrip";
        case PrimitiveTypePointList:
            return "PointList";
    }
    return "Invalid";
}

// need to compile shaders founded in SDL/test/testgpu/
// #define USE_PRECOMPILED_SHADERS

#ifdef USE_PRECOMPILED_SHADERS
#include "../android-project/app/jni/SDL/test/testgpu/testgpu_dxil.h"
#include "../android-project/app/jni/SDL/test/testgpu/testgpu_spirv.h"
#include "../android-project/app/jni/SDL/test/testgpu/testgpu_metallib.h"
#endif

bool Programs::init(const std::string& gpuDriver)
{
#ifndef USE_PRECOMPILED_SHADERS
    static const std::vector<std::string> shaderFiles {
        "cube",
        "texture"
    };
#endif

    std::unique_ptr<Shaders> vsShader, fsShader;

#ifdef USE_PRECOMPILED_SHADERS
    const uint8_t* vData = nullptr, *fData = nullptr;
    size_t vSize = 0, fSize = 0;
    if(gpuDriver == "direct3d12") {
        vData = D3D12_CubeVert; vSize = SDL_arraysize(D3D12_CubeVert);
        fData = D3D12_CubeFrag; fSize = SDL_arraysize(D3D12_CubeFrag);
    } else if(gpuDriver == "vulkan") {
        vData = cube_vert_spv; vSize = cube_vert_spv_len;
        fData = cube_frag_spv; fSize = cube_frag_spv_len;
    } else if(gpuDriver == "metal") {
        vData = cube_vert_metallib; vSize = cube_vert_metallib_len;
        fData = cube_frag_metallib; fSize = cube_frag_metallib_len;
    }

    vsShader = std::unique_ptr<Shaders>(new Shaders(vData, vSize, true, gpuDriver));
    vsShader->createPreCompiledShaderInfo(1);
    fsShader = std::unique_ptr<Shaders>(new Shaders(fData, fSize, false, gpuDriver));
    fsShader->createPreCompiledShaderInfo(0);

    for(uint8_t i = 0; i < LastPrimitiveType; ++i) {
        PrimitiveType primitiveType = (PrimitiveType)i;
        Program* program = g_programs.get(primitiveType);
        if(!program->createPipeline(vsShader, fsShader, primitiveType, (uint32_t)sizeof(VertexBuffer)))
            std::cout << "Failed to create " << getPrimitiveType(primitiveType) << " program to precompiled shader." << std::endl;
    }
#else
    for(const auto& shaderFile : shaderFiles) {
        vsShader = std::unique_ptr<Shaders>(new Shaders);
        fsShader = std::unique_ptr<Shaders>(new Shaders);
        
        if(!vsShader->compile(shaderFile, true, gpuDriver) || !fsShader->compile(shaderFile, false, gpuDriver)) {
            std::cout << "Failed to compile shader " << shaderFile << "." << std::endl;
            return false;
        }
        
        for(uint8_t i = 0; i < LastPrimitiveType; ++i) {
            PrimitiveType primitiveType = (PrimitiveType)i;
            Program* program = g_programs.get(primitiveType, shaderFile == "texture");
            if(!program->createPipeline(vsShader, fsShader, primitiveType, (uint32_t)sizeof(VertexBuffer))) {
                std::cout << "Failed to create " << getPrimitiveType(primitiveType) << " program to shader " << shaderFile << "." << std::endl;
                return false;
            }
        }
    }
#endif

    return true;
}
