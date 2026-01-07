#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "yrm100/yrm100.h"
#include "yrm100/yrm100_string.h"
#include "yrm100/yrm100_command.h"
#include "yrm100/yrm100_util.h"
#include "yrm100/yrm100_print.h"
#include "yrm100/yrm100_error.h"

int main()
{
#ifdef _WIN32
    const char *port_name = "COM7";
#else
    const char *port_name = "/dev/ttyUSB0";
#endif

    printf("Serial port: %s \n", port_name);
    yrm100_context_t *device = yrm100_init(port_name);
    if (device == NULL)
    {
        printf("yrm100_init failed\n");
        return 1;
    }

    char manufacturer_buf[50];
    char software_version_buf[50];
    char hardware_version_buf[50];

    yrm100_zero_buf(manufacturer_buf, sizeof(manufacturer_buf));
    yrm100_zero_buf(hardware_version_buf, sizeof(hardware_version_buf));
    yrm100_zero_buf(software_version_buf, sizeof(software_version_buf));

    int result = yrm100_command_get_module_manufacturer(device, manufacturer_buf, sizeof(manufacturer_buf));
    if (result < 0)
    {
        printf("get module manufacturer failed: %s (%d)\n", yrm100_error_code_to_string(result), result);
    }

    result = yrm100_command_get_module_hardware_version(device, hardware_version_buf, sizeof(hardware_version_buf));
    if (result < 0)
    {
        printf("get module hardware version failed: %s (%d)\n", yrm100_error_code_to_string(result), result);
    }

    result = yrm100_command_get_module_software_version(device, software_version_buf, sizeof(software_version_buf));
    if (result < 0)
    {
        printf("get module software version failed: %s (%d)\n", yrm100_error_code_to_string(result), result);
    }

    if (manufacturer_buf[0] != '\0' && hardware_version_buf[0] != '\0' && software_version_buf[0] != '\0')
    {
        printf("%s %s / FW: %s\n", manufacturer_buf, hardware_version_buf, software_version_buf);
    }

    yrm100_command_disable_idle_sleep(device);

    //    yrm100_command_enable_continous_wave(device);
    yrm100_command_disable_continous_wave(device);

    yrm100_command_set_operating_region(device, YRM100_PARAM_REGION_CHINA_900);

    int set_select_mode_result = yrm100_command_set_select_mode(device, YRM100_PARAM_SELECT_MODE_DONT_SEND_BEFORE_ANY_OPERATIONS);
    printf("yrm100_command_set_select_mode(%i) -> %i\n", YRM100_PARAM_SELECT_MODE_DONT_SEND_BEFORE_ANY_OPERATIONS, set_select_mode_result);

    //    yrm100_command_get_operating_region(port);

    char tx[YRM100_PARAM_TX_POWER_STRING_LENGTH];
    result = yrm100_command_get_tx_power(device);
    if (result < 0)
    {
        printf("get TX power failed: %s (%d)\n", yrm100_error_code_to_string(result), result);
    }
    else
    {
        printf("TX power: %s\n", yrm100_convert_to_tx_power_string((unsigned int)result, tx));
    }

    result = yrm100_command_get_operating_region(device);
    if (result < 0)
    {
        printf("get operating region failed: %s (%d)\n", yrm100_error_code_to_string(result), result);
    }
    else
    {
        printf("Region: %s\n", yrm100_convert_to_region_string((unsigned int)result));
    }

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
    result = yrm100_command_single_poll(device, tag, 5);
    if (result < 0)
    {
        printf("single poll failed: %s (%d)\n", yrm100_error_code_to_string(result), result);
    }
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
