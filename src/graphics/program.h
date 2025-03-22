#ifndef PROGRAM_H
#define PROGRAM_H

#include <utils/include.h>
#include <unordered_map>

#include "shaders.h"

class Window;
class Program {
public:
	Program(SDL_GPUSampleCount sampleCount = SDL_GPU_SAMPLECOUNT_1) : m_sampleCount(sampleCount) { }

	bool createPipeline(const std::unique_ptr<Shaders>& vertexShader, const std::unique_ptr<Shaders>& fragmentShader, PrimitiveType primitiveType, uint32_t pitch);
	void destroy();

	void bind(SDL_GPURenderPass* renderPass);

	void pushData(SDL_GPUCommandBuffer* cmdBuffer, uint32_t slot, const void* data, uint32_t length);

private:
	SDL_GPUGraphicsPipeline* m_pipeline;
	SDL_GPUSampleCount m_sampleCount;
};

class Programs {
public:
	Program* get(PrimitiveType primitiveType, size_t index) { return &m_programs[primitiveType][index]; }
	void clear() {
		for(int i = 0; i < LastPrimitiveType; ++i) {
			for(auto& it : m_programs[i])
				it.second.destroy();
			m_programs[i].clear();
		}
	}

private:
	std::unordered_map<size_t, Program> m_programs[LastPrimitiveType];
};

extern Programs g_programs;

#endif
