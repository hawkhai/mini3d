#include "mini3d.h"
#include "window.h"

Device device;

#define DEVICE_WIDTH    800
#define DEVICE_HEIGHT   600

int main(void)
{
    TCHAR *title = _T("Mini3d (software render tutorial) - ")
        _T("Left/Right: rotation, Up/Down: forward/backward, Space: switch state");

    Window window;
	if (window.screen_init(DEVICE_WIDTH, DEVICE_HEIGHT, title))
		return -1;

    device.device_init(DEVICE_WIDTH, DEVICE_HEIGHT, window.getScreenFrameBuffer());
    device.camera_at_zero(3, 0, 0);

    device.init_texture();
	device.render_state = RENDER_STATE_TEXTURE;

    int kbhit = 0;
    float theta = 1;
    float pos = 3.5;
    int states[] = { RENDER_STATE_TEXTURE, RENDER_STATE_COLOR, RENDER_STATE_WIREFRAME };
    int indicator = 0;

	while (window.device_exit == 0 && window.device_keys[VK_ESCAPE] == 0) {
        window.win_dispatch(); // 事件分发

        device.device_clear(1);
        device.camera_at_zero(pos, 0, 0);
		
		if (window.device_keys[VK_UP]) pos -= 0.01f;
		if (window.device_keys[VK_DOWN]) pos += 0.01f;
		if (window.device_keys[VK_LEFT]) theta += 0.01f;
		if (window.device_keys[VK_RIGHT]) theta -= 0.01f;

		if (window.device_keys[VK_SPACE]) {
			if (kbhit == 0) {
				kbhit = 1;
				if (++indicator >= 3) indicator = 0;
				device.render_state = states[indicator];
			}
		} else {
			kbhit = 0;
		}

        device.draw_box(theta);
        window.screen_update();
		Sleep(1);
	}
	return 0;
}
