#include "tigui/tigui.h"
#include "canvi.h"
#include "libirecovery/irecovery.h"

int main(void) {
    // Get irecovery ready
    irecovery_client_t client = NULL;
    irecovery_error_t irecv_err = irecovery_client_new(IRECOVERY_CLIENT_DEVICE_POLICY_ACCEPT_ONLY_WHEN_NO_CURRENT_CONNECTION, 0, NULL, &client);
    if (irecv_err != IRECOVERY_E_SUCCESS) return irecv_err;
    
    tigui_init();
    tigui_error_t err = TIGUI_E_SUCCESS;
    while (err == TIGUI_E_SUCCESS) {
        if (irecovery_client_is_usable(client, true) == false) {
            // Look for a client.
            err = scanning_screen(client);
        } else {
            // Let the user interact with the client.
            err = control_panel_screen(client);

            if (err == TIGUI_E_USER_EXIT) {
                // Prevents the app from quitting if the user exits a device's info panel.
                err = TIGUI_E_SUCCESS;
            }

            // Returning from the control panel signals that the user wants to stop
            // interacting with that device.
            // ...or something bad happened.
            irecovery_client_clear_device_zone(client);
        }
    }
    tigui_deinit();

    irecovery_client_free(&client);
    return err;
}