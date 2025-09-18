#ifdef WIN32
#include "shaders.h"

#include <wrl.h>

#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")

#include "../dxc_files/inc/dxcapi.h"
#include "../dxc_files/inc/d3d12shader.h"

std::wstring ConvertToLPCWSTR(const char* str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    
    wchar_t* wide_str = new wchar_t[size_needed];
    
    MultiByteToWideChar(CP_UTF8, 0, str, -1, wide_str, size_needed);
    
    std::wstring wstr = std::wstring(wide_str, size_needed);

    delete[] wide_str;

    return wstr;
}

#pragma comment(lib, "dxcompiler.lib")

#if defined( DEBUG ) || defined( _DEBUG )
#define DXCall(x)                               \
if(FAILED(x)) {                                 \
    char lineNumber[32];                        \
    sprintf_s(lineNumber, "%u", __LINE__);      \
    OutputDebugStringA("Error in: ");           \
    OutputDebugStringA(__FILE__);               \
    OutputDebugStringA("\nLine: ");             \
    OutputDebugStringA(lineNumber);             \
    OutputDebugStringA("\n");                   \
    OutputDebugStringA(#x);                     \
    OutputDebugStringA("\n");                   \
    __debugbreak();                             \
}
#else
#define DXCall(x) x
#endif

bool Shaders::compileD3D(const char* file, bool vertexShader, const char* profile)
{   
    HRESULT hr{ S_OK };

    using namespace Microsoft::WRL;

    ComPtr<IDxcCompiler3> compiler;
    DXCall(hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));
    if(FAILED(hr)) {
        m_error = "Failed to create compiler instance.";
        return false;
    }

    ComPtr<IDxcUtils> utils;
    DXCall(hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));
    if(FAILED(hr)) {
        m_error = "Failed to create utils instance.";
        return false;
    }

    ComPtr<IDxcIncludeHandler> includeHandler;
    DXCall(hr = utils->CreateDefaultIncludeHandler(&includeHandler));
    if(FAILED(hr)) {
        m_error = "Failed to create default include handler";
        return false;
    }

    std::wstring wfile = ConvertToLPCWSTR(file);

    ComPtr<IDxcBlobEncoding> sourceBlob;
    DXCall(hr=utils->LoadFile(wfile.c_str(), nullptr, &sourceBlob));
    if(FAILED(hr)) {
        m_error = "Failed to load file " + std::string(file);
        return false;
    }

    const char* entryPoint;
    if(vertexShader)
        entryPoint = "VSMain";
    else
        entryPoint = "PSMain";

    std::wstring wprofile = ConvertToLPCWSTR(profile);
    std::wstring wentryPoint = ConvertToLPCWSTR(entryPoint);

    LPCWSTR args[]
    {
        wfile.c_str(),
        L"-E", wentryPoint.c_str(),
        L"-T", wprofile.c_str(),
        DXC_ARG_ALL_RESOURCES_BOUND,
#if defined( DEBUG ) || defined( _DEBUG )
        DXC_ARG_DEBUG,
        DXC_ARG_SKIP_OPTIMIZATIONS,
#else
        DXC_ARG_OPTIMIZATION_LEVEL3,
#endif
        DXC_ARG_WARNINGS_ARE_ERRORS,
        L"-Qstrip_reflect",
        L"-Qstrip_debug",
    };

    DxcBuffer buffer{ };
    buffer.Encoding = DXC_CP_ACP;
    buffer.Ptr = sourceBlob->GetBufferPointer();
    buffer.Size = sourceBlob->GetBufferSize();

    ComPtr<IDxcResult> results{ nullptr };
    DXCall(hr = compiler->Compile(&buffer, args, _countof(args), includeHandler.Get(), IID_PPV_ARGS(&results)));
    if(FAILED(hr)) {
        m_error = "Failed to compile shaders.";
        return false;
    }

    ComPtr<IDxcBlobUtf8> errors { nullptr };
    DXCall(hr = results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
    if(FAILED(hr)) {
        m_error = "Failed to get result errors.";
        return false;
    }

    if(errors && errors->GetStringLength() > 0) {
        m_error = std::string(errors->GetStringPointer(), errors->GetStringLength());
        return false;
    }

    HRESULT status{ S_OK };
    DXCall(hr = results->GetStatus(&status));
    if(FAILED(hr) || FAILED(status)) {
        m_error = "Status failed.";
        return false;
    }

    ComPtr<IDxcBlob> shaderBlob { nullptr };
    DXCall(hr = results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr));
    if(FAILED(hr)) {
        m_error = "Failed to get object.";
        return false;
    }

    ComPtr<IDxcBlob> reflectionBlob { nullptr };
    DXCall(hr = results->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionBlob), nullptr));
    if(FAILED(hr)) {
        m_error = "Failed to get reflection.";
        return false;
    }

    DxcBuffer reflectionBuffer{};
    reflectionBuffer.Ptr = reflectionBlob->GetBufferPointer();
    reflectionBuffer.Size = reflectionBlob->GetBufferSize();
    reflectionBuffer.Encoding = 0;

    ComPtr<ID3D12ShaderReflection> reflection{ };
    DXCall(hr = utils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&reflection)));
    if(FAILED(hr)) {
        m_error = "Failed to create reflection.";
        return false;
    }

    m_size = shaderBlob->GetBufferSize();
    uint8_t* shaderBuffer = new uint8_t[m_size];
    m_entryPoint = entryPoint;
    m_compiled = true;
    memcpy(shaderBuffer, shaderBlob->GetBufferPointer(), m_size);
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

    D3D12_SHADER_DESC shaderDesc;
    reflection->GetDesc(&shaderDesc);

    D3D12_SHADER_BUFFER_DESC bufferDesc;
    D3D12_SHADER_VARIABLE_DESC variableDesc;
    ID3D12ShaderReflectionConstantBuffer* constantBuffer;
    ID3D12ShaderReflectionVariable* reflectionVariable;

    D3D12_SHADER_INPUT_BIND_DESC bindDesc;
    for(uint32_t i = 0; i < shaderDesc.BoundResources; i++) {
        reflection->GetResourceBindingDesc(i, &bindDesc);

        if(bindDesc.Type == D3D_SIT_SAMPLER)
            m_shaderCreateInfo.num_samplers++;
    }

    for(uint32_t i = 0; i < shaderDesc.ConstantBuffers; ++i) {
        constantBuffer = reflection->GetConstantBufferByIndex(i);
        constantBuffer->GetDesc(&bufferDesc);

        reflection->GetResourceBindingDescByName(bufferDesc.Name, &bindDesc);

        size_t size = 0;
        std::unordered_map<std::string, size_t> variables;
        for(uint32_t l = 0; l < bufferDesc.Variables; ++l) {
            reflectionVariable = constantBuffer->GetVariableByIndex(l);
            reflectionVariable->GetDesc(&variableDesc);

            ID3D12ShaderReflectionType* varType = reflectionVariable->GetType();
            if(varType) {
                D3D12_SHADER_TYPE_DESC typeDesc;
                varType->GetDesc(&typeDesc);

                variables[variableDesc.Name] = size;
                if(typeDesc.Class == D3D10_SVC_STRUCT)
                    size += addStruct(typeDesc.Name) * typeDesc.Elements;
                else
                    size += addPrimitive(typeDesc.Type) * typeDesc.Rows * typeDesc.Columns;
                if(size == 0)
                    std::cout << "Indefined type " << typeDesc.Name << " " << typeDesc.Type << std::endl;
            }
        }

        m_uniforms[bindDesc.BindPoint] = CBufferPtr(new CBuffer(bindDesc.BindPoint, size, std::move(variables)));

        m_shaderCreateInfo.num_uniform_buffers++;
    }

    if(!vertexShader)
        return true;

    D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
    m_vertexAttributes.resize(shaderDesc.InputParameters);

    int32_t lastMask = -1;
    for(uint32_t i = 0; i < shaderDesc.InputParameters; ++i) {
        reflection->GetInputParameterDesc(i, &paramDesc);

        SDL_GPUVertexAttribute& vertexAttributes = m_vertexAttributes[i];
        
        vertexAttributes.buffer_slot = 0;
        vertexAttributes.location = paramDesc.SemanticIndex;

        if(paramDesc.Mask >= 15) {
            if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
                vertexAttributes.format = SDL_GPU_VERTEXELEMENTFORMAT_INT4;
            else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
                vertexAttributes.format = SDL_GPU_VERTEXELEMENTFORMAT_UINT4;
            else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
                vertexAttributes.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
        } else if(paramDesc.Mask >= 7) {
            if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
                vertexAttributes.format = SDL_GPU_VERTEXELEMENTFORMAT_INT3;
            else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
                vertexAttributes.format = SDL_GPU_VERTEXELEMENTFORMAT_UINT3;
            else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
                vertexAttributes.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
        } else if(paramDesc.Mask >= 3) {
            if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
                vertexAttributes.format = SDL_GPU_VERTEXELEMENTFORMAT_INT2;
            else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
                vertexAttributes.format = SDL_GPU_VERTEXELEMENTFORMAT_UINT2;
            else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
                vertexAttributes.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        } else if(paramDesc.Mask == 1) {
            if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
                vertexAttributes.format = SDL_GPU_VERTEXELEMENTFORMAT_INT;
            else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
                vertexAttributes.format = SDL_GPU_VERTEXELEMENTFORMAT_UINT;
            else if(paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
                vertexAttributes.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT;
        }

        if(lastMask == -1)
            vertexAttributes.offset = 0;
        else if(lastMask >= 15)
            vertexAttributes.offset = sizeof(float) * 4 * i;
        else if(lastMask >= 7)
            vertexAttributes.offset = sizeof(float) * 3 * i;
        else if(lastMask >= 3)
            vertexAttributes.offset = sizeof(float) * 2 * i;
        else if(lastMask == 1)
            vertexAttributes.offset = sizeof(float) * 1 * i;
        lastMask = paramDesc.Mask;
    }

    return true;
}

#endif
