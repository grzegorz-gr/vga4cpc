cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(vga4cpc-project C CXX ASM)

pico_sdk_init()

add_executable(vga4cpc)

# must match with pio filename and executable name from above
pico_generate_pio_header(vga4cpc ${CMAKE_CURRENT_LIST_DIR}/pioasm/hsync_50.pio)
pico_generate_pio_header(vga4cpc ${CMAKE_CURRENT_LIST_DIR}/pioasm/vsync_50.pio)
pico_generate_pio_header(vga4cpc ${CMAKE_CURRENT_LIST_DIR}/pioasm/rgb_50.pio)
pico_generate_pio_header(vga4cpc ${CMAKE_CURRENT_LIST_DIR}/pioasm/hsync_60.pio)
pico_generate_pio_header(vga4cpc ${CMAKE_CURRENT_LIST_DIR}/pioasm/vsync_60.pio)
pico_generate_pio_header(vga4cpc ${CMAKE_CURRENT_LIST_DIR}/pioasm/rgb_60.pio)
pico_generate_pio_header(vga4cpc ${CMAKE_CURRENT_LIST_DIR}/pioasm/rgbin.pio)
pico_generate_pio_header(vga4cpc ${CMAKE_CURRENT_LIST_DIR}/pioasm/vsyncgen.pio)

target_sources(vga4cpc PRIVATE main.c test_screen.c test_screen.h vga_pio.c vga_pio.h cpc_pio.c cpc_pio.h constants.h mode_switch.c mode_switch.h vga_dma.c vga_dma.h cpc_dma.c cpc_dma.h)
target_link_libraries(vga4cpc PRIVATE pico_stdlib hardware_pio hardware_dma)

pico_add_extra_outputs(vga4cpc)
