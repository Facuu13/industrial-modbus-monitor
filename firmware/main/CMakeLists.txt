idf_component_register(
    SRCS "firmware.c"
         "rs485.c"
         "modbus_crc.c"
         "modbus_rtu.c"
         "modbus_sim.c"
         "motor_model.c"
         "modbus_poll.c"
         "transport_sim.c"
         "transport_rs485.c"
    INCLUDE_DIRS "."
)

