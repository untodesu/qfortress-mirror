#include "tools/geomp/pch.hh"

#include "core/cmdline.hh"
#include "core/exceptions.hh"
#include "core/paths.hh"
#include "core/utils/physfs.hh"

static void qfgeomp_main(void)
{
    LOG_INFO("qfortress geometry processor [geomp]");
}

static void wrapped_main(int argc, char** argv)
{
    uulog::add_sink(&uulog::builtin::stderr_ansi);

    cmdline::create(argc, argv);

    auto physfs_init_ok = PHYSFS_init(argv[0]);
    qf::throw_if_not_fmt<std::runtime_error>(physfs_init_ok, "failed to initialize physfs: {}", utils::physfs_error());

    paths::init();

    qfgeomp_main();

    auto physfs_deinit_ok = PHYSFS_deinit();
    qf::throw_if_not_fmt<std::runtime_error>(physfs_deinit_ok, "failed to de-initialize physfs: {}", utils::physfs_error());
}

int main(int argc, char** argv)
{
    try {
        wrapped_main(argc, argv);
        return EXIT_SUCCESS;
    }
    catch(const std::exception& ex) {
        std::cerr << argv[0] << ": " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch(...) {
        std::cerr << argv[0] << ": non-std::exception throw" << std::endl;
        return EXIT_FAILURE;
    }
}
