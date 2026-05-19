# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS 
    main.c         # list the source files of this component
    DB_Main.c
    DB_Parameters.c
    Block_FRAM.c
    Block_SPI.c
    Block_Modbus.c
    Block_GPIO.c
    Block_Network.c
    web_server.c

    INCLUDE_DIRS 
    "include"       # optional, add here public include directories    

    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    esp_http_server
    esp_wifi 
    nvs_flash 
    esp_hw_support
    esp_mm
    efuse

    PRIV_REQUIRES       # optional, list the private requirements
)
