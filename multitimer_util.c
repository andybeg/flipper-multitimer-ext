#include "multitimer.h"

void format_time(uint32_t seconds, char* buffer, size_t buffer_size) {
    if(!buffer || buffer_size == 0) return;

    uint32_t hours = seconds / 3600;
    uint32_t minutes = (seconds % 3600) / 60;
    uint32_t secs = seconds % 60;
    snprintf(buffer, buffer_size, "%02lu:%02lu:%02lu", hours, minutes, secs);
}

MultiTimerApp* multitimer_view_get_app(void* model) {
    if(!model) return NULL;
    return *(MultiTimerApp**)model;
}

void timer_setup_refresh(MultiTimerApp* app) {
    if(!app || !app->timer_setup_view) return;
    with_view_model(app->timer_setup_view, MultiTimerApp** model, { UNUSED(model); }, true);
}

static void multitimer_set_view_rotation(View* view, bool rotate) {
    if(!view) return;
    view_set_orientation(
        view, rotate ? ViewOrientationHorizontalFlip : ViewOrientationHorizontal);
}

void multitimer_apply_screen_rotation(MultiTimerApp* app) {
    if(!app) return;

    if(app->submenu) {
        multitimer_set_view_rotation(submenu_get_view(app->submenu), app->rotate_screen);
    }
    multitimer_set_view_rotation(app->clock_view, app->rotate_screen);
    multitimer_set_view_rotation(app->timer_setup_view, app->rotate_screen);
    multitimer_set_view_rotation(app->timer_running_view, app->rotate_screen);
    multitimer_set_view_rotation(app->timer_list_view, app->rotate_screen);
    multitimer_set_view_rotation(app->timer_delete_view, app->rotate_screen);
    if(app->settings_list) {
        multitimer_set_view_rotation(
            variable_item_list_get_view(app->settings_list), app->rotate_screen);
    }
    if(app->eink_settings_list) {
        multitimer_set_view_rotation(
            variable_item_list_get_view(app->eink_settings_list), app->rotate_screen);
    }
    if(app->welcome_popup) {
        multitimer_set_view_rotation(popup_get_view(app->welcome_popup), app->rotate_screen);
    }

    if(app->view_dispatcher) {
        view_dispatcher_send_to_front(app->view_dispatcher);
    }
}
