#include <xiamocon.hpp>

#include <math.h>
#include <stdint.h>

namespace {

using namespace xmc;
using namespace xmc::audio;

constexpr int SCREEN_W = display::WIDTH;
constexpr int SCREEN_H = display::HEIGHT;
constexpr int NUM_TONES = 4;

constexpr int PADDLE_W = 44;
constexpr int PADDLE_H = 6;
constexpr int PADDLE_Y = SCREEN_H - 18;
constexpr int PADDLE_SPEED = 5;

constexpr int BALL_SIZE = 4;

constexpr int BRICK_COLS = 10;
constexpr int BRICK_ROWS = 6;
constexpr int BRICK_GAP = 2;
constexpr int BRICK_MARGIN_X = 10;
constexpr int BRICK_TOP = 24;
constexpr int BRICK_H = 10;
constexpr int BRICK_W =
    (SCREEN_W - BRICK_MARGIN_X * 2 - BRICK_GAP * (BRICK_COLS - 1)) / BRICK_COLS;

constexpr uint16_t COLOR_BG = clipPack444(0, 0, 0);
constexpr uint16_t COLOR_WALL = clipPack444(1, 1, 2);
constexpr uint16_t COLOR_PADDLE = clipPack444(2, 15, 15);
constexpr uint16_t COLOR_BALL = clipPack444(15, 15, 15);
constexpr uint16_t COLOR_SERVE = clipPack444(15, 15, 0);
constexpr uint16_t COLOR_CLEAR = clipPack444(0, 10, 0);
constexpr uint16_t COLOR_GAME_OVER = clipPack444(10, 0, 0);

struct Brick {
  int x;
  int y;
  bool alive;
  uint16_t color;
};

enum class GameState {
  Serve,
  Playing,
  Clear,
  GameOver,
};

FrameBuffer frameBuffer = createFrameBuffer(PixelFormat::RGB444, true);
FpsKeeper fpsKeeper(60.0f);

Brick bricks[BRICK_ROWS][BRICK_COLS];
Mixer mixer = createMixer(NUM_TONES);
Tone tones[NUM_TONES];

GameState gameState = GameState::Serve;
int nextToneIndex = 0;

int paddleX = (SCREEN_W - PADDLE_W) / 2;

float ballX = 0.0f;
float ballY = 0.0f;
float ballVX = 0.0f;
float ballVY = 0.0f;

int bricksLeft = 0;

void playTone(uint8_t note, uint32_t lenMs, audio::Waveform waveform,
              uint8_t velocity, uint16_t attackMs, uint16_t decayMs,
              uint16_t sustain, uint16_t releaseMs, int32_t sweep_delta,
              uint32_t sweepPeriodMs) {
  audio::Tone &tone = tones[nextToneIndex];
  nextToneIndex = (nextToneIndex + 1) % NUM_TONES;

  tone->setWaveform(waveform);
  tone->setVelocity(velocity);
  tone->setEnvelope(attackMs, decayMs, sustain, releaseMs);
  tone->setSweep(sweep_delta, sweepPeriodMs);
  tone->noteOn(note, lenMs);
}

void sfxLaunch() {
  playTone(76, 90, Waveform::SQUARE, 96, 0, 60, 140, 40, 2800, 7);
}

void sfxWallBounce() {
  playTone(82, 28, Waveform::SQUARE, 72, 0, 24, 96, 18, 0, 10);
}

void sfxPaddleBounce() {
  playTone(62, 45, Waveform::TRIANGLE, 86, 0, 30, 120, 24, 900, 8);
}

void sfxBrickBreak() {
  playTone(72, 55, Waveform::SQUARE, 88, 0, 40, 96, 28, -1700, 10);
}

void sfxClear() {
  playTone(76, 130, Waveform::TRIANGLE, 104, 0, 80, 160, 40, 2200, 9);
  playTone(83, 150, Waveform::SINE, 90, 0, 100, 140, 50, 1100, 12);
}

void sfxGameOver() {
  playTone(45, 180, Waveform::SAWTOOTH, 92, 0, 120, 110, 70, -2400, 12);
}

bool intersectsRect(int ax, int ay, int aw, int ah, int bx, int by, int bw,
                    int bh) {
  return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

void resetBallOnPaddle() {
  ballX = static_cast<float>(paddleX + PADDLE_W / 2 - BALL_SIZE / 2);
  ballY = static_cast<float>(PADDLE_Y - BALL_SIZE - 1);
  ballVX = 0.0f;
  ballVY = 0.0f;
}

void buildStage() {
  static constexpr uint16_t row_colors[BRICK_ROWS] = {
      clipPack444(15, 4, 4), clipPack444(15, 8, 2),  clipPack444(15, 12, 2),
      clipPack444(6, 15, 4), clipPack444(4, 10, 15), clipPack444(10, 6, 15),
  };

  bricksLeft = BRICK_ROWS * BRICK_COLS;
  for (int row = 0; row < BRICK_ROWS; ++row) {
    for (int col = 0; col < BRICK_COLS; ++col) {
      Brick &brick = bricks[row][col];
      brick.x = BRICK_MARGIN_X + col * (BRICK_W + BRICK_GAP);
      brick.y = BRICK_TOP + row * (BRICK_H + BRICK_GAP);
      brick.alive = true;
      brick.color = row_colors[row];
    }
  }
}

void resetGame() {
  paddleX = (SCREEN_W - PADDLE_W) / 2;
  buildStage();
  resetBallOnPaddle();
  gameState = GameState::Serve;
}

void launchBall(input::Button buttons) {
  float dir = 0.0f;
  if (hasFlag(buttons, input::Button::LEFT)) {
    dir = -1.0f;
  }
  if (hasFlag(buttons, input::Button::RIGHT)) {
    dir = 1.0f;
  }
  ballVX = dir * 1.4f;
  ballVY = -2.2f;
  gameState = GameState::Playing;
  sfxLaunch();
}

void movePaddle(input::Button buttons) {
  if (hasFlag(buttons, input::Button::LEFT)) {
    paddleX -= PADDLE_SPEED;
  }
  if (hasFlag(buttons, input::Button::RIGHT)) {
    paddleX += PADDLE_SPEED;
  }
  if (paddleX < 0) {
    paddleX = 0;
  }
  if (paddleX > SCREEN_W - PADDLE_W) {
    paddleX = SCREEN_W - PADDLE_W;
  }
}

void updateBall() {
  const float prev_x = ballX;
  const float prev_y = ballY;

  ballX += ballVX;
  ballY += ballVY;

  if (ballX < 0.0f) {
    ballX = 0.0f;
    ballVX = fabsf(ballVX);
    sfxWallBounce();
  } else if (ballX + BALL_SIZE > SCREEN_W) {
    ballX = static_cast<float>(SCREEN_W - BALL_SIZE);
    ballVX = -fabsf(ballVX);
    sfxWallBounce();
  }

  if (ballY < 0.0f) {
    ballY = 0.0f;
    ballVY = fabsf(ballVY);
    sfxWallBounce();
  }

  if (ballY > SCREEN_H) {
    gameState = GameState::GameOver;
    sfxGameOver();
    return;
  }

  const int biX = static_cast<int>(ballX);
  const int biY = static_cast<int>(ballY);

  if (ballVY > 0.0f && intersectsRect(biX, biY, BALL_SIZE, BALL_SIZE, paddleX,
                                      PADDLE_Y, PADDLE_W, PADDLE_H)) {
    ballY = static_cast<float>(PADDLE_Y - BALL_SIZE - 1);
    const float hit =
        ((ballX + BALL_SIZE * 0.5f) - (paddleX + PADDLE_W * 0.5f)) /
        (PADDLE_W * 0.5f);
    ballVX = hit * 2.4f;
    ballVY = -fabsf(ballVY);
    const float speed = sqrtf(ballVX * ballVX + ballVY * ballVY);
    if (speed < 3.2f) {
      const float scale = 3.2f / speed;
      ballVX *= scale;
      ballVY *= scale;
    }
    sfxPaddleBounce();
  }

  for (int row = 0; row < BRICK_ROWS; ++row) {
    for (int col = 0; col < BRICK_COLS; ++col) {
      Brick &brick = bricks[row][col];
      if (!brick.alive) {
        continue;
      }
      if (!intersectsRect(biX, biY, BALL_SIZE, BALL_SIZE, brick.x, brick.y,
                          BRICK_W, BRICK_H)) {
        continue;
      }

      brick.alive = false;
      --bricksLeft;
      sfxBrickBreak();

      const float prev_right = prev_x + BALL_SIZE;
      const float prev_bottom = prev_y + BALL_SIZE;
      const bool from_left = prev_right <= brick.x;
      const bool from_right = prev_x >= brick.x + BRICK_W;
      const bool from_top = prev_bottom <= brick.y;
      const bool from_bottom = prev_y >= brick.y + BRICK_H;

      if (from_left || from_right) {
        ballVX = -ballVX;
      } else if (from_top || from_bottom) {
        ballVY = -ballVY;
      } else {
        ballVY = -ballVY;
      }

      if (bricksLeft <= 0) {
        gameState = GameState::Clear;
        sfxClear();
      }
      return;
    }
  }
}

void drawScene() {
  Graphics2D gfx = frameBuffer->createGraphics();
  gfx->clear(COLOR_BG);

  gfx->fillRect(0, 0, SCREEN_W, 2, COLOR_WALL);

  for (int row = 0; row < BRICK_ROWS; ++row) {
    for (int col = 0; col < BRICK_COLS; ++col) {
      const Brick &brick = bricks[row][col];
      if (!brick.alive) {
        continue;
      }
      gfx->fillRect(brick.x, brick.y, BRICK_W, BRICK_H, brick.color);
    }
  }

  gfx->fillRect(paddleX, PADDLE_Y, PADDLE_W, PADDLE_H, COLOR_PADDLE);
  gfx->fillRect(static_cast<int>(ballX), static_cast<int>(ballY), BALL_SIZE,
                BALL_SIZE, COLOR_BALL);

  if (gameState == GameState::Serve) {
    gfx->fillRect(SCREEN_W / 2 - 40, SCREEN_H / 2 - 8, 80, 16, COLOR_SERVE);
  } else if (gameState == GameState::Clear) {
    gfx->fillRect(SCREEN_W / 2 - 52, SCREEN_H / 2 - 12, 104, 24, COLOR_CLEAR);
  } else if (gameState == GameState::GameOver) {
    gfx->fillRect(SCREEN_W / 2 - 52, SCREEN_H / 2 - 12, 104, 24,
                  COLOR_GAME_OVER);
  }

  frameBuffer->renderStatusBar(gfx);
  frameBuffer->renderDebugBar(gfx);
}

}  // namespace

AppConfig xmcAppGetConfig(void) {
  AppConfig cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = PixelFormat::RGB444;
  cfg.speakerEnabled = true;
  return cfg;
}

void xmcAppSetup(void) {
  for (int i = 0; i < NUM_TONES; ++i) {
    tones[i] = createTone();
    tones[i]->init();
    mixer->setSource(i, tones[i]->getOutputPort());
  }
  speaker::setSourcePort(mixer->getOutputPort());
  speaker::setMuted(false);

  resetGame();
}

void xmcAppLoop(void) {
  const input::Button buttons = input::getState();

  movePaddle(buttons);

  if (gameState == GameState::Serve) {
    resetBallOnPaddle();
    if (hasFlag(buttons, input::Button::A | input::Button::B |
                             input::Button::X | input::Button::Y |
                             input::Button::UP)) {
      launchBall(buttons);
    }
  } else if (gameState == GameState::Playing) {
    updateBall();
  } else {
    if (hasFlag(buttons, input::Button::A | input::Button::B |
                             input::Button::X | input::Button::Y |
                             input::Button::UP)) {
      resetGame();
    }
  }

  fpsKeeper.waitVsync();
  if (!fpsKeeper.isFrameSkipping()) {
    frameBuffer->beginRender();
    drawScene();
    frameBuffer->endRender();
  }
}
