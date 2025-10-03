#include "program.h"

#include "engine.h"

#include <unordered_set>

Programs g_programs;

bool Program::createPipeline(const std::unique_ptr<Shaders> &vertexShader, const std::unique_ptr<Shaders> &fragmentShader, BlendMode blendMode, PrimitiveType primitiveType, uint32_t pitch)
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

    SDL_GPUColorTargetBlendState blendState;
    SDL_zero(blendState);

    switch(blendMode)
    {
        case BlendMode_NoBlend:
            blendState.enable_blend = false;
            break;

        case BlendMode_Blend:
            blendState.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
            blendState.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            blendState.color_blend_op  = SDL_GPU_BLENDOP_ADD;
            blendState.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
            blendState.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            blendState.alpha_blend_op  = SDL_GPU_BLENDOP_ADD;
            break;

        case BlendMode_Multiply:
            blendState.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
            blendState.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_COLOR;
            blendState.color_blend_op  = SDL_GPU_BLENDOP_ADD;
            blendState.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
            blendState.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
            blendState.alpha_blend_op  = SDL_GPU_BLENDOP_ADD;
            break;

        case BlendMode_Add:
            blendState.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
            blendState.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
            blendState.color_blend_op  = SDL_GPU_BLENDOP_ADD;
            blendState.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
            blendState.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
            blendState.alpha_blend_op  = SDL_GPU_BLENDOP_ADD;
            break;

        case BlendMode_MultiplyMixed:
            blendState.src_color_blendfactor = SDL_GPU_BLENDFACTOR_DST_COLOR;
            blendState.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            blendState.color_blend_op  = SDL_GPU_BLENDOP_ADD;
            blendState.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_DST_ALPHA;
            blendState.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
            blendState.alpha_blend_op  = SDL_GPU_BLENDOP_ADD;
            break;
    }

    SDL_GPUColorTargetDescription tempColor;
    SDL_zero(tempColor);

    tempColor.format = SDL_GetGPUSwapchainTextureFormat(gpuDevice, g_window->getSDLWindow());
    tempColor.blend_state = blendState;

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

    for(Uniform& uniform : m_uniformLocations)
        uniform.value = 0LL;

    if(m_features == 0) {
        for(int i = 0; i < LastShaderType; ++i) {
            for(const CBufferPtr& uniform : m_uniforms[i]) {
                if(uniform->hasVariable("v_TexCoord"))
                    m_features |= ShaderFeature_TexCoord;
                if(uniform->hasVariable("u_Tex0"))
                    m_features |= ShaderFeature_Texture0;
                if(uniform->hasVariable("u_Tex1"))
                    m_features |= ShaderFeature_Texture1;
                if(uniform->hasVariable("u_Tex2"))
                    m_features |= ShaderFeature_Texture2;
                if(uniform->hasVariable("u_TexScale"))
                    m_features |= ShaderFeature_TextureScale;
                if(uniform->hasVariable("u_Time"))
                    m_features |= ShaderFeature_Time;
                if(uniform->hasVariable("u_GlobalTime"))
                    m_features |= ShaderFeature_GlobalTime;
                if(uniform->hasVariable("u_Resolution"))
                    m_features |= ShaderFeature_Resolution;
                if(uniform->hasVariable("u_Color"))
                    m_features |= ShaderFeature_Color;
                if(uniform->hasVariable("u_Level"))
                    m_features |= ShaderFeature_Level;
                if(uniform->hasVariable("u_Size"))
                    m_features |= ShaderFeature_Size;
                if(uniform->hasVariable("u_Position"))
                    m_features |= ShaderFeature_Position;
                if(uniform->hasVariable("u_RectSize"))
                    m_features |= ShaderFeature_RectSize;
                if(uniform->hasVariable("u_RectOffset"))
                    m_features |= ShaderFeature_RectOffset;
            }
        }
    }

    bindUniformLocation(PROJECTIONTRANSFORM_MATRIX_UNIFORM, "u_ProjectionTransformMatrix");

    if(m_features & ShaderFeature_Color)
        bindUniformLocation(COLOR_UNIFORM, "u_Color");
    if(m_features & ShaderFeature_Resolution)
        bindUniformLocation(RESOLUTION_UNIFORM, "u_Resolution");
    if(m_features & ShaderFeature_Size)
        bindUniformLocation(SIZE_UNIFORM, "u_Size");
    if(m_features & ShaderFeature_Level)
        bindUniformLocation(LEVEL_UNIFORM, "u_Level");
    if(m_features & ShaderFeature_Time)
        bindUniformLocation(TIME_UNIFORM, "u_Time");
    if(m_features & ShaderFeature_GlobalTime)
        bindUniformLocation(GLOBAL_TIME_UNIFORM, "u_GlobalTime");
    if(m_features & ShaderFeature_Texture0)
        bindUniformLocation(TEX0_UNIFORM, "u_Tex0");
    if(m_features & ShaderFeature_Texture1)
        bindUniformLocation(TEX1_UNIFORM, "u_Tex1");
    if(m_features & ShaderFeature_Texture2)
        bindUniformLocation(TEX2_UNIFORM, "u_Tex2");
    if(m_features & ShaderFeature_TextureScale)
        bindUniformLocation(TEXTURE_SCALE_UNIFORM, "u_TexScale");
    if(m_features & ShaderFeature_Position)
        bindUniformLocation(POSITION_UNIFORM, "u_Position");
    if(m_features & ShaderFeature_RectSize)
        bindUniformLocation(RECT_SIZE_UNIFORM, "u_RectSize");
    if(m_features & ShaderFeature_RectOffset)
        bindUniformLocation(RECT_OFFSET_UNIFORM, "u_RectOffset");

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
    for(const auto& it : m_uniforms[VertexShader]) {
        if(it->hasChanged()) {
            SDL_PushGPUVertexUniformData(commandBuffer, it->getSlot(), it->getData(), it->getSize());
            it->setUnchanged();
        }
    }

    for(const auto& it : m_uniforms[FragmentShader]) {
        if(it->hasChanged()) {
            SDL_PushGPUFragmentUniformData(commandBuffer, it->getSlot(), it->getData(), it->getSize());
            it->setUnchanged();
        }
    }
}

