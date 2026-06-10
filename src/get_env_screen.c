#include <string.h>
#include <stdlib.h>
#include "tigui.h"
#include "irecovery.h"

char* get_env_screen_default_text_box_value = "Environment variable value will show up here!";
char get_env_screen_var_name_buf[32] = tigui_element_init();
char* get_env_screen_value = NULL; // heap

/* Actions/Callbacks (Forward Declaration) */
bool get_env_screen_check_connection(void* user_data);
bool get_env_screen_reset_value_text_box(void* user_data);
bool get_env_screen_fetch_variable(void* user_data);

/* Elements */
tigui_text_t get_env_screen_usage_text = {
    .text = "Type the variable's name into the input field below then hit the \"fetch\" button.\n\nTo switch to different typing modes, hit [2nd].\nTo go back, hit [clear].",
    .reference_point = TIGUI_TEXT_LEFT_POINT,
    .x = 2,
    .y = 2,
    .x_clip = TIGUI_SCREEN_WIDTH,
    .y_clip = TIGUI_SCREEN_HEIGHT,
    .color = TIGUI_BLACK,
    .wrap = true,
    .update = true
};
tigui_input_field_t get_env_screen_input_field = {
    .label = "Name",
    .placeholder = "auto-boot",
    .label_pos = TIGUI_LABEL_POS_LEFT,
    .label_padding = 2,
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = 96,
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
    .input_buf = get_env_screen_var_name_buf,
    .input_buf_size = sizeof(get_env_screen_var_name_buf),
    .input_buf_offset = 0,
    .draw_cursor = false,
    .update = true
};
tigui_button_t get_env_screen_fetch_button = {
    .label = "Fetch",
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = 122,
    .w = 80,
    .h = 20,
    .bg_color = TIGUI_GRAY,
    .txt_fg_color = TIGUI_BLACK,
    .bdr_color = TIGUI_BLACK,
    .selected_bdr_color = TIGUI_RED,
    .selected = false,
    .update = true
};
tigui_text_box_t get_env_screen_value_text_box = {
    .text = NULL, // SET IN CALLPOINT
    .padding = 2,
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = 186,
    .w = TIGUI_SCREEN_WIDTH - 4,
    .h = 102,
    .bg_color = TIGUI_GRAY,
    .txt_fg_color = TIGUI_BLACK,
    .bdr_color = TIGUI_BLACK,
    .selected_bdr_color = TIGUI_RED,
    .selected = false,
    .wrap = true,
    .update = true
};
tigui_alert_t get_env_screen_failed_alert = {
    .title = "Failed",
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
tigui_canvas_node_t get_env_screen_usage_text_node;
tigui_canvas_node_t get_env_screen_input_field_node;
tigui_canvas_node_t get_env_screen_fetch_button_node;
tigui_canvas_node_t get_env_screen_value_text_box_node;
tigui_canvas_node_t get_env_screen_alert_node;

tigui_canvas_node_t get_env_screen_usage_text_node = {
    .element = &get_env_screen_usage_text,
    .type = TIGUI_ELEMENT_TEXT,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};
tigui_canvas_node_t get_env_screen_input_field_node = {
    .element = &get_env_screen_input_field,
    .type = TIGUI_ELEMENT_INPUT_FIELD,
    .up = NULL,
    .down = &get_env_screen_fetch_button_node,
    .left = NULL,
    .right = NULL,
    .selectable = true,
    .on_pressed = get_env_screen_reset_value_text_box
};
tigui_canvas_node_t get_env_screen_fetch_button_node = {
    .element = &get_env_screen_fetch_button,
    .type = TIGUI_ELEMENT_BUTTON,
    .up = &get_env_screen_input_field_node,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = true,
    .on_pressed = get_env_screen_fetch_variable
};
tigui_canvas_node_t get_env_screen_value_text_box_node = {
    .element = &get_env_screen_value_text_box,
    .type = TIGUI_ELEMENT_TEXT_BOX,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};
tigui_canvas_node_t get_env_screen_failed_alert_node = {
    .element = &get_env_screen_failed_alert,
    .type = TIGUI_ELEMENT_ALERT,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};

tigui_canvas_node_t* get_env_screen_nodeset[] = {
    &get_env_screen_usage_text_node,
    &get_env_screen_input_field_node,
    &get_env_screen_fetch_button_node,
    &get_env_screen_value_text_box_node,
    &get_env_screen_failed_alert_node
};

/* Canvas */
tigui_canvas_t get_env_screen_canvas = {
    .bg_color = TIGUI_WHITE,
    .is_running = true,
    .nodeset = get_env_screen_nodeset,
    .num_nodes = sizeof(get_env_screen_nodeset) / sizeof(get_env_screen_nodeset[0]),
    .user_data = NULL, // SET IN CALLPOINT
    .on_draw_complete = get_env_screen_check_connection,
    .exit_key = sk_Clear,
    .enter_key = sk_Enter,
    .delete_key = sk_Del,
    .up_key = sk_Up,
    .down_key = sk_Down,
    .left_key = sk_Left,
    .right_key = sk_Right
};

/* Callpoint */
tigui_error_t get_env_screen(irecovery_client_t client) {
    if (client == NULL) return TIGUI_E_BAD_PARAM;

    get_env_screen_canvas.is_running = true;
    get_env_screen_canvas.user_data  = client;
    get_env_screen_input_field.current_descriptor_i = 0;
    memset(get_env_screen_var_name_buf, 0, sizeof(get_env_screen_var_name_buf));
    get_env_screen_input_field.input_buf_offset = 0;
    get_env_screen_input_field.draw_cursor = false;
    get_env_screen_value_text_box.text = get_env_screen_default_text_box_value;
    if (get_env_screen_value != NULL) {
        free(get_env_screen_value);
        get_env_screen_value = NULL;
    }
    get_env_screen_input_field.selected = true;
    get_env_screen_fetch_button.selected = false;
    get_env_screen_failed_alert.is_showing = false;

    tigui_error_t err = tigui_orchestrate_canvas(&get_env_screen_canvas);
    if (get_env_screen_value != NULL) {
        free(get_env_screen_value);
        get_env_screen_value = NULL;
    }
    return err;
}

/* Actions/Callbacks (Implimentation) */
bool get_env_screen_check_connection(void* user_data) {
    irecovery_client_t client = user_data;

    if (irecovery_client_is_usable(client, true) == false) {
        get_env_screen_canvas.is_running = false;
        if (get_env_screen_value != NULL) {
            free(get_env_screen_value);
            get_env_screen_value = NULL;
        }
    }

    return false; // Don't redraw anything.
}

bool get_env_screen_reset_value_text_box(void* user_data) {
    (void)user_data;

    if (get_env_screen_value != NULL) {
        free(get_env_screen_value);
        get_env_screen_value = NULL;
    }

    if (get_env_screen_value_text_box.text != get_env_screen_default_text_box_value) {
        get_env_screen_value_text_box.text = get_env_screen_default_text_box_value;
        get_env_screen_value_text_box.update = true;
    }

    return false; // Don't redraw EVERYTHING
}

bool get_env_screen_fetch_variable(void* user_data) {
    irecovery_client_t client = user_data;

    if (get_env_screen_value != NULL) {
        free(get_env_screen_value);
        get_env_screen_value = NULL;
    }

    irecovery_error_t err = irecovery_getenv(client, get_env_screen_var_name_buf, &get_env_screen_value);
    if (err != IRECOVERY_E_SUCCESS) {
        // title body isshowing callback
        get_env_screen_failed_alert.body = (char*)irecovery_strerror(err);
        get_env_screen_failed_alert.is_showing = true;
        return false;
    }

    get_env_screen_value_text_box.text   = get_env_screen_value;
    get_env_screen_value_text_box.update = true;

    return false; // Don't redraw EVERYTHING
}