#define CGLM_DEFINE_PRINTS
#include "log.h"
#include "state.h"

int main() {
    game_init();

    float delta_time = 0.0f, last_frame = 0.0f;
    while (!game_should_stop()) {
        float current_frame = (float)glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;
        
        game_tick(delta_time);
    }

    game_deinit();
    return 0;
}
