#include "squish_compat.h"
#include "squish.h"

#include <cstddef>
#include <vector>

extern "C" void CompressImage(unsigned char* rgba, int width, int height, void* blocks, int flags)
{
    if (!rgba || !blocks || width <= 0 || height <= 0)
        return;

    const std::size_t pixelCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    std::vector<float> pixels(pixelCount * 4);
    for (std::size_t i = 0; i < pixelCount * 4; ++i)
        pixels[i] = static_cast<float>(rgba[i]) / 255.0f;

    squish::CompressImage(pixels.data(), width, height, blocks, flags);
}

extern "C" void DecompressImage(unsigned char* rgba, int width, int height, void* blocks, int flags)
{
    if (!rgba || !blocks || width <= 0 || height <= 0)
        return;

    const std::size_t pixelCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    std::vector<float> pixels(pixelCount * 4);
    squish::DecompressImage(pixels.data(), width, height, blocks, flags);

    const bool dxt1 = (flags & squish::kDxt1) != 0;
    for (std::size_t i = 0; i < pixelCount; ++i) {
        // libsquish stores decoded RGB components as byte-range floats. DXT3/5
        // alpha is normalized, while DXT1 alpha is already 0 or 255.
        for (std::size_t channel = 0; channel < 4; ++channel) {
            float value = pixels[4 * i + channel];
            if (channel == 3 && !dxt1)
                value *= 255.0f;
            if (value < 0.0f) value = 0.0f;
            if (value > 255.0f) value = 255.0f;
            rgba[4 * i + channel] = static_cast<unsigned char>(value + 0.5f);
        }
    }
}

