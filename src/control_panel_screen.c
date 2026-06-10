#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include "tigui.h"
#include "irecovery.h"
#include "canvi.h"

/* Actions/Callbacks (Forward Declarations) */
bool control_panel_screen_check_device_connection(void* user_data);
bool control_panel_screen_reset_device(void* user_data);
bool control_panel_screen_send_var(void* user_data);
bool control_panel_screen_send_command(void* user_data);
bool control_panel_screen_reboot_device(void* user_data);
bool control_panel_screen_reboot_to_normal_mode(void* user_data);
bool control_panel_screen_get_env(void* user_data);
bool control_panel_screen_set_env(void* user_data);
bool control_panel_screen_save_env(void* user_data);
void control_panel_screen_cleanup_alert_cb(void* user_data);
void control_panel_screen_reboot_alert_cb(void* user_data);
void control_panel_screen_reboot_to_normal_mode_alert_cb(void* user_data);

/* Elements */
tigui_text_box_t control_panel_screen_info_text_box = {
    .text = NULL, // SET INSIDE HELPERS, MALLOC'D
    .padding = 2,
    .x = 115,
    .y = TIGUI_SCREEN_HEIGHT / 2,
    .w = 230,
    .h = TIGUI_SCREEN_HEIGHT,
    .bg_color = TIGUI_WHITE,
    .txt_fg_color = TIGUI_BLACK,
    .bdr_color = TIGUI_BLACK,
    .selected_bdr_color = TIGUI_RED,
    .selected = false,
    .wrap = true,
    .update = true
};
tigui_rectangle_t control_panel_screen_button_box = {
    .x = 275,
    .y = TIGUI_SCREEN_HEIGHT / 2,
    .w = 90,
    .h = TIGUI_SCREEN_HEIGHT,
    .filled_color = TIGUI_WHITE,
    .bdr_color = TIGUI_BLACK,
    .selected_bdr_color = TIGUI_RED,
    .selected = false,
    .filled = false,
    .update = true
};
tigui_alert_t control_panel_screen_alert = {
    .title = NULL, // SET BY CALLER
    .body = NULL, // SET BY CALLER
    .label_padding = 2,
    .fail_button_label = NULL, // SET BY CALLER
    .pass_button_label = NULL, // SET BY CALLER
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
    .pass_cb = NULL, // SET BY CALLER
    .fail_cb = NULL, // SET BY CALLER
    .selection_i = 0, // SET BY CALLER
    .update = true
};

/* Helpers */
typedef struct {
    char* label;
    tigui_interactivity_cb_t on_pressed;
    bool available_in_recovery;
    bool available_in_dfu;
    bool available_in_wtf;
} control_panel_screen_irecovery_feature_t;

static const control_panel_screen_irecovery_feature_t control_panel_features[] = {
    {"Reset",    control_panel_screen_reset_device,             true, true,  true},
    {"Send var", control_panel_screen_send_var,                 true, true,  true},
    {"Command",  control_panel_screen_send_command,             true, false, false},
    {"Reboot",   control_panel_screen_reboot_device,            true, false, false},
    {"Normal",   control_panel_screen_reboot_to_normal_mode,    true, false, false},
    {"Get ENV",  control_panel_screen_get_env,                  true, false, false},
    {"Set ENV",  control_panel_screen_set_env,                  true, false, false},
    {"Save ENV", control_panel_screen_save_env,                 true, false, false},
};
#define CONTROL_PANEL_NUM_FEATURES sizeof(control_panel_features) / sizeof(control_panel_features[0])

/* Nodes */
#define CONTROL_PANEL_SCREEN_NUM_STATIC_NODES 3
tigui_canvas_node_t control_panel_screen_info_text_box_node = {
    .element = &control_panel_screen_info_text_box,
    .type = TIGUI_ELEMENT_TEXT_BOX,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};
tigui_canvas_node_t control_panel_screen_button_box_node = {
    .element = &control_panel_screen_button_box,
    .type = TIGUI_ELEMENT_RECTANGLE,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};
tigui_canvas_node_t control_panel_screen_alert_node = {
    .element = &control_panel_screen_alert,
    .type = TIGUI_ELEMENT_ALERT,
    .up = NULL,
    .down = NULL,
    .left = NULL,
    .right = NULL,
    .selectable = false,
    .on_pressed = NULL
};

int control_panel_screen_nodeset_dynamic_start_offset = CONTROL_PANEL_SCREEN_NUM_STATIC_NODES;
tigui_canvas_node_t* control_panel_screen_nodeset[CONTROL_PANEL_SCREEN_NUM_STATIC_NODES + CONTROL_PANEL_NUM_FEATURES] = {
    &control_panel_screen_info_text_box_node,
    &control_panel_screen_button_box_node,
    &control_panel_screen_alert_node
};

