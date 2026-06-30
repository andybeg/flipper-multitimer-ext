#include "multitimer.h"
#include <multitimerext_icons.h>

void multitimer_show_splash_popup(MultiTimerApp* app, PopupCallback callback, uint32_t timeout_ms) {
    if(!app || !app->welcome_popup) return;

    popup_set_icon(app->welcome_popup, 36, 0, &I_dolphin_welcome_56x56);
    popup_set_header(app->welcome_popup, NULL, 0, 0, AlignLeft, AlignTop);
    popup_set_text(app->welcome_popup, "MultiTimer", 64, 58, AlignCenter, AlignTop);
    popup_set_callback(app->welcome_popup, callback);
    popup_set_timeout(app->welcome_popup, timeout_ms);
    popup_enable_timeout(app->welcome_popup);

    if(app->view_dispatcher) {
        view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewWelcomePopup);
    }

    multitimer_request_epaper_splash(app);
}

void submenu_rebuild(MultiTimerApp* app) {
    if(!app || !app->submenu) return;

    submenu_reset(app->submenu);
    submenu_add_item(app->submenu, "Clock", MultiTimerMenuClock, submenu_callback, app);

    for(int i = 0; i < MAX_TIMERS; i++) {
        SavedTimerData* timer = &app->timer_storage.saved_timers[i];
        if(timer->used) {
            submenu_add_item(
                app->submenu,
                timer->name,
                MultiTimerMenuSavedTimerBase + i,
                submenu_callback,
                app);
        }
    }

    submenu_add_item(app->submenu, "Add Timer", MultiTimerMenuAddTimer, submenu_callback, app);

    char active_timers_label[32];
    int active_count = count_active_timers(app);
    if(active_count > 0) {
        snprintf(active_timers_label, sizeof(active_timers_label), "Active Timers (%d)", active_count);
    } else {
        snprintf(active_timers_label, sizeof(active_timers_label), "Active Timers");
    }
    submenu_add_item(
        app->submenu, active_timers_label, MultiTimerMenuActiveTimers, submenu_callback, app);
    submenu_add_item(app->submenu, "Settings", MultiTimerMenuSettings, submenu_callback, app);
}

// Welcome popup callback
void welcome_popup_callback(void* context) {
    MultiTimerApp* app = context;
    if(!app || !app->view_dispatcher) return;
    view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewSubmenu);
    if(app->show_on_eink && app->timer_storage.count > 0) {
        multitimer_epaper_refresh_now(app);
    }
}

void goodbye_popup_callback(void* context) {
    MultiTimerApp* app = context;
    if(!app || !app->view_dispatcher) return;
    view_dispatcher_stop(app->view_dispatcher);
}

// Submenu callbacks
void submenu_callback(void* context, uint32_t index) {
    MultiTimerApp* app = context;
    if(!app) return;

    if(index == MultiTimerMenuClock) {
        app->clock_screen_active = true;
        multitimer_epaper_show_clock(app);
        if(app->view_dispatcher) {
            view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewClock);
        }
    } else if(index >= MultiTimerMenuSavedTimerBase &&
       index < MultiTimerMenuSavedTimerBase + MAX_TIMERS) {
        int saved_index = index - MultiTimerMenuSavedTimerBase;
        SavedTimerData* saved_timer = &app->timer_storage.saved_timers[saved_index];
        if(saved_timer->used) {
            app->selected_saved_timer_index = saved_index;
            start_timer(app, saved_timer->duration_seconds, saved_timer->name);
        }
    } else if(index == MultiTimerMenuAddTimer) {
        app->setup_hours = 0;
        app->setup_minutes = 0;
        app->setup_seconds = 0;
        app->setup_selection = 0;
        if(app->view_dispatcher) {
            app->setup_screen_active = true;
            view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewTimerSetup);
            timer_setup_refresh(app);
        }
    } else if(index == MultiTimerMenuActiveTimers) {
        timer_list_prepare_selection(app, -1);
        if(app->view_dispatcher) {
            view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewTimerList);
        }
    } else if(index == MultiTimerMenuSettings) {
        settings_refresh(app);
        if(app->view_dispatcher) {
            view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewSettings);
        }
    }
}
