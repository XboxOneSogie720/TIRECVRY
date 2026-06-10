#include <string.h>
#include "tigui.h"
#include "irecovery.h"

char set_env_screen_var_name_buf[32] = tigui_element_init();
char set_env_screen_var_value_buf[sizeof(set_env_screen_var_name_buf) * 4] = tigui_element_init();

/* Actions/Callbacks (Forward Declaration) */
bool set_env_screen_check_connection(void* user_data);
bool set_env_screen_apply_variable(void* user_data);
void set_env_screen_stop_canvas_alert_cb(void* user_data);

/* Elements */
tigui_text_t set_env_screen_usage_text = {
    .text = "Type the variable's name and value into their respective fields below then hit the \"apply\" button.\n\nTo switch to different typing modes, hit [2nd].\nTo go back, hit [clear].",
    .reference_point = TIGUI_TEXT_LEFT_POINT,
    .x = 2,
    .y = 2,
    .x_clip = TIGUI_SCREEN_WIDTH,
    .y_clip = TIGUI_SCREEN_HEIGHT,
    .color = TIGUI_BLACK,
    .wrap = true,
    .update = true
};
tigui_input_field_t set_env_screen_var_name_input_field = {
    .label = "Name",
    .placeholder = "auto-boot",
    .label_pos = TIGUI_LABEL_POS_LEFT,
    .label_padding = 2,
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = 92,
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
    .input_buf = set_env_screen_var_name_buf,
    .input_buf_size = sizeof(set_env_screen_var_name_buf),
    .input_buf_offset = 0,
    .draw_cursor = false,
    .update = true
};
tigui_input_field_t set_env_screen_var_value_input_field = {
    .label = "Value",
    .placeholder = "false",
    .label_pos = TIGUI_LABEL_POS_LEFT,
    .label_padding = 2,
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = 118,
    .w = TIGUI_SCREEN_WIDTH / 2,
    .h = 16,
    .bg_color = TIGUI_WHITE,
    .txt_fg_color = TIGUI_BLACK,
    .placeholder_txt_fg_color = TIGUI_GRAY,
    .type_descriptor_bg_color = TIGUI_BLACK,
    .type_descriptor_txt_fg_color = TIGUI_WHITE,
    .bdr_color = TIGUI_BLACK,
    .selected_bdr_color = TIGUI_RED,
    .selected = false,
    .descriptors = tigui_default_input_field_descriptors,
    .num_descriptors = tigui_num_default_input_field_descriptors,
    .current_descriptor_i = 0,
    .descriptor_switch_key = sk_2nd,
    .input_buf = set_env_screen_var_value_buf,
    .input_buf_size = sizeof(set_env_screen_var_value_buf),
    .input_buf_offset = 0,
    .draw_cursor = false,
    .update = true
};
tigui_toggle_t set_env_screen_save_env_toggle = {
    .label = "Save ENV",
    .right_label_padding = 2,
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = 144,
    .off_color = TIGUI_GRAY,
    .on_color = TIGUI_GREEN,
    .knob_color = TIGUI_WHITE,
    .txt_fg_color = TIGUI_BLACK,
    .bdr_color = TIGUI_BLACK,
    .selected_bdr_color = TIGUI_RED,
    .selected = false,
    .on = false,
    .update = true
};
tigui_button_t set_env_screen_apply_button = {
    .label = "Apply",
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = 176,
    .w = 80,
    .h = 20,
    .bg_color = TIGUI_GRAY,
    .txt_fg_color = TIGUI_BLACK,
    .bdr_color = TIGUI_BLACK,
    .selected_bdr_color = TIGUI_RED,
    .selected = false,
    .update = true
};
tigui_alert_t set_env_screen_alert = {
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
    .is_showing = false, // SET BY CALLER
    .pass_cb = NULL,
    .fail_cb = NULL,
    .selection_i = 0,
    .update = true
};

/* Nodes */
tigui_canvas_node_t set_env_screen_usage_text_node;
tigui_canvas_node_t set_env_screen_var_name_input_field_node;
tigui_canvas_node_t set_env_screen_var_value_input_field_node;
tigui_canvas_node_t set_env_screen_save_env_toggle_node;
tigui_canvas_node_t set_env_screen_apply_button_node;
tigui_canvas_node_t set_env_screen_alert_node;

tigui_canvas_node_t set_env_screen_usage_text_node = {
    .element = &set_env_screen_usage_text,
    .type = TIGUI_ELEMENT_TEXT,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};
