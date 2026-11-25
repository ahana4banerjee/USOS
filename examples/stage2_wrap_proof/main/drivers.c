#include <stddef.h>

int DHT_Read(float *temperature)
{
    if (temperature == NULL) {
        return -1;
    }
    *temperature = 25.4f;
    return 0;
}

int Sensor_UnwrappedPing(void)
{
    return 42;
}
