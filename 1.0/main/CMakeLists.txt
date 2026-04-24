set(include_dirs . console fonts web)

set(srcs 
    "main.c"
    "ade7753.c"
    "bl0937.c"
    "console_cb.c"
    "console/console.c"
    "console/console_cb_utils.c"
    "lcd_pico.c"
    "web/dns_server.c"
    "web/time_wrapper.c"
    "web/web_common.c"
    "web/ws.c"
    "web/wifi.c")

FILE(GLOB fonts fonts/*.c)

idf_component_register(
    SRCS "${srcs}" "${fonts}"
    INCLUDE_DIRS "${include_dirs}"
    EMBED_FILES web/page/dist/index.html)