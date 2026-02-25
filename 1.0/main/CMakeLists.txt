set(include_dirs . console fonts web)

set(srcs 
    "main.c"
    "ade7753.c"
    "bl0937.c"
    "console_cb.c"
    "console/console.c"
    "console/console_cb_utils.c"
    "fonts.c"
    "lcd_pico.c"
    "fonts/font12.c"
    "fonts/font16.c"
    "fonts/font20.c"
    "fonts/font24.c"
    "fonts/font8.c"
    "web/dns_server.c"
    "web/time_wrapper.c"
    "web/web_common.c"
    "web/ws.c"
    "web/wifi.c")

idf_component_register(
    SRCS "${srcs}" 
    INCLUDE_DIRS "${include_dirs}"
    EMBED_FILES web/page/dist/index.html)