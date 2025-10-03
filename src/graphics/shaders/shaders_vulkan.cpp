#include "shaders.h"

#include <shaderc/shaderc.hpp>
#include <spirv_reflect.h>
#include <algorithm>

bool Shaders::compileVulkan(const char* file, bool vertexShader, const char* /* profile */, const char* /* sourceName */)
{
    FILE* f = fopen(file, "r");
    if(!f) {
        std::cout << "Cannot open shaders file: " << file << "." << std::endl;
        return false;
    }

    static std::vector<char> data;
    fseek(f, 0, SEEK_END);
    data.resize(ftell(f));
    fseek(f, 0, SEEK_SET);
    fread(data.data(), 1, data.size(), f);
    fclose(f);

    if(data.size() == 0) {
        std::cout << "Failed to compile shaders, maybe empty file. " << file << "." << std::endl;
        return false;
    }

    shaderc::CompileOptions options;
    shaderc_shader_kind kind;
    if(vertexShader) {
        kind = shaderc_glsl_vertex_shader;
        options.AddMacroDefinition("VERTEX");
    }
    else
        kind = shaderc_glsl_fragment_shader;
#if defined( DEBUG ) || defined( _DEBUG )
    options.SetOptimizationLevel(shaderc_optimization_level_zero);
    options.SetGenerateDebugInfo();
#else
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif

    shaderc::Compiler compiler;

    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(data.data(), data.size(), kind, file, options);
    if(result.GetCompilationStatus() != shaderc_compilation_status_success) {
        m_error = result.GetErrorMessage();
        return false;
    }

    const uint32_t* src = result.cbegin();
    size_t size = (result.cend() - src) * sizeof(uint32_t);

    m_size = size;
    uint8_t* shaderBuffer = new uint8_t[size];
    m_entryPoint = "main";
    m_compiled = true;
    memcpy(shaderBuffer, src, size);
    m_buffer = shaderBuffer;

    SDL_zero(m_shaderCreateInfo);
    
    m_shaderCreateInfo.format = getFormat();
    m_shaderCreateInfo.code = m_buffer;
    m_shaderCreateInfo.code_size = m_size;
    m_shaderCreateInfo.entrypoint = m_entryPoint.c_str();

    m_shaderCreateInfo.stage = vertexShader ? SDL_GPU_SHADERSTAGE_VERTEX : SDL_GPU_SHADERSTAGE_FRAGMENT;

    // isn't good enough :(
    // SDL_PropertiesID props = SDL_CreateProperties();
    // SDL_SetStringProperty(props, SDL_PROP_GPU_DEVICE_CREATE_D3D12_SEMANTIC_NAME_STRING, "TEXCOORD");
    m_shaderCreateInfo.props = 0;

    SpvReflectShaderModule module;
    SpvReflectResult reflectionResult = spvReflectCreateShaderModule(m_size, (uint32_t*)m_buffer, &module);
    if(reflectionResult != SPV_REFLECT_RESULT_SUCCESS) {
        m_error = "Falha ao carregar SPIR-V para reflexão!";
        return false;
    }

    uint32_t binding_count = 0;
    spvReflectEnumerateDescriptorBindings(&module, &binding_count, nullptr);
    
    std::vector<SpvReflectDescriptorBinding*> bindings(binding_count);
    spvReflectEnumerateDescriptorBindings(&module, &binding_count, bindings.data());

    uint32_t uniform_buffer_count = 0;
    uint32_t sampler_count = 0;

    m_uniforms.resize(binding_count);
    for(uint32_t i = 0; i < binding_count; ++i) {
        SpvReflectDescriptorBinding* binding = bindings[i];
        
        uniform_buffer_count++;
        
        SpvReflectBlockVariable* block = &binding->block;
        
        size_t buffer_size = block->size;
        std::unordered_map<std::string, size_t> variables;
        
        for(uint32_t j = 0; j < block->member_count; ++j) {
            SpvReflectBlockVariable* member = &block->members[j];
            variables[member->name] = member->offset;
        }
        
        m_uniforms[binding->binding] = CBufferPtr(new CBuffer(binding->binding, buffer_size, std::move(variables)));
        if(binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER ||
                binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
            sampler_count++;
    }

    m_shaderCreateInfo.num_uniform_buffers = uniform_buffer_count;
    m_shaderCreateInfo.num_samplers = sampler_count;

    if(vertexShader) {
        uint32_t input_count = 0;
        spvReflectEnumerateInputVariables(&module, &input_count, nullptr);

        std::vector<SpvReflectInterfaceVariable*> input_variables(input_count);
        spvReflectEnumerateInputVariables(&module, &input_count, input_variables.data());

        std::vector<SpvReflectInterfaceVariable*> valid_inputs;
        for(uint32_t i = 0; i < input_count; ++i) {
            if(input_variables[i] && input_variables[i]->location != UINT32_MAX)
                valid_inputs.push_back(input_variables[i]);
        }

        std::sort(valid_inputs.begin(), valid_inputs.end(), [](SpvReflectInterfaceVariable* a, SpvReflectInterfaceVariable* b) {
            return a->location < b->location;
        });

        m_vertexAttributes.resize(valid_inputs.size());

        size_t current_offset = 0;
        for(uint32_t i = 0; i < valid_inputs.size(); ++i) {
            SpvReflectInterfaceVariable* variable = valid_inputs[i];

            SDL_GPUVertexAttribute& vertexAttribute = m_vertexAttributes[i];
            vertexAttribute.buffer_slot = 0;
            vertexAttribute.location = variable->location;
            
            switch(variable->format) {
                case SPV_REFLECT_FORMAT_R32_SFLOAT:             vertexAttribute.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT; break;
                case SPV_REFLECT_FORMAT_R32G32_SFLOAT:          vertexAttribute.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2; break;
                case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:       vertexAttribute.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3; break;
                case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:    vertexAttribute.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4; break;
                case SPV_REFLECT_FORMAT_R32_UINT:               vertexAttribute.format = SDL_GPU_VERTEXELEMENTFORMAT_UINT; break;
                case SPV_REFLECT_FORMAT_R32G32_UINT:            vertexAttribute.format = SDL_GPU_VERTEXELEMENTFORMAT_UINT2; break;
                case SPV_REFLECT_FORMAT_R32G32B32_UINT:         vertexAttribute.format = SDL_GPU_VERTEXELEMENTFORMAT_UINT3; break;
                case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:      vertexAttribute.format = SDL_GPU_VERTEXELEMENTFORMAT_UINT4; break;
                case SPV_REFLECT_FORMAT_R32_SINT:               vertexAttribute.format = SDL_GPU_VERTEXELEMENTFORMAT_INT; break;
                case SPV_REFLECT_FORMAT_R32G32_SINT:            vertexAttribute.format = SDL_GPU_VERTEXELEMENTFORMAT_INT2; break;
                case SPV_REFLECT_FORMAT_R32G32B32_SINT:         vertexAttribute.format = SDL_GPU_VERTEXELEMENTFORMAT_INT3; break;
                case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:      vertexAttribute.format = SDL_GPU_VERTEXELEMENTFORMAT_INT4; break;
                default:                                        vertexAttribute.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT; break;
            }

            if(i == 0)
                vertexAttribute.offset = 0;
            else {
                switch(m_vertexAttributes[i-1].format) {
                    case SDL_GPU_VERTEXELEMENTFORMAT_FLOAT:     current_offset += sizeof(float); break;
                    case SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2:    current_offset += sizeof(float) * 2; break;
                    case SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3:    current_offset += sizeof(float) * 3; break;
                    case SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4:    current_offset += sizeof(float) * 4; break;
                    case SDL_GPU_VERTEXELEMENTFORMAT_INT:       current_offset += sizeof(int32_t); break;
                    case SDL_GPU_VERTEXELEMENTFORMAT_INT2:      current_offset += sizeof(int32_t) * 2; break;
                    case SDL_GPU_VERTEXELEMENTFORMAT_INT3:      current_offset += sizeof(int32_t) * 3; break;
                    case SDL_GPU_VERTEXELEMENTFORMAT_INT4:      current_offset += sizeof(int32_t) * 4; break;
                    case SDL_GPU_VERTEXELEMENTFORMAT_UINT:      current_offset += sizeof(uint32_t); break;
                    case SDL_GPU_VERTEXELEMENTFORMAT_UINT2:     current_offset += sizeof(uint32_t) * 2; break;
                    case SDL_GPU_VERTEXELEMENTFORMAT_UINT3:     current_offset += sizeof(uint32_t) * 3; break;
                    case SDL_GPU_VERTEXELEMENTFORMAT_UINT4:     current_offset += sizeof(uint32_t) * 4; break;
                    default:                                    current_offset += sizeof(float) * 4; break;
                }
                vertexAttribute.offset = (uint32_t)current_offset;
            }
        }
    }

    spvReflectDestroyShaderModule(&module);
    return true;
}
