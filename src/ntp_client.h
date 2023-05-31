#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwipopts.h"
#include <time.h>
#define PICO_CYW43_ARCH_POLL 1
void run_ntp_test(void) ;
int init_wifi();
int deinit_wifi();
extern bool time_updated;
extern time_t utc_time;
int ntp_task(void);