/* Canvas */
tigui_canvas_t control_panel_screen_canvas = {
    .bg_color = TIGUI_WHITE,
    .is_running = false,
    .nodeset = control_panel_screen_nodeset,
    .num_nodes = 0, // SET INSIDE HELPERS
    .user_data = NULL, // SET INSIDE CALLPOINT
    .on_draw_complete = control_panel_screen_check_device_connection,
    .exit_key = sk_Clear,
    .enter_key = sk_Enter,
    .delete_key = sk_Del,
    .up_key = sk_Up,
    .down_key = sk_Down,
    .left_key = sk_Left,
    .right_key = sk_Right
};

/* Helpers */
bool control_panel_feature_is_available(const control_panel_screen_irecovery_feature_t* feature, int mode) {
    if (mode == IRECOVERY_K_RECOVERY_MODE_1 ||
        mode == IRECOVERY_K_RECOVERY_MODE_2 ||
        mode == IRECOVERY_K_RECOVERY_MODE_3 ||
        mode == IRECOVERY_K_RECOVERY_MODE_4) {
        return feature->available_in_recovery;
    } else if (mode == IRECOVERY_K_DFU_MODE || mode == IRECOVERY_K_PWNDFU_MODE) {
        return feature->available_in_dfu;
    } else if (mode == IRECOVERY_K_WTF_MODE) {
        return feature->available_in_wtf;
    } else {
        return false;
    }
}

void control_panel_screen_free_dynamic_nodeset_members_and_reset_canvas_count(void) {
    size_t start = CONTROL_PANEL_SCREEN_NUM_STATIC_NODES;
    size_t end = CONTROL_PANEL_SCREEN_NUM_STATIC_NODES + CONTROL_PANEL_NUM_FEATURES;

    for (size_t i = start; i < end; i++) {
        if (control_panel_screen_nodeset[i] != NULL) {

            if (control_panel_screen_nodeset[i]->element != NULL) free(control_panel_screen_nodeset[i]->element);

            free(control_panel_screen_nodeset[i]);
            control_panel_screen_nodeset[i] = NULL;
        }
    }

    control_panel_screen_canvas.num_nodes = CONTROL_PANEL_SCREEN_NUM_STATIC_NODES;
}

void control_panel_screen_reset(bool is_running, void* user_data) {
    control_panel_screen_canvas.is_running = is_running;
    control_panel_screen_free_dynamic_nodeset_members_and_reset_canvas_count();
    control_panel_screen_canvas.user_data = user_data;

    if (control_panel_screen_info_text_box.text != NULL) {
        free(control_panel_screen_info_text_box.text);
        control_panel_screen_info_text_box.text = NULL;
    }

    control_panel_screen_alert.title = NULL;
    control_panel_screen_alert.body = NULL;
    control_panel_screen_alert.fail_button_label = NULL;
    control_panel_screen_alert.pass_button_label = NULL;
    control_panel_screen_alert.pass_cb = NULL;
    control_panel_screen_alert.fail_cb = NULL;
    control_panel_screen_alert.is_showing = false;
    control_panel_screen_alert.selection_i = 0;
}

tigui_error_t control_panel_screen_sb_appendf(char** buf, size_t* len, size_t* cap, const char* fmt, ...) {
    va_list args;

    va_start(args, fmt);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (needed < 0) return TIGUI_E_THIRDPARTY_ERROR;

    size_t required = *len + (size_t)needed + 1;

    if (required > *cap) {
        size_t new_cap = (*cap == 0) ? 256 : *cap;
        while (new_cap < required) new_cap *= 2;

        char* new_buf = realloc(*buf, new_cap);
        if (!new_buf) return TIGUI_E_THIRDPARTY_ERROR;

        *buf = new_buf;
        *cap = new_cap;
    }

    va_start(args, fmt);
    vsnprintf(*buf + *len, *cap - *len, fmt, args);
    va_end(args);

    *len += (size_t)needed;

    return TIGUI_E_SUCCESS;
}

