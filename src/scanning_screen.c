#include "tigui/tigui.h"
#include "libirecovery/irecovery.h"

/* Actions/Callbacks (Forward Declarations) */
bool scanning_screen_device_poller(void* user_data);

/* Elements */
tigui_text_t scanning_screen_text = {
    .text = "Scanning for devices...",
    .reference_point = TIGUI_TEXT_CENTER_POINT,
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = (TIGUI_SCREEN_HEIGHT / 2) - 45,
    .x_clip = TIGUI_SCREEN_WIDTH,
    .y_clip = TIGUI_SCREEN_HEIGHT,
    .color = TIGUI_BLACK,
    .wrap = false,
    .update = true
};
tigui_spinner_t scanning_screen_spinner = {
    .label = NULL,
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = TIGUI_SCREEN_HEIGHT / 2,
    .radius = 16,
    .dot_radius = 2,
    .num_dots = 8,
    .txt_fg_color = TIGUI_BLACK,
    .active_color = TIGUI_BLACK,
    .inactive_color = TIGUI_GRAY,
    .current_frame = 0,
    .tick = 0,
    .speed = 625,
    .update = true
};
tigui_alert_t scanning_screen_usage_alert = {
    .title =  "Welcome!",
    .body = "Plug in an Apple device in WTF, DFU, PWNDFU, or Recovery mode to get started!\n\nHow to Navigate with Keypad:\n[D-Pad] = Move selection cursor (red box)\n\n[clear] = Quit/Back\n\n[del] = Backspace (for input fields)\n\n[enter] = Press/Tap/Go into",
    .label_padding = 2,
    .fail_button_label = NULL,
    .pass_button_label = "Ok",
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = TIGUI_SCREEN_HEIGHT / 2,
    .w = 200,
    .h = 160,
    .button_h = 20,
    .bg_color = TIGUI_GRAY,
    .info_txt_fg_color = TIGUI_BLACK,
    .fail_button_txt_fg_color = TIGUI_BLACK,
    .pass_button_txt_fg_color = TIGUI_BLACK,
    .bdr_color = TIGUI_BLACK,
    .selected_bdr_color = TIGUI_RED,
    .is_showing = true,
    .pass_cb = NULL,
    .fail_cb = NULL,
    .selection_i = 0,
    .update = true
};

/* Nodes */
tigui_canvas_node_t scanning_screen_text_node = {
    .element = &scanning_screen_text,
    .type = TIGUI_ELEMENT_TEXT,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};
tigui_canvas_node_t scanning_screen_spinner_node = {
    .element = &scanning_screen_spinner,
    .type = TIGUI_ELEMENT_SPINNER,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};
tigui_canvas_node_t scanning_screen_alert_node = {
    .element = &scanning_screen_usage_alert,
    .type = TIGUI_ELEMENT_ALERT,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};

tigui_canvas_node_t* scanning_screen_nodeset[] = {
    &scanning_screen_text_node,
    &scanning_screen_spinner_node,
    &scanning_screen_alert_node
};

/* Canvas */
tigui_canvas_t scanning_screen_canvas = {
    .bg_color = TIGUI_WHITE,
    .is_running = false,
    .nodeset = scanning_screen_nodeset,
    .num_nodes = sizeof(scanning_screen_nodeset) / sizeof(scanning_screen_nodeset[0]),
    .user_data = NULL, // SET INSIDE CALLPOINT
    .on_draw_complete = scanning_screen_device_poller,
    .exit_key = sk_Clear,
    .enter_key = sk_Enter,
    .delete_key = sk_Del,
    .up_key = sk_Up,
    .down_key = sk_Down,
    .left_key = sk_Left,
    .right_key = sk_Right
};

/* Callpoint */
tigui_error_t scanning_screen(irecovery_client_t client) {
    if (client == NULL) return TIGUI_E_BAD_PARAM;

    scanning_screen_canvas.is_running = true;
    scanning_screen_canvas.user_data  = client;

    return tigui_orchestrate_canvas(&scanning_screen_canvas);
}

/* Actions/Callbacks (Implementation) */
bool scanning_screen_device_poller(void* user_data) {
    irecovery_client_t client = user_data;

    if (irecovery_poll_for_device(client) == IRECOVERY_E_SUCCESS) { // success on this function = device was found and finalized.
        scanning_screen_canvas.is_running = false;
    }

    return false; // Don't redraw anything
}