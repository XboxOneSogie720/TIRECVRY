#include "tigui.h"
#include "irecovery.h"

/* Elements */
tigui_text_t progress_bar_screen_usage_text = {
    .text = "Do not unplug your device while USB transfers are occuring.\n\nIf you must cancel the transfer, hit [clear].",
    .reference_point = TIGUI_TEXT_LEFT_POINT,
    .x = 2,
    .y = 2,
    .x_clip = TIGUI_SCREEN_WIDTH,
    .y_clip = TIGUI_SCREEN_HEIGHT,
    .color = TIGUI_BLACK,
    .wrap = true,
    .update = true
};
tigui_progress_bar_t progress_bar_screen_progress_bar = {
    .label = "Sending...",
    .label_pos = TIGUI_LABEL_POS_TOP,
    .label_padding = 2,
    .x = TIGUI_SCREEN_WIDTH / 2,
    .y = TIGUI_SCREEN_HEIGHT / 2,
    .w = TIGUI_SCREEN_WIDTH / 2,
    .h = 20,
    .bg_color = TIGUI_GRAY,
    .progressed_color = TIGUI_GREEN,
    .txt_fg_color = TIGUI_BLACK,
    .bdr_color = TIGUI_BLACK,
    .selected_bdr_color = TIGUI_RED,
    .selected = false,
    .progress = 0.0f,
    .update = true
};
tigui_alert_t progress_bar_screen_alert = {
    .title = "Failed",
    .body = NULL, // SET INSIDE CALLPOINT
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
    .is_showing = true,
    .pass_cb = NULL,
    .fail_cb = NULL,
    .selection_i = 0,
    .update = true
};

/* Actions/Helpers (Forward Declaration) */
int progress_bar_screen_progress_cb(irecovery_client_t client, const irecovery_event_t* event);

// Since progress is driven by irecovery_send_x, we must draw ourselves.
// Luckily we're only drawing a couple of things.
tigui_error_t progress_bar_screen(tigui_canvas_t* canvas, irecovery_client_t client, const char* filename, bool more_after_this) {
    if (canvas == NULL || irecovery_client_is_usable(client, true) == false || filename == NULL) return TIGUI_E_BAD_PARAM;

    tigui_error_t err = tigui_bgcolor(TIGUI_WHITE); if (err != TIGUI_E_SUCCESS) return err;

    progress_bar_screen_progress_bar.progress = 0.0f;
    progress_bar_screen_progress_bar.update = true;
    progress_bar_screen_alert.update = true;

    err = tigui_draw_text(&progress_bar_screen_usage_text); if (err != TIGUI_E_SUCCESS) return err;
    err = tigui_draw_progress_bar(&progress_bar_screen_progress_bar); if (err != TIGUI_E_SUCCESS) return err;

    irecovery_error_t irecv_err = irecovery_event_subscribe(client, IRECOVERY_APPVAR_PROGRESS, progress_bar_screen_progress_cb); 
    if (irecv_err != IRECOVERY_E_SUCCESS) {
        progress_bar_screen_alert.body = (char*)irecovery_strerror(irecv_err);
        if (tigui_draw_alert(&progress_bar_screen_alert) == TIGUI_E_SUCCESS) wait_for_specific_keypress(canvas->enter_key);
        return TIGUI_E_THIRDPARTY_ERROR;
    }

    static bool previous_more_after_this = false;
    unsigned int base_options = previous_more_after_this ? IRECOVERY_SEND_OPT_DFU_CRC_RETAIN : IRECOVERY_SEND_OPT_NONE;
    previous_more_after_this = more_after_this;

    irecv_err = irecovery_send_appvar(filename, client, base_options, more_after_this);
    if (irecv_err != IRECOVERY_E_SUCCESS) {
        progress_bar_screen_alert.body = (char*)irecovery_strerror(irecv_err);
        if (tigui_draw_alert(&progress_bar_screen_alert) == TIGUI_E_SUCCESS) wait_for_specific_keypress(canvas->enter_key);
    }

    irecovery_event_unsubscribe(client, IRECOVERY_APPVAR_PROGRESS);
    
    if (irecv_err != IRECOVERY_E_SUCCESS) {
        return TIGUI_E_THIRDPARTY_ERROR;
    } else {
        return TIGUI_E_SUCCESS;
    }
}

/* Actions/Helpers (Implementation) */
int progress_bar_screen_progress_cb(irecovery_client_t client, const irecovery_event_t* event) {
    (void)client;

    progress_bar_screen_progress_bar.progress = (event->progress / 100.0);
    progress_bar_screen_progress_bar.update = true;
    if (tigui_draw_progress_bar(&progress_bar_screen_progress_bar) != TIGUI_E_SUCCESS || os_GetCSC() == sk_Clear) {
        return -1;
    } else {
        return 0;
    }
}