void Program::createShaderProgram(const std::string& vertexShader, const std::string& fragmentShader, uint32_t features)
{
    m_features = features;
}

void Program::setProjectionTransformMatrix(const Matrix3 &projectionTransformMatrix)
{
    static Matrix4 projTransMatrix;

    float* data = projTransMatrix.data();

    data[0] = projectionTransformMatrix(1,1);
    data[1] = projectionTransformMatrix(1,2);
    data[2] = 0.0f;
    data[3] = projectionTransformMatrix(1,3);

    data[4] = projectionTransformMatrix(2,1);
    data[5] = projectionTransformMatrix(2,2);
    data[6] = 0.0f;
    data[7] = projectionTransformMatrix(2,3);

    data[8] = 0.0f;
    data[9] = 0.0f;
    data[10] = 1.0f;
    data[11] = 0.0f;

    data[12] = projectionTransformMatrix(3,1);
    data[13] = projectionTransformMatrix(3,2);
    data[14] = 0.0f;
    data[15] = projectionTransformMatrix(3,3);

    setUniform(PROJECTIONTRANSFORM_MATRIX_UNIFORM, projTransMatrix);
}

void Program::setResolution(const SizeI& resolution)
{
    if(m_features & ShaderFeature_Resolution)
        setUniform(RESOLUTION_UNIFORM, resolution);
}

void Program::setTextureSize(const SizeI& textureSize)
{
    if(m_features & ShaderFeature_TextureScale) {
        auto textureScale = PointF(1.0f, 1.0f) / textureSize.toPointF();
        setUniform(TEXTURE_SCALE_UNIFORM, textureScale);
    }
}

void Program::setSize(float size)
{
    if(m_features & ShaderFeature_Size)
        setUniform(SIZE_UNIFORM, size);
}

void Program::setLevel(float level)
{
    if(m_features & ShaderFeature_Level)
        setUniform(LEVEL_UNIFORM, level);
}

void Program::setColor(const Color& color)
{
    if(m_features & ShaderFeature_Color)
        setUniform(COLOR_UNIFORM, color);
}

void Program::setTime(float time)
{
    if(m_features & ShaderFeature_Time)
        setUniform(TIME_UNIFORM, time);
}
/* 
void Program::setPosition(Point3F position)
{
    if(m_features & ShaderFeature_Position)
        setUniform(POSITION_UNIFORM, position.x, position.y, position.z);
}
 */
void Program::setRectSize(const SizeF& rectOffset)
{
    if(m_features & ShaderFeature_RectSize)
        setUniform(RECT_SIZE_UNIFORM, rectOffset);
}

void Program::setRectOffset(const PointF& rectOffset)
{
    if(m_features & ShaderFeature_RectOffset)
        setUniform(RECT_OFFSET_UNIFORM, rectOffset);
}

