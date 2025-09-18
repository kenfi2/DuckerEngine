#ifndef PROGRAM_H
#define PROGRAM_H

#include <utils/include.h>
#include <utils/color.h>
#include <utils/matrix.h>
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

struct VertexBuffer {
    float x, y;
    float u, v;
    float r, g, b, a;
};

class Window;
class Program {
public:
	Program(SDL_GPUSampleCount sampleCount = SDL_GPU_SAMPLECOUNT_1) : m_sampleCount(sampleCount) { }

	bool createPipeline(const std::unique_ptr<Shaders>& vertexShader, const std::unique_ptr<Shaders>& fragmentShader, PrimitiveType primitiveType, uint32_t pitch);
	void destroy();

	bool link() const { return true; }
	void bind(SDL_GPURenderPass* renderPass);

	template<typename T>
	void setConstantBufferValue(int slot, uint8_t shaderType, T t);

	void pushData(SDL_GPUCommandBuffer* commandBuffer);

private:
	std::vector<CBufferPtr> m_uniforms[LastShaderType];
	SDL_GPUGraphicsPipeline* m_pipeline = nullptr;
	SDL_GPUSampleCount m_sampleCount = SDL_GPU_SAMPLECOUNT_1;
};

class Programs {
public:
	bool init(const std::string& gpuDriver);
	Program* get(PrimitiveType primitiveType, bool texture) {
		return &m_programs[texture][primitiveType];
	}

	void clear() {
		for(int y = 0; y < 2; ++y) {
			for(int i = 0; i < LastPrimitiveType; ++i)
				m_programs[y][i].destroy();
		}
	}

private:
	Program m_programs[2][LastPrimitiveType];
};

extern Programs g_programs;

template <typename T>
inline void Program::setConstantBufferValue(int slot, uint8_t shaderType, T t)
{
	if(slot >= m_uniforms[shaderType].size())
		return;

	m_uniforms[shaderType][slot]->setData(t);
}

#endif
