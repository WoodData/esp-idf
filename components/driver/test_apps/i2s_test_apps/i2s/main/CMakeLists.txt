set(srcs "test_app_main.c"
         "test_i2s.c"
         "test_i2s_iram.c")

idf_component_register(SRCS ${srcs}
                       PRIV_INCLUDE_DIRS "../../"
                       WHOLE_ARCHIVE)
