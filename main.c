#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846

#define WIDTH 1920
#define HEIGHT 1080

#define BPP 4

static int showMetrics = 1;

static int x = 10;

static int dynWidth = WIDTH;
static int dynHeight = HEIGHT;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct pixel {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

static struct pixel pixels[WIDTH][HEIGHT];
static void* bitmap_encoded;

void pixels_init() {
	if(bitmap_encoded == NULL) bitmap_encoded = malloc(WIDTH * HEIGHT * 4);

	struct pixel px = {
		0,
		0,
		0
	};

	for (int x = 0; x < dynWidth; x++) {
		for (int y = 0; y < dynHeight; y++) {
			pixels[x][y] = px;
		}
	}
}

void pixels_encode() {
	int offset = 0;

	for (int y = 0; y < dynHeight; y++) {
		for (int x = 0; x < dynWidth; x++) {
			((char*)bitmap_encoded)[offset] = pixels[x][y].b;
			((char*)bitmap_encoded)[offset + 1] = pixels[x][y].g;
			((char*)bitmap_encoded)[offset + 2] = pixels[x][y].r;
			((char*)bitmap_encoded)[offset + 3] = 0xff;
			offset += 4;
		}
	}
}

void pixels_put(int x, int y, struct pixel px) {
	pixels[x][y] = px;
}

struct pixel pixels_get(int x, int y) {
	return pixels[x][y];
}

BOOL pixels_in_range(int x, int y) {
	return x >= 0 && y >= 0 && x < dynWidth && y < dynHeight;
}

double pixels_hue_to_rgb(double p, double q, double t) {
	if (t < 0) t += 1.0;
	if (t > 1) t -= 1.0;
	if (t < 1.0 / 6.0) return p + (q - p) * 6.0 * t;
	if (t < 1.0 / 2.0) return q;
	if (t < 2.0 / 3.0) return p + (q - p) * (2.0 / 3.0 - t) * 6.0;
	return p;
}

struct pixel pixels_hsl_to_pixel(double h, double s, double l) {
	double r, g, b;
	if (s == 0) {
		r = g = b = l;
	}
	else {
		double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
		double p = 2.0 * l - q;

		r = pixels_hue_to_rgb(p, q, h + 1.0 / 3.0);
		g = pixels_hue_to_rgb(p, q, h);
		b = pixels_hue_to_rgb(p, q, h - 1.0 / 3.0);
	}

	struct pixel px = {
		(int) round(r * 255.0),
		(int) round(g * 255.0),
		(int) round(b * 255.0)
	};

	return px;
}

void pixels_modify(HWND hwnd, int frame_number) {
	pixels_init();

	double step = 1.0 / 100.0;
	double x = 0;
	double y = 0;

	for (double i = 0; i < (dynWidth / 2 + dynHeight / 2) / 2; i += step) {
		x = (dynWidth / 2) + i * sin((i/20.0)+((double)frame_number/10.0));
		y = (dynHeight / 2) - i * cos(i/20.0);

		if (pixels_in_range((int)round(x), (int)round(y))) {
			struct pixel px = pixels_hsl_to_pixel(
				abs(cos(i / 20.0)),
				0.5,
				1
			);

			pixels_put((int)round(x), (int)round(y), px);
		}

		double k = x - i * sin(cos(i));
		double j = y + i * cos(sin(i));

		if (pixels_in_range((int)round(j), (int)round(k))) {
			struct pixel px = pixels_hsl_to_pixel(
				abs(cos(i / 40.0)),
				0.5,
				1
			);

			pixels_put((int)round(j), (int)round(k), px);
		}
	}
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	srand(time(NULL));

	pixels_init();
	pixels_encode();

	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Sample Window Class";

	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// Create the window.
	
	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		L"Learn to Program Windows",    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd == NULL)
	{
		return -1;
	}

	LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
	lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
	SetWindowLong(hwnd, GWL_STYLE, lStyle);

	SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, WIDTH, HEIGHT, 0x0);

	ShowCursor(FALSE);

	ShowWindow(hwnd, nCmdShow);

	// Run the message loop.

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return -4;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY: {

		SetWindowTextA(hwnd, L"Exiting");
		Sleep(1000);
		PostQuitMessage(-6);
		break;
	}
	case WM_KEYDOWN: {
		if (wParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}
		else if (wParam == VK_SPACE) {
			showMetrics = abs(showMetrics - 1);
		}
		break;
	}
	case WM_PAINT:
	{
		RECT screen;
		GetWindowRect(hwnd, &screen);

		dynWidth = screen.right - screen.left;
		dynHeight = screen.bottom - screen.top;

		if (dynHeight > HEIGHT) {
			dynHeight = HEIGHT;
		}

		if (dynWidth > WIDTH) {
			dynWidth = WIDTH;
		}

		clock_t before = clock();
		HDC hdc = GetDC(hwnd);
		x += 1;

		pixels_modify(hwnd, x);
		pixels_encode();

		HBITMAP bitmap = CreateBitmap(
				dynWidth,
				dynHeight,
				1,
				32,
				bitmap_encoded
		);

		RECT r = {
			.left = 0,
			.top = 0,
			.right = dynWidth,
			.bottom = dynHeight
		};

		HBRUSH brush = CreatePatternBrush(bitmap);
		int res = FillRect(hdc, &r, brush);

		if (showMetrics) {
			clock_t after = clock() - before;
			int msec = msec = after * 1000 / CLOCKS_PER_SEC;

			int elapsed = clock() / CLOCKS_PER_SEC;
			float fps = x / (elapsed == 0 ? 1 : elapsed);

			char str[100];
			sprintf_s(str, sizeof(str), "%.0f FPS / %d ms\nFrame #%d", fps, msec, x);

			GetClientRect(hwnd, &screen);

			HFONT font = CreateFont(30, 0, 0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET,
				OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
				VARIABLE_PITCH | FF_SWISS, L"Comic Sans MS");

			SelectObject(hdc, font);

			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, RGB(255, 255, 255));

			DrawTextA(hdc, str, -1, &screen, DT_LEFT | DT_TOP);
		}

		ReleaseDC(hwnd, hdc);

		DeleteObject(brush);
		DeleteObject(bitmap);
	}
	return -3;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
