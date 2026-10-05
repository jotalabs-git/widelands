wl_library(api_health
  SRCS
    health_server.cc
    health_server.h
  DEPENDS
    base
    build_info
)
