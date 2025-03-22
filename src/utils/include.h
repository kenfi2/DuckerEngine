#ifndef INCLUDE_H
#define INCLUDE_H

#define _CRT_SECURE_NO_WARNINGS

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <memory>
#include <vector>
#include <iostream>
#include <sstream>
#include <chrono>

class Image;
using ImagePtr = std::shared_ptr<Image>;

class Texture;
using TexturePtr = std::shared_ptr<Texture>;

#endif
