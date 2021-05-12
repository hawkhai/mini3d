#include "mini3d.h"

Screen screen;
int Screen::screen_exit;
int Screen::screen_keys[512];	// 当前键盘按下状态

// 主程序
vertex_t mesh[8] = {
	{ { -1, -1,  1, 1 }, { 0, 0 }, { 1.0f, 0.2f, 0.2f }, 1 },
	{ {  1, -1,  1, 1 }, { 0, 1 }, { 0.2f, 1.0f, 0.2f }, 1 },
	{ {  1,  1,  1, 1 }, { 1, 1 }, { 0.2f, 0.2f, 1.0f }, 1 },
	{ { -1,  1,  1, 1 }, { 1, 0 }, { 1.0f, 0.2f, 1.0f }, 1 },
	{ { -1, -1, -1, 1 }, { 0, 0 }, { 1.0f, 1.0f, 0.2f }, 1 },
	{ {  1, -1, -1, 1 }, { 0, 1 }, { 0.2f, 1.0f, 1.0f }, 1 },
	{ {  1,  1, -1, 1 }, { 1, 1 }, { 1.0f, 0.3f, 0.3f }, 1 },
	{ { -1,  1, -1, 1 }, { 1, 0 }, { 0.2f, 1.0f, 0.3f }, 1 },
};

void draw_plane(device_t *device, int a, int b, int c, int d) {
	vertex_t p1 = mesh[a], p2 = mesh[b], p3 = mesh[c], p4 = mesh[d];
	p1.tc.u = 0, p1.tc.v = 0, p2.tc.u = 0, p2.tc.v = 1;
	p3.tc.u = 1, p3.tc.v = 1, p4.tc.u = 1, p4.tc.v = 0;
	device_draw_primitive(device, &p1, &p2, &p3);
	device_draw_primitive(device, &p3, &p4, &p1);
}

void draw_box(device_t *device, float theta) {
	matrix_t m;
	matrix_set_rotate(&m, -1, -0.5, 1, theta);
	device->transform.world = m;
	transform_update(&device->transform);
	draw_plane(device, 0, 1, 2, 3);
	draw_plane(device, 7, 6, 5, 4);
	draw_plane(device, 0, 4, 5, 1);
	draw_plane(device, 1, 5, 6, 2);
	draw_plane(device, 2, 6, 7, 3);
	draw_plane(device, 3, 7, 4, 0);
}

void camera_at_zero(device_t *device, float x, float y, float z) {
	point_t eye = { x, y, z, 1 }, at = { 0, 0, 0, 1 }, up = { 0, 0, 1, 1 };
	matrix_set_lookat(&device->transform.view, &eye, &at, &up);
	transform_update(&device->transform);
}

void init_texture(device_t *device) {
	static UINT32 texture[256][256];
	int i, j;
	for (j = 0; j < 256; j++) {
		for (i = 0; i < 256; i++) {
			int x = i / 32, y = j / 32;
			texture[j][i] = ((x + y) & 1)? 0xffffff : 0x3fbcef;
		}
	}
	device_set_texture(device, texture, 256 * 4, 256, 256);
}

int main(void)
{
	device_t device;
	int states[] = { RENDER_STATE_TEXTURE, RENDER_STATE_COLOR, RENDER_STATE_WIREFRAME };
	int indicator = 0;
	int kbhit = 0;
	float alpha = 1;
	float pos = 3.5;

	TCHAR *title = _T("Mini3d (software render tutorial) - ")
		_T("Left/Right: rotation, Up/Down: forward/backward, Space: switch state");

	if (screen.screen_init(800, 600, title))
		return -1;

	device_init(&device, 800, 600, screen.screen_fb);
	camera_at_zero(&device, 3, 0, 0);

	init_texture(&device);
	device.render_state = RENDER_STATE_TEXTURE;

	while (screen.screen_exit == 0 && screen.screen_keys[VK_ESCAPE] == 0) {
        screen.screen_dispatch();
		device_clear(&device, 1);
		camera_at_zero(&device, pos, 0, 0);
		
		if (screen.screen_keys[VK_UP]) pos -= 0.01f;
		if (screen.screen_keys[VK_DOWN]) pos += 0.01f;
		if (screen.screen_keys[VK_LEFT]) alpha += 0.01f;
		if (screen.screen_keys[VK_RIGHT]) alpha -= 0.01f;

		if (screen.screen_keys[VK_SPACE]) {
			if (kbhit == 0) {
				kbhit = 1;
				if (++indicator >= 3) indicator = 0;
				device.render_state = states[indicator];
			}
		}	else {
			kbhit = 0;
		}

		draw_box(&device, alpha);
        screen.screen_update();
		Sleep(1);
	}
	return 0;
}

