#include "multitimer.h"
#include <multitimerext_icons.h>

void timer_running_draw_callback(Canvas* canvas, void* model) {
    MultiTimerApp* app = multitimer_view_get_app(model);
    if(!app || !canvas) return;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);

    if(app->selected_timer_index >= 0 && app->selected_timer_index < MAX_TIMERS) {
        TimerData* timer = &app->timer_storage.timers[app->selected_timer_index];

        if(timer->active) {
            canvas_draw_icon(canvas, 0, 3, &I_hourglass_10x10);
            canvas_draw_str_aligned(canvas, 64, 5, AlignCenter, AlignTop, timer->name);

            // Draw remaining time
            char time_str[32];
            uint32_t remaining = get_remaining_seconds(timer);
            format_time(remaining, time_str, sizeof(time_str));

            canvas_set_font(canvas, FontBigNumbers);
            canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignCenter, time_str);

            // Draw progress bar
            uint32_t elapsed = timer->duration_seconds - remaining;
            if(timer->duration_seconds > 0) {
                uint32_t progress_width =
                    (uint32_t)((float)elapsed / timer->duration_seconds * 110);
                canvas_draw_frame(canvas, 9, 45, 110, 8);
                if(progress_width > 0) {
                    canvas_draw_box(canvas, 10, 46, progress_width, 6);
                }
            }

            // Instructions
            canvas_set_font(canvas, FontSecondary);
            if(timer->state == TimerStateRunning) {
                canvas_draw_str_aligned(
                    canvas,
                    64,
                    58,
                    AlignCenter,
                    AlignTop,
                    "OK:Pause Back:Menu Left:Stop Right:List");
            } else if(timer->state == TimerStatePaused) {
                canvas_draw_str_aligned(
                    canvas,
                    64,
                    58,
                    AlignCenter,
                    AlignTop,
                    app->selected_saved_timer_index >= 0 ? "OK:Run Back:Menu Left:Stop Down:Del" :
                                                           "OK:Run Back:Menu Left:Stop");
            } else if(timer->state == TimerStateFinished) {
                canvas_set_font(canvas, FontPrimary);
                canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignTop, "FINISHED!");
                canvas_set_font(canvas, FontSecondary);
                canvas_draw_str_aligned(
                    canvas, 64, 58, AlignCenter, AlignTop, "OK: Dismiss");
            }
        }
    }
}

bool timer_running_input_callback(InputEvent* event, void* context) {
    MultiTimerApp* app = context;
    if(!app || !event) return false;

    if(event->type == InputTypePress) {
        if(app->selected_timer_index >= 0 && app->selected_timer_index < MAX_TIMERS) {
            TimerData* timer = &app->timer_storage.timers[app->selected_timer_index];

            if(timer->state == TimerStateFinished) {
                deactivate_timer(app, app->selected_timer_index);
                navigate_after_timer_removed(app);
                return true;
            }

            switch(event->key) {
            case InputKeyOk:
                if(timer->state == TimerStateRunning) {
                    timer->state = TimerStatePaused;
                    // Store the remaining time when paused
                    uint32_t remaining = get_remaining_seconds(timer);
                    timer->duration_seconds = remaining;
                    timer->start_timestamp = furi_hal_rtc_get_timestamp();
                } else if(timer->state == TimerStatePaused) {
                    timer->state = TimerStateRunning;
                    timer->start_timestamp = furi_hal_rtc_get_timestamp();
                }
                save_timer_data(app);
                multitimer_epaper_update(app, true);
                return true;

            case InputKeyBack:
                submenu_rebuild(app);
                if(app->view_dispatcher) {
                    view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewSubmenu);
                }
                return true;

            case InputKeyLeft:
                deactivate_timer(app, app->selected_timer_index);
                navigate_after_timer_removed(app);
                return true;

            case InputKeyRight:
                timer_list_prepare_selection(app, app->selected_timer_index);
                if(app->view_dispatcher) {
                    view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewTimerList);
                }
                return true;

            case InputKeyDown:
                if(timer->state == TimerStatePaused && app->selected_saved_timer_index >= 0 &&
                   app->selected_saved_timer_index < MAX_TIMERS &&
                   app->timer_storage.saved_timers[app->selected_saved_timer_index].used) {
                    app->delete_saved_timer_index = app->selected_saved_timer_index;
                    if(app->view_dispatcher) {
                        view_dispatcher_switch_to_view(
                            app->view_dispatcher, MultiTimerViewDeleteTimer);
                    }
                    return true;
                }
                break;

            default:
                break;
            }
        }
    }

    return false;
}
