#include "buffermanager.h"

#include <graphics/painter.h>
#include <graphics/shaders.h>

// need to compile shaders founded in SDL/test/testgpu/
// #define USE_PRECOMPILED_SHADERS

#ifdef USE_PRECOMPILED_SHADERS
#include "../android-project/app/jni/SDL/test/testgpu/testgpu_dxil.h"
#include "../android-project/app/jni/SDL/test/testgpu/testgpu_spirv.h"
#include "../android-project/app/jni/SDL/test/testgpu/testgpu_metallib.h"
#endif

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

bool BufferManager::init(const std::string& gpuDriver)
{
    #ifndef USE_PRECOMPILED_SHADERS
    static const std::unordered_map<std::string, std::shared_ptr<BaseShaderHash>> shaderFiles {
        { "cube.hlsl", std::make_shared<ShaderHash<SolidVertex>>() },
        { "texture.hlsl", std::make_shared<ShaderHash<TextureVertex>>() }
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
        ShaderHash<SolidVertex> cc;
        Program* program = g_programs.get(primitiveType, cc.getId());
        if(!program->createPipeline(vsShader, fsShader, primitiveType, (uint32_t)cc.getSize()))
            std::cout << "Failed to create " << getPrimitiveType(primitiveType) << " program to precompiled shader." << std::endl;
    }
#else
    for(const auto& it : shaderFiles) {
        vsShader = std::unique_ptr<Shaders>(new Shaders);
        fsShader = std::unique_ptr<Shaders>(new Shaders);
        
        if(!vsShader->compile(it.first.c_str(), true, gpuDriver) || !fsShader->compile(it.first.c_str(), false, gpuDriver)) {
            std::cout << "Failed to compile shader " << it.first << "." << std::endl;
            continue;
        }
        
        for(uint8_t i = 0; i < LastPrimitiveType; ++i) {
            PrimitiveType primitiveType = (PrimitiveType)i;
            Program* program = g_programs.get(primitiveType, it.second->getId());
            if(!program->createPipeline(vsShader, fsShader, primitiveType, (uint32_t)it.second->getSize())) {
                std::cout << "Failed to create " << getPrimitiveType(primitiveType) << " program to shader " << it.first << "." << std::endl;
                continue;
            }
        }
    }
#endif

    m_chunks.emplace_back();
    return true;
}

void BufferManager::clear()
{
    m_buffers.clear();
    m_chunks.clear();
}

void BufferManager::map()
{
    for(BufferChunk& chunk : m_chunks)
        chunk.map();
}

void BufferManager::upload(SDL_GPUCommandBuffer* commandBuffer)
{
    for(BufferChunk& chunk : m_chunks)
        chunk.upload(commandBuffer);
}

void BufferManager::reset()
{
    m_buffers.clear();
    for(BufferChunk& chunk : m_chunks)
        chunk.clear();
}
