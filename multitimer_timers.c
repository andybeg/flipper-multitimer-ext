#include "multitimer.h"

uint32_t get_remaining_seconds(TimerData* timer) {
    if(!timer || !timer->active || timer->state != TimerStateRunning) {
        return timer ? timer->duration_seconds : 0;
    }

    uint32_t current_timestamp = furi_hal_rtc_get_timestamp();
    uint32_t elapsed = current_timestamp - timer->start_timestamp;

    if(elapsed >= timer->duration_seconds) {
        return 0;
    }

    return timer->duration_seconds - elapsed;
}

void check_expired_timers(MultiTimerApp* app) {
    if(!app) return;

    bool has_expired = false;

    for(int i = 0; i < MAX_TIMERS; i++) {
        TimerData* timer = &app->timer_storage.timers[i];
        if(timer->active && timer->state == TimerStateRunning) {
            if(get_remaining_seconds(timer) == 0) {
                timer->state = TimerStateFinished;
                has_expired = true;
                FURI_LOG_I(TAG, "Timer '%s' expired", timer->name);
            }
        }
    }

    if(has_expired && app->notifications) {
        notification_message(app->notifications, &sequence_audiovisual_alert);
        app->alarm_until_timestamp =
            furi_hal_rtc_get_timestamp() + app->alarm_duration_seconds;
        app->last_alarm_timestamp = furi_hal_rtc_get_timestamp();
        save_timer_data(app);
    }
}

void multitimer_update_alarm(MultiTimerApp* app) {
    if(!app || !app->notifications || app->alarm_until_timestamp == 0) return;

    uint32_t now = furi_hal_rtc_get_timestamp();
    if(now > app->alarm_until_timestamp) {
        notification_message(app->notifications, &sequence_reset_sound);
        notification_message(app->notifications, &sequence_reset_vibro);
        app->alarm_until_timestamp = 0;
        app->last_alarm_timestamp = 0;
        return;
    }

    if(now - app->last_alarm_timestamp >= 1) {
        notification_message(app->notifications, &sequence_audiovisual_alert);
        app->last_alarm_timestamp = now;
    }
}

int find_free_timer_slot(MultiTimerApp* app) {
    if(!app) return -1;

    for(int i = 0; i < MAX_TIMERS; i++) {
        if(!app->timer_storage.timers[i].active) {
            return i;
        }
    }
    return -1;
}

int find_free_saved_timer_slot(MultiTimerApp* app) {
    if(!app) return -1;

    for(int i = 0; i < MAX_TIMERS; i++) {
        if(!app->timer_storage.saved_timers[i].used) {
            return i;
        }
    }
    return -1;
}

int count_active_timers(MultiTimerApp* app) {
    if(!app) return 0;
    return (int)app->timer_storage.count;
}

int get_active_timer_slot_by_row(MultiTimerApp* app, int row) {
    if(!app || row < 0) return -1;

    int current_row = 0;
    for(int i = 0; i < MAX_TIMERS; i++) {
        if(app->timer_storage.timers[i].active) {
            if(current_row == row) {
                return i;
            }
            current_row++;
        }
    }
    return -1;
}

int get_active_timer_row_for_slot(MultiTimerApp* app, int slot) {
    if(!app || slot < 0 || slot >= MAX_TIMERS || !app->timer_storage.timers[slot].active) {
        return -1;
    }

    int row = 0;
    for(int i = 0; i < slot; i++) {
        if(app->timer_storage.timers[i].active) {
            row++;
        }
    }
    return row;
}

void multitimer_clear_active_timers(MultiTimerApp* app) {
    if(!app) return;

    for(int i = 0; i < MAX_TIMERS; i++) {
        app->timer_storage.timers[i].active = false;
    }
    app->timer_storage.count = 0;
    app->selected_timer_index = -1;
    app->timer_list_selected_index = -1;
    app->alarm_until_timestamp = 0;
    app->last_alarm_timestamp = 0;
}

