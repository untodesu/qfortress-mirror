#include "render/modern/pch.hh"

#include "render/frontend.hh"

#include "core/cmdline.hh"
#include "core/exceptions.hh"

#include "render/modern/experimental.hh"
#include "render/modern/globals.hh"

void render_frontend::init(void)
{
    experimental::init();
}

void render_frontend::init_late(void)
{
    experimental::init_late();
}

void render_frontend::shutdown(void)
{
    experimental::shutdown_early();
}

void render_frontend::update(void)
{
    experimental::update();
}

void render_frontend::update_late(void)
{
    experimental::update_late();
}

void render_frontend::render(void)
{
    experimental::render();
}

void render_frontend::layout(void)
{
    // empty
}