tigui_error_t control_panel_screen_concatenate_device_info_string(irecovery_client_t client) {
    if (client == NULL) return TIGUI_E_BAD_PARAM;

    tigui_error_t err;
    const char* err_str = "Error: Unknown error.";
    char* out = NULL;
    size_t len = 0, cap = 0;

    const struct irecovery_device_info* devinfo = irecovery_get_device_info(client);
    if (devinfo == NULL) {
        err = TIGUI_E_THIRDPARTY_ERROR;
        err_str = "Error: Failed to get advanced device info.";
        goto fail;
    }

    #define APPEND(fmt, ...) \
        do { \
            err = control_panel_screen_sb_appendf(&out, &len, &cap, fmt, ##__VA_ARGS__); \
            if (err != TIGUI_E_SUCCESS) { \
                err_str = "Error: Failed to append text."; \
                goto fail; \
            } \
        } while (0)

    /* Basic fields */
    APPEND("CPID: 0x%04x\n", devinfo->cpid);
    APPEND("CPRV: 0x%02x\n", devinfo->cprv);
    APPEND("BDID: 0x%02x\n", devinfo->bdid);
    APPEND("ECID: 0x%016" PRIx64 "\n", devinfo->ecid);
    APPEND("CPFM: 0x%02x\n", devinfo->cpfm);
    APPEND("SCEP: 0x%02x\n", devinfo->scep);
    APPEND("IBFL: 0x%02x\n", devinfo->ibfl);

    /* Strings */
    APPEND("SRTG: %s\n", devinfo->srtg ? devinfo->srtg : "N/A");
    APPEND("SRNM: %s\n", devinfo->srnm ? devinfo->srnm : "N/A");
    APPEND("IMEI: %s\n", devinfo->imei ? devinfo->imei : "N/A");

    /* NONCE */
    APPEND("NONC: ");
    if (devinfo->ap_nonce) {
        for (unsigned int i = 0; i < devinfo->ap_nonce_size; i++) {
            APPEND("%02x", devinfo->ap_nonce[i]);
        }
    } else {
        APPEND("N/A");
    }
    APPEND("\n");

    /* SEP nonce */
    APPEND("SNON: ");
    if (devinfo->sep_nonce) {
        for (unsigned int i = 0; i < devinfo->sep_nonce_size; i++) {
            APPEND("%02x", devinfo->sep_nonce[i]);
        }
    } else {
        APPEND("N/A");
    }
    APPEND("\n");

    /* PWND extraction (same logic as iRecovery) */
    if (devinfo->serial_string) {
        char* p = strstr(devinfo->serial_string, "PWND:[");
        if (p) {
            p += 6;
            char* pend = strchr(p, ']');
            if (pend) {
                APPEND("PWND: %.*s\n", (int)(pend - p), p);
            }
        }
    }

    int mode;
    irecovery_error_t irecv_err = irecovery_get_mode(client, &mode); if (irecv_err != IRECOVERY_E_SUCCESS) {
        err_str = "Error: Failed to get device mode.";
        goto fail;
    }
    APPEND("MODE: %s\n", irecovery_mode_to_str(mode));

    irecovery_device_t device = NULL;
    irecovery_devices_get_device_by_client(client, &device);
    if (device != NULL) {
        APPEND("PRODUCT: %s\n", device->product_type);
        APPEND("MODEL: %s\n", device->hardware_model);
        APPEND("NAME: %s\n", device->display_name);
    }

    out[len] = '\0';
    control_panel_screen_info_text_box.text = out;

    return TIGUI_E_SUCCESS;

fail:
    free(out);
    control_panel_screen_info_text_box.text = strdup(err_str);
    control_panel_screen_info_text_box.update = true;
    return err;
}

tigui_error_t control_panel_screen_build_button_linkage(irecovery_client_t client) {
    if (client == NULL) return TIGUI_E_BAD_PARAM;

    control_panel_screen_free_dynamic_nodeset_members_and_reset_canvas_count();

    int mode;
    irecovery_error_t err = irecovery_get_mode(client, &mode);
    if (err != IRECOVERY_E_SUCCESS) return TIGUI_E_THIRDPARTY_ERROR;

    size_t dyn_index = 0;
    uint16_t button_y = 16;
    tigui_canvas_node_t* prev = NULL;
    for (size_t i = 0; i < CONTROL_PANEL_NUM_FEATURES; i++) {
        const control_panel_screen_irecovery_feature_t* feature = &control_panel_features[i];

        if (control_panel_feature_is_available(feature, mode) == false) continue;

        /* Allocate button */
        tigui_button_t* btn = malloc(sizeof(tigui_button_t));
        if (btn == NULL) {
            control_panel_screen_free_dynamic_nodeset_members_and_reset_canvas_count();
            return TIGUI_E_THIRDPARTY_ERROR;
        }

        btn->label              = feature->label;
        btn->x                  = 275;
        btn->y                  = button_y;
        btn->w                  = 80;
        btn->h                  = 20;
        btn->bg_color           = TIGUI_GRAY;
        btn->txt_fg_color       = TIGUI_BLACK;
        btn->bdr_color          = TIGUI_BLACK;
        btn->selected_bdr_color = TIGUI_RED;
        btn->selected           = (dyn_index == 0);
        btn->update             = true;

        /* Allocate node */
        tigui_canvas_node_t* node = malloc(sizeof(tigui_canvas_node_t));
        if (node == NULL) {
            control_panel_screen_free_dynamic_nodeset_members_and_reset_canvas_count();
            return TIGUI_E_THIRDPARTY_ERROR;
        }

        node->element    = btn;
        node->type       = TIGUI_ELEMENT_BUTTON;
        node->up         = prev;
        node->down       = NULL;
        node->left       = NULL;
        node->right      = NULL;
        node->selectable = true;
        node->on_pressed = feature->on_pressed;

        /* Link previous node's down pointer to current */
        if (prev != NULL) {
            prev->down = node;
        }

        /* Insert into nodeset */
        control_panel_screen_nodeset[CONTROL_PANEL_SCREEN_NUM_STATIC_NODES + dyn_index] = node;

        /* Advance state */
        prev = node;
        button_y += 26;
        dyn_index++;
    }

    control_panel_screen_canvas.num_nodes = CONTROL_PANEL_SCREEN_NUM_STATIC_NODES + dyn_index;

    return TIGUI_E_SUCCESS;
}

/* Callpoint */
tigui_error_t control_panel_screen(irecovery_client_t client) {
    if (client == NULL) return TIGUI_E_BAD_PARAM;

    control_panel_screen_reset(true, client);

    tigui_error_t err = control_panel_screen_build_button_linkage(client); if (err != TIGUI_E_SUCCESS) goto exit;
    err = control_panel_screen_concatenate_device_info_string(client); if (err != TIGUI_E_SUCCESS) goto exit;
    
    err = tigui_orchestrate_canvas(&control_panel_screen_canvas);
    
    goto exit;

exit:
    control_panel_screen_reset(false, NULL);
    return err;
}

bool control_panel_screen_check_device_connection(void* user_data) {
    irecovery_client_t client = user_data;

    if (irecovery_client_is_usable(client, true) == false) {
        control_panel_screen_alert.title = "Device Disconnected";
        control_panel_screen_alert.body = "Connection to the device was lost.\n\nWill now return to the scanning screen.";
        control_panel_screen_alert.fail_button_label = NULL;
        control_panel_screen_alert.pass_button_label = "Ok";
        control_panel_screen_alert.pass_cb = control_panel_screen_cleanup_alert_cb;
        control_panel_screen_alert.fail_cb = NULL;
        control_panel_screen_alert.is_showing = true;
        control_panel_screen_alert.selection_i = 0;
    }

    return false; // Doesn't matter, canvas always gets redrawn on alert callbacks.
}

/* Actions/Callbacks (Implimentation) */
bool control_panel_screen_reset_device(void* user_data) {
    irecovery_client_t client = user_data;

    irecovery_error_t err = irecovery_reset(client);
    if (err != IRECOVERY_E_SUCCESS) {
        control_panel_screen_alert.title = "Failed";
        control_panel_screen_alert.body = (char*)irecovery_strerror(err);
        control_panel_screen_alert.pass_button_label = "Return to scanning screen.";
        control_panel_screen_alert.pass_cb = control_panel_screen_cleanup_alert_cb;
    } else {
        control_panel_screen_alert.title = "Success";
        control_panel_screen_alert.body = "USB device reset successfully.\n\nYou will need to reconnect your device before doing any other actions.";
        control_panel_screen_alert.pass_button_label = "Ok";
        control_panel_screen_alert.pass_cb = control_panel_screen_cleanup_alert_cb;
    }

    control_panel_screen_alert.fail_button_label = NULL;
    control_panel_screen_alert.fail_cb = NULL;
    control_panel_screen_alert.is_showing = true;
    control_panel_screen_alert.selection_i = 0;

    return false; // Doesn't matter, canvas always gets redrawn on alert callbacks.
}

bool control_panel_screen_send_var(void* user_data) {
    irecovery_client_t client = user_data;

    tigui_error_t err = send_var_screen(client);
    if (err != TIGUI_E_SUCCESS && err != TIGUI_E_USER_EXIT) {
        control_panel_screen_reset(false, NULL);
        return false;
    }

    return true; // Redraw everything since we just ran an entirely new screen.
}

bool control_panel_screen_send_command(void* user_data) {
    irecovery_client_t client = user_data;

    tigui_error_t err = send_command_screen(client);
    if (err != TIGUI_E_SUCCESS && err != TIGUI_E_USER_EXIT) {
        control_panel_screen_reset(false, NULL);
        return false;
    }

    return true; // Redraw everything since we just ran an entirely new screen.
}

bool control_panel_screen_reboot_device(void* user_data) {
    (void)user_data;

    control_panel_screen_alert.title = "Are You Sure?";
    control_panel_screen_alert.body = "Rebooting will close the current connection and return to the scanning screen.";
    control_panel_screen_alert.fail_button_label = "Cancel";
    control_panel_screen_alert.pass_button_label = "Reboot";
    control_panel_screen_alert.pass_cb = control_panel_screen_reboot_alert_cb;
    control_panel_screen_alert.fail_cb = NULL;
    control_panel_screen_alert.is_showing = true;
    control_panel_screen_alert.selection_i = 0;

    return false; // Doesn't matter, canvas always gets redrawn on alert callbacks.
}

bool control_panel_screen_reboot_to_normal_mode(void* user_data) {
    (void)user_data;

    control_panel_screen_alert.title = "Are You Sure?";
    control_panel_screen_alert.body = "Rebooting to normal will kick your device into its normal OS and close the current connection, returning to the scanning screen.";
    control_panel_screen_alert.fail_button_label = "Cancel";
    control_panel_screen_alert.pass_button_label = "Reboot";
    control_panel_screen_alert.pass_cb = control_panel_screen_reboot_to_normal_mode_alert_cb;
    control_panel_screen_alert.fail_cb = NULL;
    control_panel_screen_alert.is_showing = true;
    control_panel_screen_alert.selection_i = 0;

    return false; // Doesn't matter, canvas always gets redrawn on alert callbacks.
}

bool control_panel_screen_get_env(void* user_data) {
    irecovery_client_t client = user_data;

    tigui_error_t err = get_env_screen(client);
    if (err != TIGUI_E_SUCCESS && err != TIGUI_E_USER_EXIT) {
        control_panel_screen_reset(false, NULL);
        return false;
    }

    return true; // Redraw everything since we just ran an entirely new screen.
}
bool control_panel_screen_set_env(void* user_data) {
    irecovery_client_t client = user_data;

    tigui_error_t err = set_env_screen(client);
    if (err != TIGUI_E_SUCCESS && err != TIGUI_E_USER_EXIT) {
        control_panel_screen_reset(false, NULL);
        return false;
    }

    return true; // Redraw everything since we just ran an entirely new screen.
}

bool control_panel_screen_save_env(void* user_data) {
    irecovery_client_t client = user_data;

    irecovery_error_t err = irecovery_saveenv(client);
    if (err == IRECOVERY_E_SUCCESS) {
        control_panel_screen_alert.title = "Success";
        control_panel_screen_alert.body = "On-device environment variables were saved.";
        control_panel_screen_alert.pass_button_label = "Ok";
        control_panel_screen_alert.pass_cb = NULL;
    } else {
        control_panel_screen_alert.title = "Failed";
        control_panel_screen_alert.body = (char*)irecovery_strerror(err);
        control_panel_screen_alert.pass_button_label = "Return to scanning screen.";
        control_panel_screen_alert.pass_cb = control_panel_screen_cleanup_alert_cb;
    }

    control_panel_screen_alert.fail_button_label = NULL;
    control_panel_screen_alert.fail_cb = NULL;
    control_panel_screen_alert.is_showing = true;
    control_panel_screen_alert.selection_i = 0;

    return false; // Doesn't matter, canvas always gets redrawn on alert callbacks.
}

void control_panel_screen_cleanup_alert_cb(void* user_data) {
    (void)user_data;
    control_panel_screen_reset(false, NULL);
}

void control_panel_screen_reboot_alert_cb(void* user_data) {
    irecovery_client_t client = user_data;
    irecovery_reboot(client);
    control_panel_screen_reset(false, NULL);
}

void control_panel_screen_reboot_to_normal_mode_alert_cb(void* user_data) {
    irecovery_client_t client = user_data;

    if (irecovery_setenv(client, "auto-boot", "true") != IRECOVERY_E_SUCCESS) {
        control_panel_screen_reset(false, NULL);
        return;
    }

    if (irecovery_saveenv(client) != IRECOVERY_E_SUCCESS) {
        control_panel_screen_reset(false, NULL);
        return;
    }

    if (irecovery_reboot(client) != IRECOVERY_E_SUCCESS) control_panel_screen_reset(false, NULL);
}