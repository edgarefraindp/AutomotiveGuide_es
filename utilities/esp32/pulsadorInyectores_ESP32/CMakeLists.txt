# Proyecto de Pulsador de Inyectores para ESP32 utilizando ESP-IDF
# 
# Este CMakeLists.txt configura el proyecto ESP-IDF para simular
# señales de pulso para 4 inyectores de combustible.
#
# For support or to donate: Se debe visitar la página del repositorio en GitHub
# Repository: https://github.com/edgarefraindp/AutomotiveGuide_es

cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(pulsadorInyectores_ESP32)