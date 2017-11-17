#include "common.h"

sstring server_status() {
    sstring res =  sstring_empty();

    sstring_cat(&res, "------------server status--------------\n");
    sstring_cat(&res, "---------server memory used------------\n");
    sstring_vscat(&res, "smalloc: %d\n", dump_used_bytes());
    sstring_cat(&res, "-------------client detail------------\n");
    sstring_vscat(&res, "active_client:%d, total_client:%d\n", server.active_client_num, server.total_client_num);

    return res;
}
