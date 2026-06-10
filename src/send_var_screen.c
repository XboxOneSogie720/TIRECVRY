#include <fileioc.h>
#include <stdlib.h>
#include <string.h>
#include "tigui.h"
#include "irecovery.h"
#include "canvi.h"

tigui_list_item_t* send_var_screen_items = NULL; // heap
char msg_buf[64] = tigui_element_init();

/* Actions/Callbacks (Forward Declaration) */
bool send_var_screen_check_connection(void* user_data);
void send_var_screen_confirmation(char* text, void* user_data);
void send_var_screen_send_var_alert_cb(void* user_data);

/* Elements */
tigui_text_t send_var_screen_usage_text = {
    .text = "Enter list selection by hitting Enter, scroll to find the appvar you want to send, then hit enter again!\n\nHit clear to back out.",
    .reference_point = TIGUI_TEXT_LEFT_POINT,
    .x = 2,
    .y = 2,
    .x_clip = TIGUI_SCREEN_WIDTH,
    .y_clip = TIGUI_SCREEN_HEIGHT,
    .color = TIGUI_BLACK,
    .wrap = true,
    .update = true
};
tigui_list_t send_var_screen_appvar_list = {
    .label = "AppVars",
    .label_pos = TIGUI_LABEL_POS_TOP,
    .item_txt_reference_point = TIGUI_TEXT_LEFT_POINT,
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = (TIGUI_SCREEN_HEIGHT / 2) + 10,
    .w = TIGUI_SCREEN_WIDTH / 2,
    .h = TIGUI_SCREEN_HEIGHT / 2,
    .item_h = 20,
    .bg_color = TIGUI_GRAY,
    .bdr_color = TIGUI_BLACK,
    .selected_bdr_color = TIGUI_RED,
    .selected = true,
    .items = NULL, // SET INSIDE HELPER
    .num_items = 0, // SET INSIDE HELPER
    .scroll_offset_i = 0,
    .selected_i = 0,
    .is_browsing = false,
    .update = true
};
tigui_text_t send_var_screen_send_more_after_this_description = {
    .text = "If enabled, the device won't detect a new appvar being added to the send queue.",
    .reference_point = TIGUI_TEXT_LEFT_POINT,
    .x = 2,
    .y = (TIGUI_SCREEN_HEIGHT / 2) + 10 + 62,
    .x_clip = TIGUI_SCREEN_WIDTH,
    .y_clip = TIGUI_SCREEN_HEIGHT,
    .color = TIGUI_BLACK,
    .wrap = true,
    .update = true
};
tigui_toggle_t send_var_screen_send_more_after_this_toggle = {
    .label = "Data piece",
    .right_label_padding = 2,
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = (TIGUI_SCREEN_HEIGHT / 2) + 10 + 95,
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
tigui_alert_t send_var_screen_alert = {
    .title = "Are You Sure?",
    .body = NULL, // SET INSIDE CALLBACK
    .label_padding = 2,
    .fail_button_label = "Cancel",
    .pass_button_label = "Yes",
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
    .is_showing = false, //  SET INSIDE CALLBACK
    .pass_cb = send_var_screen_send_var_alert_cb,
    .fail_cb = NULL,
    .selection_i = 0,
    .update = true
};

/* Nodes */
tigui_canvas_node_t send_var_screen_usage_text_node;
tigui_canvas_node_t send_var_screen_appvar_list_node;
tigui_canvas_node_t send_var_screen_send_more_after_this_description_node;
tigui_canvas_node_t send_var_screen_send_more_after_this_toggle_node;
tigui_canvas_node_t send_var_screen_alert_node;

tigui_canvas_node_t send_var_screen_usage_text_node = {
    .element = &send_var_screen_usage_text,
    .type = TIGUI_ELEMENT_TEXT,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};
tigui_canvas_node_t send_var_screen_appvar_list_node = {
    .element = &send_var_screen_appvar_list,
    .type = TIGUI_ELEMENT_LIST,
    .up = NULL,
    .down = &send_var_screen_send_more_after_this_toggle_node,
    .left = NULL,
    .right = NULL,
    .selectable = true,
    .on_pressed = NULL
};
tigui_canvas_node_t send_var_screen_send_more_after_this_description_node = {
    .element = &send_var_screen_send_more_after_this_description,
    .type = TIGUI_ELEMENT_TEXT,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};
tigui_canvas_node_t send_var_screen_send_more_after_this_toggle_node = {
    .element = &send_var_screen_send_more_after_this_toggle,
    .type = TIGUI_ELEMENT_TOGGLE,
    .up = &send_var_screen_appvar_list_node,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = true,
    .on_pressed = NULL
};
tigui_canvas_node_t send_var_screen_alert_node = {
    .element = &send_var_screen_alert,
    .type = TIGUI_ELEMENT_ALERT,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};

tigui_canvas_node_t* nodeset[] = {
    &send_var_screen_usage_text_node,
    &send_var_screen_appvar_list_node,
    &send_var_screen_send_more_after_this_description_node,
    &send_var_screen_send_more_after_this_toggle_node,
    &send_var_screen_alert_node
};

/* Cavnas */
tigui_canvas_t send_var_screen_canvas = {
    .bg_color = TIGUI_WHITE,
    .is_running = true,
    .nodeset = nodeset,
    .num_nodes = sizeof(nodeset) / sizeof(nodeset[0]),
    .user_data = NULL, // SET INSIDE CALLPOINT
    .on_draw_complete = send_var_screen_check_connection,
    .exit_key = sk_Clear,
    .enter_key = sk_Enter,
    .up_key = sk_Up,
    .down_key = sk_Down,
    .left_key = sk_Left,
    .right_key = sk_Right
};

/* Helpers */
static size_t send_var_screen_get_num_appvars(void) {
    char* var_name;
    void* vat_ptr = NULL;
    size_t count = 0;
    while ((var_name = ti_Detect(&vat_ptr, NULL)) != NULL) count++;

    return count;
}

static void send_var_screen_recursive_items_free(size_t num_items) {
    if (send_var_screen_items == NULL) return;

    for (size_t i = 0; i < num_items; i++) {
        if (send_var_screen_items[i].text != NULL) {
            free(send_var_screen_items[i].text);
        }
    }

    free(send_var_screen_items);
    send_var_screen_items = NULL;
}

static tigui_error_t send_var_screen_populate_allocd_list(size_t num_appvars) {
    if (send_var_screen_items == NULL) return TIGUI_E_THIRDPARTY_ERROR;

    char* var_name;
    void* vat_ptr = NULL;
    for (size_t i = 0; i < num_appvars; i++) {
        if ((var_name = ti_Detect(&vat_ptr, NULL)) == NULL) break;

        send_var_screen_items[i].text = strdup(var_name);
        if (send_var_screen_items[i].text == NULL) {
            send_var_screen_recursive_items_free(num_appvars);
            return TIGUI_E_THIRDPARTY_ERROR;
        }

        send_var_screen_items[i].pressed_cb = send_var_screen_confirmation;
    }

    return TIGUI_E_SUCCESS;
}

static tigui_error_t send_var_screen_build_list_items(void) {
    size_t num_appvars = send_var_screen_get_num_appvars();
    send_var_screen_items = malloc(sizeof(tigui_list_item_t) * num_appvars);
    if (send_var_screen_items == NULL) return TIGUI_E_THIRDPARTY_ERROR;

    tigui_error_t err = send_var_screen_populate_allocd_list(num_appvars);
    if (err != TIGUI_E_SUCCESS) {
        send_var_screen_recursive_items_free(num_appvars);
        return err;
    }

    send_var_screen_appvar_list.items = send_var_screen_items;
    send_var_screen_appvar_list.num_items = num_appvars;

    return TIGUI_E_SUCCESS;    
}

static void send_var_screen_reset(void) {
    send_var_screen_canvas.is_running = true;
    send_var_screen_appvar_list.num_items = 0;
    send_var_screen_appvar_list.scroll_offset_i = 0;
    send_var_screen_appvar_list.selected_i = 0;
    send_var_screen_appvar_list.is_browsing = false;
    send_var_screen_send_more_after_this_toggle.on = false;
}

/* Callpoint */
tigui_error_t send_var_screen(irecovery_client_t client) {
    send_var_screen_canvas.user_data = client;

    send_var_screen_reset();

    tigui_error_t err = send_var_screen_build_list_items(); if (err != TIGUI_E_SUCCESS) return err;

    err = tigui_orchestrate_canvas(&send_var_screen_canvas);
    send_var_screen_recursive_items_free(send_var_screen_appvar_list.num_items);

    return err;
}

/* Actions/Callbacks (Implimentation) */
bool send_var_screen_check_connection(void* user_data) {
    irecovery_client_t client = user_data;

    if (irecovery_client_is_usable(client, true) == false) send_var_screen_canvas.is_running = false;

    return false; // Don't redraw anything.
}

void send_var_screen_confirmation(char* text, void* user_data) {
    (void)user_data;

    memset(msg_buf, 0, sizeof(msg_buf));
    snprintf(msg_buf, sizeof(msg_buf), "You are about to send data from %s to your device.", text);
    
    send_var_screen_alert.body = msg_buf;
    send_var_screen_alert.is_showing = true;
}

void send_var_screen_send_var_alert_cb(void* user_data) {
    irecovery_client_t client = user_data;
    
    if (progress_bar_screen(&send_var_screen_canvas, client, send_var_screen_items[send_var_screen_appvar_list.selected_i].text, send_var_screen_send_more_after_this_toggle.on) != TIGUI_E_SUCCESS) {
        send_var_screen_canvas.is_running = false;
    }

    send_var_screen_alert.selection_i = 0;
}