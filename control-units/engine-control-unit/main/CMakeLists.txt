# CMakeLists.txt para el directorio main
idf_component_register(SRCS "lvgl_port.c" "waveshare_rgb_lcd_port.c" "main.c"
                    INCLUDE_DIRS ".")