void deactivate_timer(MultiTimerApp* app, int slot) {
    if(!app || slot < 0 || slot >= MAX_TIMERS) return;

    TimerData* timer = &app->timer_storage.timers[slot];
    if(!timer->active) return;

    timer->active = false;
    app->timer_storage.count--;
    if(app->selected_timer_index == slot) {
        app->selected_timer_index = -1;
    }
    if(app->timer_list_selected_index == slot) {
        app->timer_list_selected_index = -1;
    }
    save_timer_data(app);
    submenu_rebuild(app);

    if(app->show_on_eink) {
        if(app->timer_storage.count > 0) {
            multitimer_epaper_update(app, true);
        } else {
            multitimer_epaper_show_splash(app);
        }
    }
}

void navigate_after_timer_removed(MultiTimerApp* app) {
    if(!app || !app->view_dispatcher) return;

    if(app->timer_storage.count > 0) {
        timer_list_prepare_selection(app, -1);
        view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewTimerList);
    } else {
        view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewSubmenu);
    }
}

void timer_list_prepare_selection(MultiTimerApp* app, int prefer_slot) {
    if(!app) return;

    app->timer_list_scroll = 0;
    app->timer_list_selected_index = -1;

    if(prefer_slot >= 0 && prefer_slot < MAX_TIMERS &&
       app->timer_storage.timers[prefer_slot].active) {
        app->timer_list_selected_index = prefer_slot;
    } else {
        for(int i = 0; i < MAX_TIMERS; i++) {
            if(app->timer_storage.timers[i].active) {
                app->timer_list_selected_index = i;
                break;
            }
        }
    }

    if(app->timer_list_selected_index >= 0) {
        int row = get_active_timer_row_for_slot(app, app->timer_list_selected_index);
        if(row > 3) {
            app->timer_list_scroll = row - 3;
        }
    }
}

bool multitimer_has_finished_timers(MultiTimerApp* app) {
    if(!app) return false;

    for(int i = 0; i < MAX_TIMERS; i++) {
        TimerData* timer = &app->timer_storage.timers[i];
        if(timer->active && timer->state == TimerStateFinished) {
            return true;
        }
    }
    return false;
}

int multitimer_count_finished_timers(MultiTimerApp* app) {
    if(!app) return 0;

    int count = 0;
    for(int i = 0; i < MAX_TIMERS; i++) {
        TimerData* timer = &app->timer_storage.timers[i];
        if(timer->active && timer->state == TimerStateFinished) {
            count++;
        }
    }
    return count;
}

void finished_popup_callback(void* context) {
    MultiTimerApp* app = context;
    if(!app || !app->view_dispatcher) return;

    timer_list_prepare_selection(app, -1);
    view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewTimerList);
}

bool start_timer(MultiTimerApp* app, uint32_t duration_seconds, const char* name) {
    int slot = find_free_timer_slot(app);
    if(slot < 0) return false;

    TimerData* timer = &app->timer_storage.timers[slot];
    timer->active = true;
    timer->start_timestamp = furi_hal_rtc_get_timestamp();
    timer->duration_seconds = duration_seconds;
    timer->state = TimerStateRunning;
    snprintf(timer->name, sizeof(timer->name), "%s", name);

    app->timer_storage.count++;
    app->selected_timer_index = slot;
    save_timer_data(app);
    submenu_rebuild(app);
    multitimer_request_epaper_refresh(app);

    if(app->view_dispatcher) {
        view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewTimerRunning);
    }
    return true;
}

int save_new_timer_preset(MultiTimerApp* app, uint32_t duration_seconds) {
    int slot = find_free_saved_timer_slot(app);
    if(slot < 0) return -1;

    SavedTimerData* saved_timer = &app->timer_storage.saved_timers[slot];
    saved_timer->used = true;
    saved_timer->duration_seconds = duration_seconds;

    char duration_str[32];
    format_time(duration_seconds, duration_str, sizeof(duration_str));
    snprintf(saved_timer->name, sizeof(saved_timer->name), "%s", duration_str);

    save_timer_data(app);
    submenu_rebuild(app);
    return slot;
}
