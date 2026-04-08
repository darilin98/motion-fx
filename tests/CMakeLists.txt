add_executable(effect_tests
        test_ieffectcommon.cpp
        test_morphfilter.cpp
        test_ringmodulator.cpp
        test_saturationexciter.cpp
        testinghelpers.hpp
        test_noisemaker.cpp
)

target_link_libraries(effect_tests
        PRIVATE
        visimusic_dsp
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(effect_tests)