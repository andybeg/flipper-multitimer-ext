#include "multitimer.h"

void timer_delete_draw_callback(Canvas* canvas, void* model) {
    MultiTimerApp* app = multitimer_view_get_app(model);
    if(!app || !canvas) return;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 8, AlignCenter, AlignTop, "Delete timer?");

    canvas_set_font(canvas, FontSecondary);
    if(app->delete_saved_timer_index >= 0 && app->delete_saved_timer_index < MAX_TIMERS) {
        SavedTimerData* timer = &app->timer_storage.saved_timers[app->delete_saved_timer_index];
        if(timer->used) {
            canvas_draw_str_aligned(canvas, 64, 28, AlignCenter, AlignTop, timer->name);
        }
    }
    canvas_draw_str_aligned(canvas, 64, 52, AlignCenter, AlignTop, "OK: Delete  Back: Cancel");
}

bool timer_delete_input_callback(InputEvent* event, void* context) {
    MultiTimerApp* app = context;
    if(!app || !event) return false;

    if(event->type == InputTypePress) {
        switch(event->key) {
        case InputKeyOk:
            if(app->delete_saved_timer_index >= 0 && app->delete_saved_timer_index < MAX_TIMERS) {
                memset(
                    &app->timer_storage.saved_timers[app->delete_saved_timer_index],
                    0,
                    sizeof(SavedTimerData));
                app->selected_saved_timer_index = -1;
                app->delete_saved_timer_index = -1;
                save_timer_data(app);
                submenu_rebuild(app);
            }
            if(app->view_dispatcher) {
                view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewSubmenu);
            }
            return true;

        case InputKeyBack:
            app->delete_saved_timer_index = -1;
            if(app->view_dispatcher) {
                view_dispatcher_switch_to_view(app->view_dispatcher, MultiTimerViewTimerRunning);
            }
            return true;

        default:
            break;
        }
    }

    return false;
}
