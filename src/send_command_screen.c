#include <string.h>
#include "tigui.h"
#include "irecovery.h"

char cmd_buf[256] = tigui_element_init();

/* Actions/Callbacks (Forward Declaration) */
bool send_command_screen_check_connection(void* user_data);
bool send_command_screen_execute_command(void* user_data);

/* Elements */
tigui_text_t send_command_screen_usage_text = {
    .text = "Type your command into the input field below then send it!\n\nTo switch to different typing modes, hit [2nd].\nTo go back, hit [clear].",
    .reference_point = TIGUI_TEXT_LEFT_POINT,
    .x = 2,
    .y = 2,
    .x_clip = TIGUI_SCREEN_WIDTH,
    .y_clip = TIGUI_SCREEN_HEIGHT,
    .color = TIGUI_BLACK,
    .wrap = true,
    .update = true
};
tigui_input_field_t send_command_screen_input_field = {
    .label = "iDevice $ ",
    .placeholder = "reboot",
    .label_pos = TIGUI_LABEL_POS_LEFT,
    .label_padding = 2,
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = TIGUI_SCREEN_HEIGHT / 2,
    .w = TIGUI_SCREEN_WIDTH / 2,
    .h = 16,
    .bg_color = TIGUI_WHITE,
    .txt_fg_color = TIGUI_BLACK,
    .placeholder_txt_fg_color = TIGUI_GRAY,
    .type_descriptor_bg_color = TIGUI_BLACK,
    .type_descriptor_txt_fg_color = TIGUI_WHITE,
    .bdr_color = TIGUI_BLACK,
    .selected_bdr_color = TIGUI_RED,
    .selected = true,
    .descriptors = tigui_default_input_field_descriptors,
    .num_descriptors = tigui_num_default_input_field_descriptors,
    .current_descriptor_i = 0,
    .descriptor_switch_key = sk_2nd,
    .input_buf = cmd_buf,
    .input_buf_size = sizeof(cmd_buf),
    .input_buf_offset = 0,
    .draw_cursor = false,
    .update = true
};
tigui_button_t send_command_screen_execute_button = {
    .label = "Execute",
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = (TIGUI_SCREEN_HEIGHT / 2) + 26,
    .w = 80,
    .h = 20,
    .bg_color = TIGUI_GRAY,
    .txt_fg_color = TIGUI_BLACK,
    .bdr_color = TIGUI_BLACK,
    .selected_bdr_color = TIGUI_RED,
    .selected = false,
    .update = true
};
tigui_alert_t send_command_screen_alert = {
    .title = NULL, // SET BY CALLER
    .body = NULL, // SET BY CALLER
    .label_padding = 2,
    .fail_button_label = NULL,
    .pass_button_label = "Ok",
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = TIGUI_SCREEN_HEIGHT / 2,
    .w = 200,
    .h = 80,
    .button_h = 20,
    .bg_color = TIGUI_GRAY,
    .info_txt_fg_color = TIGUI_BLACK,
    .fail_button_txt_fg_color = TIGUI_BLACK,
    .pass_button_txt_fg_color = TIGUI_BLACK,
    .bdr_color = TIGUI_BLACK,
    .selected_bdr_color = TIGUI_RED,
    .is_showing = false,
    .pass_cb = NULL,
    .fail_cb = NULL,
    .selection_i = 0,
    .update = true
};

/* Nodes */
tigui_canvas_node_t send_command_screen_usage_text_node;
tigui_canvas_node_t send_command_screen_input_field_node;
tigui_canvas_node_t send_command_screen_execute_button_node;
tigui_canvas_node_t send_command_screen_alert_node;

tigui_canvas_node_t send_command_screen_usage_text_node = {
    .element = &send_command_screen_usage_text,
    .type = TIGUI_ELEMENT_TEXT,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};
tigui_canvas_node_t send_command_screen_input_field_node = {
    .element = &send_command_screen_input_field,
    .type = TIGUI_ELEMENT_INPUT_FIELD,
    .up = NULL,
    .down = &send_command_screen_execute_button_node,
    .left = NULL,
    .right = NULL,
    .selectable = true,
    .on_pressed = NULL
};
tigui_canvas_node_t send_command_screen_execute_button_node = {
    .element = &send_command_screen_execute_button,
    .type = TIGUI_ELEMENT_BUTTON,
    .up = &send_command_screen_input_field_node,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = true,
    .on_pressed = send_command_screen_execute_command
};
tigui_canvas_node_t send_command_screen_alert_node = {
    .element = &send_command_screen_alert,
    .type = TIGUI_ELEMENT_ALERT,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};

tigui_canvas_node_t* send_command_screen_nodeset[] = {
    &send_command_screen_usage_text_node,
    &send_command_screen_input_field_node,
    &send_command_screen_execute_button_node,
    &send_command_screen_alert_node
};

/* Canvas */
tigui_canvas_t send_command_screen_canvas = {
    .bg_color = TIGUI_WHITE,
    .is_running = true,
    .nodeset = send_command_screen_nodeset,
    .num_nodes = sizeof(send_command_screen_nodeset) / sizeof(send_command_screen_nodeset[0]),
    .user_data = NULL, // SET IN CALLPOINT
    .on_draw_complete = send_command_screen_check_connection,
    .exit_key = sk_Clear,
    .enter_key = sk_Enter,
    .delete_key = sk_Del,
    .up_key = sk_Up,
    .down_key = sk_Down,
    .left_key = sk_Left,
    .right_key = sk_Right
};

/* Callpoint */
tigui_error_t send_command_screen(irecovery_client_t client) {
    if (client == NULL) return TIGUI_E_BAD_PARAM;

    send_command_screen_canvas.is_running = true;
    send_command_screen_canvas.user_data  = client;
    send_command_screen_input_field.current_descriptor_i = 0;
    memset(cmd_buf, 0, sizeof(cmd_buf));
    send_command_screen_input_field.input_buf_offset = 0;
    send_command_screen_input_field.draw_cursor = false;
    send_command_screen_input_field.selected = true;
    send_command_screen_execute_button.selected = false;
    send_command_screen_alert.is_showing = false;

    return tigui_orchestrate_canvas(&send_command_screen_canvas);
}

/* Actions/Callbacks (Implimentation) */
bool send_command_screen_check_connection(void* user_data) {
    irecovery_client_t client = user_data;

    if (irecovery_client_is_usable(client, true) == false) send_command_screen_canvas.is_running = false;

    return false; // Don't redraw anything.
}

bool send_command_screen_execute_command(void* user_data) {
    irecovery_client_t client = user_data;

    irecovery_error_t err = irecovery_send_command(client, cmd_buf);
    if (err != IRECOVERY_E_SUCCESS) {
        send_command_screen_alert.title = "Failed";
        send_command_screen_alert.body = (char*)irecovery_strerror(err);
    } else {
        send_command_screen_alert.title = "Success";
        send_command_screen_alert.body = "The command was sent successfully.";
    }

    send_command_screen_alert.is_showing = true;

    return false; // Don't redraw EVERYTHING
}