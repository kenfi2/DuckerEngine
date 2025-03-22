#ifndef DRAWBUFFER_H
#define DRAWBUFFER_H

#include <utils/include.h>

class Program;
class BufferData;

class DrawBuffer {
public:
	DrawBuffer(uint32_t size);
	~DrawBuffer();

	SDL_GPUColorTargetInfo getColorInfo() { return m_colorInfo; }

    void setInstances(std::vector<uint32_t>&& instances) { m_instances = instances; }

	SDL_GPUBuffer* getBuffer() const { return m_buffer; }

	void draw(SDL_GPURenderPass* renderPass, const BufferData& buffer) const;

private:
    static int drawBufferId;

    SDL_GPUColorTargetInfo m_colorInfo;
    std::vector<uint32_t> m_instances;
    SDL_GPUBuffer* m_buffer = nullptr;
};

#endif
