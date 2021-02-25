set(SRC
#${CMAKE_CURRENT_SOURCE_DIR}/provision_base.c
#${CMAKE_CURRENT_SOURCE_DIR}/provisioner.c
#${CMAKE_CURRENT_SOURCE_DIR}/unprov_device.c
${CMAKE_CURRENT_SOURCE_DIR}/provision_security.c
${CMAKE_CURRENT_SOURCE_DIR}/provision_comm.c
${CMAKE_CURRENT_SOURCE_DIR}/provision_fsm_comm.c
${CMAKE_CURRENT_SOURCE_DIR}/unprov_device_fsm.c
${CMAKE_CURRENT_SOURCE_DIR}/unprov_device_intf.c
${CMAKE_CURRENT_SOURCE_DIR}/provision.c
${CMAKE_CURRENT_SOURCE_DIR}/provisioner_fsm.c
${CMAKE_CURRENT_SOURCE_DIR}/provisioner_intf.c
${CMAKE_CURRENT_SOURCE_DIR}/advertising_interfaces.c
${CMAKE_CURRENT_SOURCE_DIR}/unprov_adv_interfaces.c
${CMAKE_CURRENT_SOURCE_DIR}/provision_service_adv_interfaces.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(api)
