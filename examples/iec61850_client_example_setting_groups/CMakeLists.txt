
set(iec61850_client_example_setting_groups_SRCS
   client_example_sg.c
)

IF(MSVC)
set_source_files_properties(${iec61850_client_example_setting_groups_SRCS}
                                       PROPERTIES LANGUAGE CXX)
ENDIF(MSVC)

add_executable(iec61850_client_example_sg
  ${iec61850_client_example_setting_groups_SRCS}
)

target_link_libraries(iec61850_client_example_sg
    iec61850
)
