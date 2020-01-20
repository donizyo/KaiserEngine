#pragma once

#include "Image.h"

class GLTexture
{
private:
    GLuint id{ 0 };
public:
    GLTexture();
    ~GLTexture();

    const bool IsTexture() const;
    void ActiveTexture(const GLenum& texture) const;
    void Bind(const GLenum& target) const;
    void SetImage(const Image& image);
};
