#include "multitimer.h"
#include "epaper42_bw.h"

EpaperTimerState multitimer_epaper_state(TimerState state) {
    switch(state) {
    case TimerStateRunning:
        return EpaperTimerStateRunning;
    case TimerStatePaused:
        return EpaperTimerStatePaused;
    case TimerStateFinished:
        return EpaperTimerStateFinished;
    case TimerStateSetup:
    default:
        return EpaperTimerStateStopped;
    }
}

void multitimer_epaper_reinit(MultiTimerApp* app) {
    if(!app) return;

    multitimer_epaper_normalize_model(app);
    if(app->epaper_ready) {
        epaper42_bw_deinit();
        app->epaper_ready = false;
    }
    if(app->show_on_eink) {
        app->epaper_ready = epaper42_bw_init();
        if(!app->epaper_ready) {
            FURI_LOG_E(TAG, "e-paper init failed");
        }
    }
}

void multitimer_epaper_refresh_now(MultiTimerApp* app) {
    if(!app || !app->show_on_eink) return;

    multitimer_epaper_reinit(app);
    if(app->clock_screen_active) {
        multitimer_epaper_show_clock(app);
    } else if(app->setup_screen_active) {
        multitimer_epaper_show_setup(app);
    } else {
        multitimer_epaper_update(app, true);
    }
}

void multitimer_request_epaper_refresh(MultiTimerApp* app) {
    if(!app || !app->view_dispatcher || !app->show_on_eink) return;
    view_dispatcher_send_custom_event(
        app->view_dispatcher, MultiTimerCustomEventEpaperRefresh);
}

void multitimer_epaper_show_splash(MultiTimerApp* app) {
    if(!app || !app->show_on_eink) return;

    multitimer_epaper_reinit(app);
    if(!app->epaper_ready) {
        app->epaper_ready = epaper42_bw_init();
        if(!app->epaper_ready) {
            FURI_LOG_E(TAG, "e-paper init failed");
            return;
        }
    }

    epaper42_bw_show_splash(app->epaper_invert_colors);
    app->last_epaper_update_timestamp = furi_hal_rtc_get_timestamp();
}

void multitimer_request_epaper_splash(MultiTimerApp* app) {
    if(!app || !app->view_dispatcher || !app->show_on_eink) return;
    view_dispatcher_send_custom_event(
        app->view_dispatcher, MultiTimerCustomEventEpaperSplash);
}

TimerData* multitimer_get_epaper_timer(MultiTimerApp* app) {
    if(!app) return NULL;

    if(app->selected_timer_index >= 0 && app->selected_timer_index < MAX_TIMERS) {
        TimerData* timer = &app->timer_storage.timers[app->selected_timer_index];
        if(timer->active) return timer;
    }

    for(int i = 0; i < MAX_TIMERS; i++) {
        TimerData* timer = &app->timer_storage.timers[i];
        if(timer->active) return timer;
    }

    return NULL;
}

void multitimer_epaper_show_stopped(MultiTimerApp* app, uint32_t duration_seconds) {
    if(!app) return;
    if(!app->show_on_eink) return;

    multitimer_epaper_normalize_model(app);
    if(!app->epaper_ready) {
        app->epaper_ready = epaper42_bw_init();
        if(!app->epaper_ready) {
            FURI_LOG_E(TAG, "e-paper init failed");
            return;
        }
    }
    Epaper42RefreshMode refresh_mode = app->epaper_partial_mode ? Epaper42RefreshModePartial :
                                                                  Epaper42RefreshModeFull;
    epaper42_bw_show_timer(
        duration_seconds,
        duration_seconds,
        Epaper42TimerStateStopped,
        refresh_mode,
        app->epaper_invert_colors);
    app->last_epaper_update_timestamp = furi_hal_rtc_get_timestamp();
}

uint32_t multitimer_setup_duration(const MultiTimerApp* app) {
    if(!app) return 0;
    return app->setup_hours * 3600 + app->setup_minutes * 60 + app->setup_seconds;
}

void multitimer_epaper_show_setup(MultiTimerApp* app) {
    if(!app) return;
    uint32_t duration = multitimer_setup_duration(app);
    if(duration == 0) return;
    multitimer_epaper_show_stopped(app, duration);
}

void multitimer_epaper_show_setup_throttled(MultiTimerApp* app) {
    if(!app || !app->show_on_eink) return;

    uint32_t duration = multitimer_setup_duration(app);
    if(duration == 0) return;

    uint32_t now = furi_hal_rtc_get_timestamp();
    if(now - app->last_epaper_update_timestamp < app->epaper_refresh_seconds) {
        return;
    }

    multitimer_epaper_show_setup(app);
}

void multitimer_epaper_show_clock(MultiTimerApp* app) {
    if(!app) return;
    if(!app->show_on_eink) return;

    DateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);
    uint32_t clock_seconds = datetime.hour * 3600U + datetime.minute * 60U + datetime.second;

    multitimer_epaper_normalize_model(app);
    if(!app->epaper_ready) {
        app->epaper_ready = epaper42_bw_init();
        if(!app->epaper_ready) {
            FURI_LOG_E(TAG, "e-paper init failed");
            return;
        }
    }

    epaper42_bw_show_timer(
        clock_seconds,
        0,
        Epaper42TimerStateStopped,
        Epaper42RefreshModeFull,
        app->epaper_invert_colors);
    app->last_epaper_update_timestamp = furi_hal_rtc_get_timestamp();
}

void multitimer_epaper_update(MultiTimerApp* app, bool force) {
    if(!app) return;
    if(!app->show_on_eink) return;

    TimerData* timer = multitimer_get_epaper_timer(app);
    if(!timer) {
        if(force) {
            multitimer_epaper_show_splash(app);
        }
        return;
    }

    uint32_t now = furi_hal_rtc_get_timestamp();
    uint32_t remaining = get_remaining_seconds(timer);
    bool timer_needs_immediate_update =
        timer->state == TimerStatePaused || timer->state == TimerStateFinished || remaining == 0;

    if(!force && !timer_needs_immediate_update &&
       now - app->last_epaper_update_timestamp < app->epaper_refresh_seconds) {
        return;
    }

    multitimer_epaper_normalize_model(app);
    if(!app->epaper_ready) {
        app->epaper_ready = epaper42_bw_init();
        if(!app->epaper_ready) {
            FURI_LOG_E(TAG, "e-paper init failed");
            return;
        }
    }

    epaper42_bw_show_timer(
        remaining,
        timer->duration_seconds,
        (Epaper42TimerState)multitimer_epaper_state(timer->state),
        force ? Epaper42RefreshModeFull :
                (app->epaper_partial_mode ? Epaper42RefreshModePartial : Epaper42RefreshModeFull),
        app->epaper_invert_colors);
    app->last_epaper_update_timestamp = now;
}
