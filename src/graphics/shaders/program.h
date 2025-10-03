#ifndef PROGRAM_H
#define PROGRAM_H

#include <utils/include.h>
#include <utils/color.h>
#include <utils/matrix.h>
#include <utils/size.h>

#include <unordered_map>
#include <boost/any.hpp>

#include "shaders.h"

enum {
	VertexShader,
	FragmentShader,
	LastShaderType
};

enum ShaderFeatures {
    ShaderFeature_Color = 1,
    ShaderFeature_Texture0 = 2,
    ShaderFeature_Time = 4,
    ShaderFeature_Resolution = 8,
    ShaderFeature_Size = 16,
    ShaderFeature_VertexColor = 32,
    ShaderFeature_TextureScale = 64,
    ShaderFeature_Texture1 = 128,
    ShaderFeature_Texture2 = 256,
    ShaderFeature_Level = 512,
    ShaderFeature_TexCoord = 1024,
    ShaderFeature_GlobalTime = 2048,
    ShaderFeature_Position = 4096,
    ShaderFeature_RectSize = 8192,
    ShaderFeature_RectOffset = 16384
};

struct SolidVertexBuffer {
	float x, y;
};

struct TexelVertexBuffer {
    float x, y;
    float u, v;
    float r, g, b, a;
};

class Window;
class Program {
	union Uniform {
		uint64_t value;
		struct {
			uint32_t location;
			uint16_t slot;
			uint16_t type;
		} data;
	};

public:
    enum {
        MAX_UNIFORM_LOCATIONS = 30
    };

	enum {
		VERTEX_ATTR = 0,
		TEXCOORD_ATTR,
		VERTEXCOLOR_ATTR
	};

	enum {
		PROJECTIONTRANSFORM_MATRIX_UNIFORM,
		COLOR_UNIFORM,
		RESOLUTION_UNIFORM,
		SIZE_UNIFORM,
		TIME_UNIFORM,
		GLOBAL_TIME_UNIFORM,
		LEVEL_UNIFORM,
		POSITION_UNIFORM,
		RECT_SIZE_UNIFORM,
		RECT_OFFSET_UNIFORM,
		TEXTURE_SCALE_UNIFORM,
		TEX0_UNIFORM,
		TEX1_UNIFORM,
		TEX2_UNIFORM,
		DYNAMIC_UNIFORM_BEGIN,
	};

	Program(SDL_GPUSampleCount sampleCount = SDL_GPU_SAMPLECOUNT_1) : m_sampleCount(sampleCount) { }

	bool createPipeline(const std::unique_ptr<Shaders>& vertexShader, const std::unique_ptr<Shaders>& fragmentShader, BlendMode blendMode, PrimitiveType primitiveType, uint32_t pitch);
	void destroy();

	bool link() const { return true; }
	void bind(SDL_GPURenderPass* renderPass);

	template<typename T>
	void setUniform(size_t index, const T& value);

	template<typename T>
	void setUniform(const std::string& variable, const T& value);

	void createShaderProgram(const std::string& vertexShader, const std::string& fragmentShader, uint32_t features);

	void setProjectionTransformMatrix(const Matrix3& projectionTransformMatrix);
	void setResolution(const SizeI& resolution);
    void setTextureSize(const SizeI& textureSize);
    void setSize(float size);
    void setLevel(float level);
    void setColor(const Color& color);
    void setTime(float time);
    // void setPosition(Point3F position);
    void setRectSize(const SizeF& rectOffset);
    void setRectOffset(const PointF& rectOffset);

	void pushData(SDL_GPUCommandBuffer* commandBuffer);

private:
	void bindUniformLocation(size_t index, const std::string& variable);

	std::vector<CBufferPtr> m_uniforms[LastShaderType];
	std::array<Uniform, MAX_UNIFORM_LOCATIONS> m_uniformLocations;
	SDL_GPUGraphicsPipeline* m_pipeline = nullptr;
	SDL_GPUSampleCount m_sampleCount = SDL_GPU_SAMPLECOUNT_1;
	uint32_t m_features = 0;
};

class Programs {
public:
	bool init(const std::string& gpuDriver);
	Program* get(BlendMode blendMode, PrimitiveType primitiveType, bool texture) {
		size_t index = ((size_t)blendMode * LastPrimitiveType + (size_t)primitiveType) * 2 + texture;
		return &m_programs[index];
	}

	void clear() {
		for(size_t i = 0; i < SDL_arraysize(m_programs); ++i)
			m_programs[i].destroy();
	}

private:
	Program m_programs[BlendMode_Last * LastPrimitiveType * 2];
};

extern Programs g_programs;

template<typename T>
inline void Program::setUniform(size_t index, const T& value)
{
	Uniform& uniform = m_uniformLocations[index];

	m_uniforms[uniform.data.type][uniform.data.slot]->setValue(uniform.data.location, value);
}

template <typename T>
inline void Program::setUniform(const std::string &variable, const T &value)
{
	for(const CBufferPtr& uniform : m_uniforms[FragmentShader])
		uniform->setValue(variable, value);
}

#endif
