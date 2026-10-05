wl_test(test_api
  SRCS
    api_test_main.cc
    test_api.cc
  DEPENDS
    api_health
    base_test
)
