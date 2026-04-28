#include <xiamocon.hpp>

using namespace xmc;
using namespace xmc::input;

// Defines the pixel format for the display.
// This should match the pixel format used by the frame buffer.
static constexpr PixelFormat DISPLAY_FORMAT = PixelFormat::RGB565;

// Create a frame buffer for rendering.
// This example uses a single buffer (not double-buffered).
FrameBuffer frameBuffer = createFrameBuffer(DISPLAY_FORMAT, false);

// Create a frame rate keeper to maintain a consistent frame rate of 30 FPS.
FpsKeeper fpsKeeper(30);

void drawButtonState(Graphics2D &gfx, Button btn, int x, int y);

// Application configuration function. This function is called by the Xiamocon
// framework to retrieve the application's configuration settings.
AppConfig xmcAppGetConfig(void) {
  AppConfig cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = DISPLAY_FORMAT;
  cfg.speakerEnabled = false;
  return cfg;
}

// Application setup function. This function is called once when the application
// starts. It is used to perform any necessary initialization before the main
// loop begins.
void xmcAppSetup(void) {
  frameBuffer->enableFlag(FrameBufferFlags::SHOW_DEBUG_INFO);
}

// Main application loop. This function is called repeatedly by the Xiamocon
// framework to update the application's state and render graphics.
void xmcAppLoop(void) {
  // Wait for the next vertical sync (VSync) time to maintain a consistent frame
  // rate. If the last frame took too long to render, the FpsKeeper will
  // indicate that frame skipping is active, and we can choose to skip rendering
  // this frame to catch up.
  fpsKeeper.waitVsync();

  // If frame skipping is not active, proceed with rendering the frame.
  if (!fpsKeeper.isFrameSkipping()) {
    // Begin rendering to the frame buffer. This will prepare the back buffer
    // for drawing. If double buffering is disabled and a transfer to display is
    // in progress, this function will wait for the transfer to complete before
    // allowing rendering to begin.
    frameBuffer->beginRender();

    // Create a Graphics2D object for rendering to the back buffer of the frame
    // buffer. This object provides various drawing functions for rendering
    // shapes, text, and images.
    Graphics2D gfx = frameBuffer->createGraphics();

    // Clear the back buffer with a white background color.
    gfx->clear(gfx->devColor(Colors::WHITE));

    // Set the text color to black.
    gfx->setTextColor(gfx->devColor(Colors::BLACK));

    // Draw some text to the back buffer.
    gfx->setCursor(10, 30);
    gfx->setFont(&ShapoSansP_s21c16a01w03);
    gfx->drawString("Hello, Xiamocon!");

    gfx->setCursor(10, 80);
    gfx->setFont(&ShapoSansP_s12c09a01w02);
    gfx->drawString("Button States:");

    // Draw the state of each button on the back buffer. The drawButtonState
    // function will draw a filled rectangle for each button, with the color
    // indicating whether the button is pressed or not.
    int xl = 50;
    int xr = 170;
    int y = 140;
    drawButtonState(gfx, Button::LEFT, xl - 30, y);
    drawButtonState(gfx, Button::UP, xl, y - 30);
    drawButtonState(gfx, Button::RIGHT, xl + 30, y);
    drawButtonState(gfx, Button::DOWN, xl, y + 30);
    drawButtonState(gfx, Button::Y, xr - 30, y);
    drawButtonState(gfx, Button::X, xr, y - 30);
    drawButtonState(gfx, Button::A, xr + 30, y);
    drawButtonState(gfx, Button::B, xr, y + 30);
    drawButtonState(gfx, Button::FUNC, xr + 30, y - 60);
    frameBuffer->endRender();
  }
}

void drawButtonState(Graphics2D &gfx, Button btn, int x, int y) {
  // Check if the specified button is currently pressed. If it is pressed, fill
  // a rectangle with red color; otherwise, fill it with cyan color. Then, draw
  // a black border around the rectangle to indicate the button's position. The
  // rectangle represents the state of the button, with the color indicating
  // whether it is pressed or not.
  if (isPressed(btn)) {
    gfx->fillRect(x, y, 20, 20, gfx->devColor(Colors::RED));
  } else {
    gfx->fillRect(x, y, 20, 20, gfx->devColor(Colors::CYAN));
  }
  gfx->drawRect(x, y, 20, 20, gfx->devColor(Colors::BLACK));
}