void Program::bindUniformLocation(size_t index, const std::string &variable)
{
    for(const CBufferPtr& uniform : m_uniforms[VertexShader]) {
        const auto& variables = uniform->getVariables();
        auto it = variables.find(variable);
        if(it != variables.end()) {
            uint32_t location = (uint32_t)it->second;
            if(location != (uint32_t)-1) {
                auto& udata = m_uniformLocations[index];
                udata.data.location = location;
                udata.data.slot = (uint16_t)uniform->getSlot();
                udata.data.type = VertexShader;
            }
        }
    }

    for(const CBufferPtr& uniform : m_uniforms[FragmentShader]) {
        const auto& variables = uniform->getVariables();
        auto it = variables.find(variable);
        if(it != variables.end()) {
            uint32_t location = (uint32_t)it->second;
            if(location != (uint32_t)-1) {
                auto& udata = m_uniformLocations[index];
                udata.data.location = location;
                udata.data.slot = (uint16_t)uniform->getSlot();
                udata.data.type = FragmentShader;
            }
        }
    }
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
// #define USE_PRECOMPILED_SHADERS 1

// create shaders resource
#define USE_LUNA_SHADERS_DESIGN 1

#ifdef USE_PRECOMPILED_SHADERS
#include "../android-project/app/jni/SDL/test/testgpu/testgpu_dxil.h"
#include "../android-project/app/jni/SDL/test/testgpu/testgpu_spirv.h"
#include "../android-project/app/jni/SDL/test/testgpu/testgpu_metallib.h"
#endif

#ifdef USE_LUNA_SHADERS_DESIGN

std::string mainVertexShader = R"(
cbuffer UBO : register(b0, space1)
{
    float4x4 u_ProjectionTransformMatrix;
};

struct VertexShaderInput
{
    float2 Position : TEXCOORD0;
};

struct VertexShaderOutput
{
    float4 position : SV_Position;
};

VertexShaderOutput VSMain(VertexShaderInput input)
{
    VertexShaderOutput vertexShaderOutput;
    vertexShaderOutput.position = mul(u_ProjectionTransformMatrix, float4(input.Position.xy, 1.0, 1.0));
    return vertexShaderOutput;
}
)";

std::string solidColorFragmentShader = R"(
cbuffer UBO : register(b0, space3)
{
    float4 u_Color;
};

struct PixelShaderInput
{
    float4 position : SV_Position;
};

float4 PSMain(PixelShaderInput input) : SV_Target0
{
    return u_Color;
}
)";

#endif

bool Programs::init(const std::string& gpuDriver)
{
    std::unique_ptr<Shaders> vsShader, fsShader;
#if !USE_PRECOMPILED_SHADERS && !USE_LUNA_SHADERS_DESIGN
    static const std::vector<std::string> shaderFiles {
        "cube",
        "texture"
    };
#elif USE_LUNA_SHADERS_DESIGN
    vsShader = std::unique_ptr<Shaders>(new Shaders);
    fsShader = std::unique_ptr<Shaders>(new Shaders);
    
    if(!vsShader->compile(mainVertexShader, "mainVertexShader", true, gpuDriver) || !fsShader->compile(solidColorFragmentShader, "solidColorFragmentShader", false, gpuDriver))
        return false;

    for(uint8_t b = 0; b < BlendMode_Last; ++b) {
        for(uint8_t i = 0; i < LastPrimitiveType; ++i) {
            PrimitiveType primitiveType = (PrimitiveType)i;
            BlendMode blendMode = (BlendMode)b;
            Program* program = g_programs.get(blendMode, primitiveType, 0);
            if(!program->createPipeline(vsShader, fsShader, blendMode, primitiveType, (uint32_t)sizeof(SolidVertexBuffer))) {
                std::cout << "Failed to create " << getPrimitiveType(primitiveType) << " program to shader." << std::endl;
                return false;
            }
        }
    }
#endif

#if USE_PRECOMPILED_SHADERS
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
#elif !USE_LUNA_SHADERS_DESIGN
    for(const auto& shaderFile : shaderFiles) {
        vsShader = std::unique_ptr<Shaders>(new Shaders);
        fsShader = std::unique_ptr<Shaders>(new Shaders);
        
        if(!vsShader->load(shaderFile, true, gpuDriver) || !fsShader->load(shaderFile, false, gpuDriver)) {
            std::cout << "Failed to load and compile shader " << shaderFile << "." << std::endl;
            return false;
        }
        
        static size_t BUFFERS[] = {
            sizeof(SolidVertexBuffer),
            sizeof(TexelVertexBuffer)
        };

        for(uint8_t b = 0; b < BlendMode_Last; ++b) {
            for(uint8_t i = 0; i < LastPrimitiveType; ++i) {
                PrimitiveType primitiveType = (PrimitiveType)i;
                BlendMode blendMode = (BlendMode)b;
                uint32_t texture = shaderFile == "texture";
                Program* program = g_programs.get(blendMode, primitiveType, texture != 0);
                if(!program->createPipeline(vsShader, fsShader, blendMode, primitiveType, (uint32_t)BUFFERS[texture])) {
                    std::cout << "Failed to create " << getPrimitiveType(primitiveType) << " program to shader " << shaderFile << "." << std::endl;
                    return false;
                }
            }
        }
    }
#endif

    return true;
}
