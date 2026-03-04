add_executable(morphfilter_tests
        test_morphfilter.cpp
)

target_link_libraries(morphfilter_tests
        PRIVATE
        motion_fx_dsp
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(morphfilter_tests)