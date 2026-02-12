#ifndef CLIENT_VIDEO_HH
#define CLIENT_VIDEO_HH
#pragma once

class WindowResizeEvent final {
public:
    constexpr explicit WindowResizeEvent(int width, int height) noexcept;
    constexpr int width(void) const noexcept;
    constexpr int height(void) const noexcept;

private:
    int m_width;
    int m_height;
};

namespace video
{
extern int width;
extern int height;
extern float aspect;
} // namespace video

namespace video
{
void init(void);
void init_late(void);
void shutdown(void);
} // namespace video

constexpr WindowResizeEvent::WindowResizeEvent(int width, int height) noexcept : m_width(width), m_height(height)
{
    // empty
}

constexpr int WindowResizeEvent::width(void) const noexcept
{
    return m_width;
}

constexpr int WindowResizeEvent::height(void) const noexcept
{
    return m_height;
}

#endif
