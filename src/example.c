#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "yrm100/yrm100.h"
#include "yrm100/yrm100_string.h"
#include "yrm100/yrm100_command.h"
#include "yrm100/yrm100_util.h"
#include "yrm100/yrm100_print.h"

int main()
{
#ifdef _WIN32
    const char *port_name = "COM7";
#else
    const char *port_name = "/dev/ttyUSB0";
#endif

    printf("Serial port: %s \n", port_name);
    yrm100_context_t *device = yrm100_init(port_name);
    yrm100_print_module_info(device);
    yrm100_command_disable_idle_sleep(device);

    //    yrm100_command_enable_continous_wave(device);
    yrm100_command_disable_continous_wave(device);

    yrm100_command_set_operating_region(device, YRM100_PARAM_REGION_CHINA_900);

    int set_select_mode_result = yrm100_command_set_select_mode(device, YRM100_PARAM_SELECT_MODE_DONT_SEND_BEFORE_ANY_OPERATIONS);
    printf("yrm100_command_set_select_mode(%i) -> %i\n", YRM100_PARAM_SELECT_MODE_DONT_SEND_BEFORE_ANY_OPERATIONS, set_select_mode_result);

    //    yrm100_command_get_operating_region(port);

    char tx[YRM100_PARAM_TX_POWER_STRING_LENGTH];
    printf("TX power: %s\n", yrm100_command_get_tx_power_string(device, tx));
    printf("Region: %s\n", yrm100_convert_to_region_string((unsigned int)yrm100_command_get_operating_region(device)));

    /*
        char power[YRM100_PARAM_TX_POWER_STRING_LENGTH];
        printf("TX power: %s\n",yrm100_get_tx_power_string(port, power));
    */
    //    rfid_set_idle_sleep_time(port,0);

    /*
        rfid_get_module_manufacturer(port, string_buf, 255);
        printf("RFID module: %s ", string_buf);
        rfid_get_module_hardware_version(port, string_buf, 255);
        printf("%s ", string_buf);
        rfid_get_module_software_version(port, string_buf, 255);
        printf("(firmware: %s)\n", string_buf);
    */

    rfid_tag_t tag[5];
    yrm100_command_single_poll(device, tag, 5);
    for (int i = 0; i < 5; i++)
    {
        yrm100_print_tag_info(&tag[i]);
    }

    /*
        rfid_tag_t tag[100];
        yrm100_command_multi_poll_start(device, tag, 5);
        sleep(1);
        yrm100_command_multi_poll_continue(device);
        for (unsigned long i = 0; i < 5; i++)
        {
            yrm100_print_tag_info(&tag[i]);
        }
        yrm100_command_multi_poll_stop(device);
    */
    yrm100_deinit(device);
    return 0;
}
