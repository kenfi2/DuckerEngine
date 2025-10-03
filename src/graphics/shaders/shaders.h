#ifndef SHADERS_H
#define SHADERS_H

#include <utils/include.h>

#include <unordered_map>
#include <typeindex>

enum PrimitiveType : uint8_t {
    PrimitiveTypeTriangleList,
    PrimitiveTypeTriangleStrip,
    PrimitiveTypeLineList,
    PrimitiveTypeLineStrip,
    PrimitiveTypePointList,

    LastPrimitiveType
};

enum UniformType {
    UniformTypeBool = 1,
    UniformTypeInt,
    UniformTypeFloat
};

class CBuffer {
public:
    CBuffer(uint32_t slot, size_t size, std::unordered_map<std::string, size_t>&& variables) :
        m_slot(slot), m_hasChanged(true)
    {
        m_data.resize(size);
        m_variables = variables;
    }

    const char* getData() const { return m_data.data(); }
    uint32_t getSize() const { return (uint32_t)m_data.size(); }
    uint32_t getSlot() const { return m_slot; }
    size_t getVariableCount() const { return m_variables.size(); }

    const std::unordered_map<std::string, size_t>& getVariables() const { return m_variables; }

    bool hasVariable(const std::string& variable) const { return m_variables.find(variable) != m_variables.end(); }

    template<typename T>
    void setData(const T& v) {
        memcpy(&m_data[0], &v, sizeof(T));
        m_hasChanged = true;
    }

    template<typename T>
    void setValue(size_t offset, const T& v) {
        T& value = reinterpret_cast<T&>(m_data[offset]);
        value = v;
        m_hasChanged = true;
    }

    template<typename T>
    void setValue(const std::string& name, const T& v) {
        auto it = m_variables.find(name);
        if(it == m_variables.end())
            return;

        T& value = reinterpret_cast<T&>(m_data[it->second]);
        value = v;
        m_hasChanged = true;
    }

    bool hasChanged() const { return m_hasChanged; }
    void setUnchanged() { m_hasChanged = false; }

private:
    std::vector<char> m_data;
    std::unordered_map<std::string, size_t> m_variables;
    uint32_t m_slot;
    bool m_hasChanged;
};

using CBufferPtr = std::shared_ptr<CBuffer>;

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

    bool load(const std::string& file, bool vertexShader, const std::string& device);
    bool compile(const std::string& data, const std::string& name, bool vertexShader, const std::string& device);

    bool bind(SDL_GPUDevice* device);

    void createPreCompiledShaderInfo(uint32_t uniformBuffer = 0);

    std::string getError() const { return m_error; }
    std::string getEntryPoint() const { return m_entryPoint; }
    SDL_GPUShader* getShader() const { return m_gpuShader.shader; }
    const uint8_t* getBuffer() const { return m_buffer; }
    size_t getSize() const { return m_size; }

    SDL_GPUShaderFormat getFormat() const;

    const std::vector<SDL_GPUVertexAttribute>& getVertexAttributes() const { return m_vertexAttributes; }
    const auto& getUniforms() const { return m_uniforms; }

    static std::string mainVertexShader;

protected:
    bool compileD3D(const char* data, bool vertexShader, const char* profile, const char* sourceName = nullptr);
    bool compileVulkan(const char* data, bool vertexShader, const char* profile, const char* sourceName = nullptr);
    bool compileMetal(const char* data, bool vertexShader, const char* profile, const char* sourceName = nullptr);

    size_t addStruct(const std::string& typeName);
    size_t addPrimitive(int type);

private:
    std::vector<CBufferPtr> m_uniforms;
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
