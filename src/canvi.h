#ifndef CANVI_H
#define CANVI_H

#include <stdint.h>
#include "tigui/tigui.h"
#include "libirecovery/irecovery.h"

tigui_error_t scanning_screen(irecovery_client_t client);
tigui_error_t control_panel_screen(irecovery_client_t client);
tigui_error_t send_var_screen(irecovery_client_t client);
tigui_error_t progress_bar_screen(tigui_canvas_t* canvas, irecovery_client_t client, const char* filename, bool more_after_this);
tigui_error_t send_command_screen(irecovery_client_t client);
tigui_error_t get_env_screen(irecovery_client_t client);
tigui_error_t set_env_screen(irecovery_client_t client);

#endif