tigui_canvas_node_t set_env_screen_var_name_input_field_node = {
    .element = &set_env_screen_var_name_input_field,
    .type = TIGUI_ELEMENT_INPUT_FIELD,
    .up = NULL,
    .down = &set_env_screen_var_value_input_field_node,
    .left = NULL,
    .right = NULL,
    .selectable = true,
    .on_pressed = NULL
};
tigui_canvas_node_t set_env_screen_var_value_input_field_node = {
    .element = &set_env_screen_var_value_input_field,
    .type = TIGUI_ELEMENT_INPUT_FIELD,
    .up = &set_env_screen_var_name_input_field_node,
    .down = &set_env_screen_save_env_toggle_node,
    .left = NULL,
    .right = NULL,
    .selectable = true,
    .on_pressed = NULL
};
tigui_canvas_node_t set_env_screen_save_env_toggle_node = {
    .element = &set_env_screen_save_env_toggle,
    .type = TIGUI_ELEMENT_TOGGLE,
    .up = &set_env_screen_var_value_input_field_node,
    .down = &set_env_screen_apply_button_node,
    .left = NULL,
    .right = NULL,
    .selectable = true,
    .on_pressed = NULL
};
tigui_canvas_node_t set_env_screen_apply_button_node = {
    .element = &set_env_screen_apply_button,
    .type = TIGUI_ELEMENT_BUTTON,
    .up = &set_env_screen_save_env_toggle_node,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = true,
    .on_pressed = set_env_screen_apply_variable
};
tigui_canvas_node_t set_env_screen_alert_node = {
    .element = &set_env_screen_alert,
    .type = TIGUI_ELEMENT_ALERT,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};

tigui_canvas_node_t* set_env_screen_nodeset[] = {
    &set_env_screen_usage_text_node,
    &set_env_screen_var_name_input_field_node,
    &set_env_screen_var_value_input_field_node,
    &set_env_screen_save_env_toggle_node,
    &set_env_screen_apply_button_node,
    &set_env_screen_alert_node
};

/* Canvas */
tigui_canvas_t set_env_screen_canvas = {
    .bg_color = TIGUI_WHITE,
    .is_running = true,
    .nodeset = set_env_screen_nodeset,
    .num_nodes = sizeof(set_env_screen_nodeset) / sizeof(set_env_screen_nodeset[0]),
    .user_data = NULL, // SET IN CALLPOINT
    .on_draw_complete = set_env_screen_check_connection,
    .exit_key = sk_Clear,
    .enter_key = sk_Enter,
    .delete_key = sk_Del,
    .up_key = sk_Up,
    .down_key = sk_Down,
    .left_key = sk_Left,
    .right_key = sk_Right
};

/* Callpoint */
tigui_error_t set_env_screen(irecovery_client_t client) {
    if (client == NULL) return TIGUI_E_BAD_PARAM;

    set_env_screen_canvas.is_running = true;
    set_env_screen_canvas.user_data  = client;
    memset(set_env_screen_var_name_buf, 0, sizeof(set_env_screen_var_name_buf));
    set_env_screen_var_name_input_field.input_buf_offset = 0;
    set_env_screen_var_name_input_field.draw_cursor = false;
    memset(set_env_screen_var_value_buf, 0, sizeof(set_env_screen_var_value_buf));
    set_env_screen_var_value_input_field.input_buf_offset = 0;
    set_env_screen_var_value_input_field.draw_cursor = false;
    set_env_screen_var_name_input_field.selected = true;
    set_env_screen_var_value_input_field.selected = false;
    set_env_screen_save_env_toggle.on = false;
    set_env_screen_apply_button.selected = false;
    set_env_screen_alert.is_showing = false;

    return tigui_orchestrate_canvas(&set_env_screen_canvas);
}

/* Actions/Callbacks (Implimentation) */
bool set_env_screen_check_connection(void* user_data) {
    irecovery_client_t client = user_data;

    if (irecovery_client_is_usable(client, true) == false) set_env_screen_canvas.is_running = false;

    return false; // Don't redraw anything.
}

bool set_env_screen_apply_variable(void* user_data) {
    irecovery_client_t client = user_data;

    set_env_screen_alert.is_showing = true;

    irecovery_error_t err = irecovery_setenv(client, set_env_screen_var_name_buf, set_env_screen_var_value_buf);
    if (err != IRECOVERY_E_SUCCESS && err != IRECOVERY_E_USB_UPLOAD_FAILED) { // for some reason USB_UPLOAD_FAILED gets returned even though the command runs just fine (tested [iPhone SE 1st Gen] [iPhone 3G] [iPhone 4 (CDMA)] [iPhone XS]).
        set_env_screen_alert.title = "Error Setting ENV";
        set_env_screen_alert.body = (char*)irecovery_strerror(err);
        return false;
    }

    if (set_env_screen_save_env_toggle.on == true) {
        err = irecovery_saveenv(client);
        if (err != IRECOVERY_E_SUCCESS && err != IRECOVERY_E_USB_UPLOAD_FAILED) {
            set_env_screen_alert.title = "Error Saving ENV";
            set_env_screen_alert.body = (char*)irecovery_strerror(err);
            return false;
        }
    }

    set_env_screen_alert.title = "Success";
    set_env_screen_alert.body = "The changes have been applied successfully.";

    return false; // Don't redraw EVERYTHING
}