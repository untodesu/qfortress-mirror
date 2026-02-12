#include "client_modern/pch.hh"

#include "client/renderer.hh"

#include "core/cmdline.hh"
#include "core/exceptions.hh"

#include "client_modern/experimental.hh"
#include "client_modern/globals.hh"

void renderer::init(void)
{
    experimental::init();
}

void renderer::init_late(void)
{
    experimental::init_late();
}

void renderer::shutdown(void)
{
    experimental::shutdown_early();
}

void renderer::update(void)
{
    experimental::update();
}

void renderer::update_late(void)
{
    experimental::update_late();
}

void renderer::render(void)
{
    experimental::render();
}

void renderer::layout(void)
{
    // empty
}
