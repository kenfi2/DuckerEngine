#include "program.h"

#include "engine.h"

Programs g_programs;

bool Program::createPipeline(const std::unique_ptr<Shaders>& vertexShader, const std::unique_ptr<Shaders>& fragmentShader, PrimitiveType primitiveType, uint32_t pitch)
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

void Program::pushData(SDL_GPUCommandBuffer* cmdBuffer, uint32_t slot, const void* data, uint32_t length)
{
    SDL_PushGPUVertexUniformData(cmdBuffer, slot, data, length);
}
