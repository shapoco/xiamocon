#include "xiamocon.hpp"

#include "bmp_back_data.h"
#include "bmp_chara_data.h"
#include "bmp_miku_data.h"

#include <math.h>
#include <stdint.h>

using namespace xmc;
using namespace xmc::audio;
using namespace xmc::input;

static constexpr int NUM_KEYS = 8;

static constexpr int NUM_MAX_ENEMIES = 8;
static constexpr int DAMAGING_MS = 500;
static constexpr int INVICIBILITY_MS = 3000;
static constexpr int WEED_NUM_PATTERNS = 8;
static constexpr int WEED_PATTERN_SIZE = 16;
static constexpr int WEED_ARRAY_LENGTH = display::WIDTH / WEED_PATTERN_SIZE + 1;
static constexpr int CLOUD_ARRAY_INTERVAL = 16;
static constexpr int CLOUD_ARRAY_LENGTH =
    (display::WIDTH / CLOUD_ARRAY_INTERVAL) * 32;
static constexpr int MAX_CLOUD_SPRITES = CLOUD_ARRAY_LENGTH * 4;

struct SpriteFrame {
  int x, y;
  int width, height;
  int offsetX, offsetY;
};

enum class MikuState {
  RUN,
  JUMP,
  LAND,
  GAME_OVER,
};

enum class EnemyType {
  CACTUS,
  BIG_CACTUS,
  NUM_TYPES,
};

struct Enemy {
  bool active;
  float x, y;
  EnemyType type;
};

struct CloudSprite {
  int x, y;
  int patternIndex;
};

struct WeedArray {
  uint8_t patterns[WEED_ARRAY_LENGTH];
  float offset;
};

enum class Sound {
  STEP,
  JUMP,
  LAND,
  DAMAGE_NOISE,
  DAMAGE_PULSE,
  GAME_OVER,
  NUM_SOUNDS,
};
static constexpr int NUM_TONES = (int)Sound::NUM_SOUNDS;

Sprite frameBuffers[] = {
    createSprite444(display::WIDTH, display::HEIGHT),
    createSprite444(display::WIDTH, display::HEIGHT),
};
int backIndex = 0;

Sprite bmpChara = createSprite4444(256, 256, 0, (void *)bmp_chara_data);
Sprite bmpBack = createSprite4444(256, 256, 0, (void *)bmp_back_data);
Sprite bmpMiku = createSprite4444(256, 256, 0, (void *)bmp_miku_data);
uint64_t nextVsyncTimeUs = 0;

uint64_t lastMs = 0;

int groundY = 200;
float level = 1;
float speed = 3.0f;

float mikuX = 64;
float mikuY = groundY;
float mikuVX = 0;
float mikuVY = 0;
bool mikuJumpingUp = false;
int mikuAnimeIndex = 0;
uint64_t mikuStateChangeTimeMs = 0;
uint64_t mikuLastDamageTimeMs = 0;
MikuState mikuState = MikuState::RUN;
int mikuLife = 3;

Enemy enemies[NUM_MAX_ENEMIES];
uint64_t lastEnemySpawnTimeMs = 0;
EnemyType lastEnemyType = EnemyType::CACTUS;

WeedArray weedArrayFront;
WeedArray weedArrayBack;

int cloudY[CLOUD_ARRAY_LENGTH];
CloudSprite cloudSprites[MAX_CLOUD_SPRITES];
int numCloudSprites = 0;
float cloudOffset = 0;

SpriteFrame mikuAnimeRun[] = {
    {0 * 64, 0 * 64, 64, 64, -48, -64}, {1 * 64, 0 * 64, 64, 64, -48, -64},
    {2 * 64, 0 * 64, 64, 64, -48, -64}, {0 * 64, 1 * 64, 64, 64, -48, -64},
    {1 * 64, 1 * 64, 64, 64, -48, -64}, {2 * 64, 1 * 64, 64, 64, -48, -64},
};

SpriteFrame mikuAnimeJumpUp[] = {
    {0 * 64, 2 * 64, 64, 64, -48, -64},
    {1 * 64, 2 * 64, 64, 64, -48, -64},
};
SpriteFrame mikuAnimeJumpTop[] = {
    {3 * 64, 0 * 64, 64, 64, -48, -64},
};
SpriteFrame mikuAnimeJumpDown[] = {
    {2 * 64, 2 * 64, 64, 64, -48, -64},
    {3 * 64, 2 * 64, 64, 64, -48, -64},
};
SpriteFrame mikuAnimeLand[] = {
    {3 * 64, 1 * 64, 64, 64, -48, -64},
};
SpriteFrame mikuAnimeDamage[] = {
    {0 * 64, 3 * 64, 64, 64, -48, -60},
};

static Mixer mixer(NUM_TONES);
static Tone tones[NUM_TONES];

static void generateCloud();
static void generateCloudRecursive(int i0, int i1);

static void updateScene();
static void updateEnemies();
static void updateCloud();
static void updateWeeds(WeedArray &weedArray, float speedCoeff);
static void updateMiku(uint64_t nowMs);
static void mikuJump();

static void renderScene(Graphics2D &gfx);
static void renderEnemies(Graphics2D &gfx);
static void renderCloud(Graphics2D &gfx);
static void renderWeeds(Graphics2D &gfx, const WeedArray &weedArray, int sx,
                        int sy);
static void renderSpriteFrame(Graphics2D &gfx, const Sprite &bmp,
                              const SpriteFrame &f, int x, int y);
static void renderStatus(Graphics2D &gfx);

static void waitVsync();

AppConfig xmc::appGetConfig() {
  AppConfig cfg = getDefaultAppConfig();
  cfg.displayPixelFormat = display::InterfaceFormat::RGB444;
  cfg.speakerSampleFormat = SampleFormat::LINEAR_PCM_S16_MONO;
  return cfg;
}

void xmc::appSetup() {
  for (int i = 0; i < NUM_TONES; i++) {
    tones[i].init(audio::getPreferredSamplingRate());
    mixer.setSource(i, tones[i].getOutputPort());
  }

  speaker::setSourcePort(mixer.getOutputPort());
  speaker::setMuted(false);
  gpio::setDir(XMC_PIN_GPIO_0, true);

  for (int i = 0; i < NUM_MAX_ENEMIES; i++) {
    enemies[i].active = false;
    enemies[i].x = 0;
    enemies[i].y = 0;
    enemies[i].type = EnemyType::CACTUS;
  }

  for (int i = 0; i < WEED_ARRAY_LENGTH; i++) {
    weedArrayFront.patterns[i] = randomU32() % WEED_NUM_PATTERNS;
    weedArrayFront.offset = 0;
    weedArrayBack.patterns[i] = randomU32() % WEED_NUM_PATTERNS;
    weedArrayBack.offset = 0;
  }

  generateCloud();

  tones[(int)Sound::STEP].setWaveform(Waveform::NOISE);
  tones[(int)Sound::STEP].setEnvelope(0, 0, 255, 0);
  tones[(int)Sound::STEP].setVelocity(48);

  tones[(int)Sound::JUMP].setWaveform(Waveform::SQUARE);
  tones[(int)Sound::JUMP].setEnvelope(0, 0, 255, 300);
  tones[(int)Sound::JUMP].setSweep(0x400, 10);
  tones[(int)Sound::JUMP].setVelocity(64);

  tones[(int)Sound::LAND].setWaveform(Waveform::SQUARE);
  tones[(int)Sound::LAND].setEnvelope(0, 0, 255, 100);
  tones[(int)Sound::LAND].setSweep(-0x1000, 10);
  tones[(int)Sound::LAND].setVelocity(96);

  tones[(int)Sound::DAMAGE_NOISE].setWaveform(Waveform::NOISE);
  tones[(int)Sound::DAMAGE_NOISE].setEnvelope(0, 0, 255, 0);
  tones[(int)Sound::DAMAGE_NOISE].setSweep(-0x1000, 10);
  tones[(int)Sound::DAMAGE_NOISE].setVelocity(64);

  tones[(int)Sound::DAMAGE_PULSE].setWaveform(Waveform::SQUARE);
  tones[(int)Sound::DAMAGE_PULSE].setEnvelope(0, 0, 255, 0);
  tones[(int)Sound::DAMAGE_PULSE].setSweep(-0x800, 10);
  tones[(int)Sound::DAMAGE_PULSE].setVelocity(32);

  tones[(int)Sound::GAME_OVER].setWaveform(Waveform::SQUARE);
  tones[(int)Sound::GAME_OVER].setEnvelope(0, 0, 255, 0);
  tones[(int)Sound::GAME_OVER].setSweep(-0x400, 10);
  tones[(int)Sound::GAME_OVER].setVelocity(64);
}

void xmc::appLoop() {
  int frontIndex = (backIndex + 1) % 2;

  updateScene();

  Graphics2D gfx = createGraphics2D(frameBuffers[backIndex]);
  renderScene(gfx);

  // render status bar
  appDrawStatusBar(gfx);
  appDrawDebugInfo(gfx);

  frameBuffers[frontIndex]->completeTransfer();
  waitVsync();
  frameBuffers[backIndex]->startTransferToDisplay(0, 0);

  backIndex = frontIndex;
}

static void generateCloud() {
  for (int i = 0; i < CLOUD_ARRAY_LENGTH; i += 16) {
    cloudY[i] = randomU32() % display::HEIGHT + 64;
  }
  for (int i = 0; i < CLOUD_ARRAY_LENGTH; i += 16) {
    generateCloudRecursive(i, i + 16);
  }

  int spriteMaxDistance = 24;
  for (int i = 0; i < CLOUD_ARRAY_LENGTH; i++) {
    int i0 = i;
    int i1 = (i + 1) % CLOUD_ARRAY_LENGTH;
    int x0 = i * CLOUD_ARRAY_INTERVAL;
    int x1 = x0 + CLOUD_ARRAY_INTERVAL;
    int y0 = cloudY[i0];
    int y1 = cloudY[i1];
    int dx = x1 - x0;
    int dy = y1 - y0;
    int d = sqrt(dx * dx + dy * dy);
    int n = ((d + spriteMaxDistance - 1) / spriteMaxDistance);
    for (int j = 0; j < n; j++) {
      int sx = x0 + (x1 - x0) * j / n;
      int sy = y0 + (y1 - y0) * j / n;
      if (numCloudSprites < MAX_CLOUD_SPRITES) {
        CloudSprite &sprite = cloudSprites[numCloudSprites++];
        sprite.x = sx + CLOUD_ARRAY_INTERVAL / 2;
        sprite.y = sy;
        sprite.patternIndex = randomU32() % 4;
      }
    }
  }
}

static void generateCloudRecursive(int i0, int i1) {
  if (i1 - i0 <= 1) return;
  int im = (i0 + i1) / 2;
  int y0 = cloudY[i0 % CLOUD_ARRAY_LENGTH];
  int y1 = cloudY[i1 % CLOUD_ARRAY_LENGTH];
  int ym = (y0 + y1) / 2 + (randomU32() % 64 - 32);
  cloudY[im % CLOUD_ARRAY_LENGTH] = ym;
  generateCloudRecursive(i0, im);
  generateCloudRecursive(im, i1);
}

static void updateScene() {
  uint64_t nowMs = getTimeMs();

  if (mikuState != MikuState::GAME_OVER) {
    level += 0.001f;
  }
  speed = 1.0f + level;

  updateMiku(nowMs);
  updateCloud();
  updateWeeds(weedArrayBack, 0.8f);
  updateWeeds(weedArrayFront, 1.2f);
  updateEnemies();
}

static void updateCloud() {
  cloudOffset -= speed / 4;
  if (cloudOffset < -CLOUD_ARRAY_INTERVAL * CLOUD_ARRAY_LENGTH) {
    cloudOffset += CLOUD_ARRAY_INTERVAL * CLOUD_ARRAY_LENGTH;
  }
}

static void updateWeeds(WeedArray &weedArray, float speedCoeff) {
  weedArray.offset -= speed * speedCoeff;
  if (weedArray.offset < -WEED_PATTERN_SIZE * WEED_ARRAY_LENGTH) {
    weedArray.offset += WEED_PATTERN_SIZE * WEED_ARRAY_LENGTH;
  }
}

static void updateMiku(uint64_t nowMs) {
  MikuState lastState = mikuState;

  if (mikuState == MikuState::GAME_OVER) {
    return;
  }

  uint64_t elapsedFromDamage = nowMs - mikuLastDamageTimeMs;

  int num_anime_frames = 1;
  switch (mikuState) {
    default:
    case MikuState::RUN:
      mikuX += (64 - mikuX) * 0.05f;
      mikuY = groundY;
      lastMs = nowMs;
      num_anime_frames = 6;
      if (wasPressed(Button::A) && elapsedFromDamage > DAMAGING_MS) {
        mikuJump();
      }
      break;
    case MikuState::JUMP:
      if (mikuJumpingUp && isPressed(Button::A)) {
        mikuVY += 0.15f;
      } else {
        mikuVY += 0.6f;
        mikuJumpingUp = false;
      }
      mikuX += mikuVX;
      mikuY += mikuVY;
      num_anime_frames = 2;
      if (mikuY >= groundY) {
        mikuY = groundY;
        mikuVY = 0;
        mikuState = MikuState::LAND;
        mikuAnimeIndex = 0;
        tones[(int)Sound::LAND].noteOn(52, 1);
      }
      break;
    case MikuState::LAND:
      mikuVX -= speed / 2;
      if (mikuVX < -1) mikuVX = -1;
      mikuX += mikuVX;
      if (wasPressed(Button::A)) {
        mikuJump();
      } else if (nowMs >= mikuStateChangeTimeMs + 100) {
        mikuState = MikuState::RUN;
      }
      break;
  }

  if (elapsedFromDamage > INVICIBILITY_MS) {
    for (int i = 0; i < NUM_MAX_ENEMIES; i++) {
      Enemy &e = enemies[i];
      if (!e.active) continue;
      Rect mikuRect = {(int)(mikuX - 8), (int)(mikuY - 48), 16, 32};
      Rect enemyRect;
      switch (e.type) {
        default:
        case EnemyType::CACTUS:
          enemyRect.width = 32;
          enemyRect.height = 32;
          break;
        case EnemyType::BIG_CACTUS:
          enemyRect.width = 32;
          enemyRect.height = 56;
          break;
      }
      enemyRect.x = e.x - enemyRect.width / 2;
      enemyRect.y = e.y - enemyRect.height;
      Rect intersection = mikuRect.intersect(enemyRect);
      if (intersection.width > 0 && intersection.height > 0) {
        if (mikuLife > 0) {
          mikuLife--;
        }
        mikuLastDamageTimeMs = nowMs;
        tones[(int)Sound::DAMAGE_NOISE].noteOn(48, 200);
        tones[(int)Sound::DAMAGE_PULSE].noteOn(112, 200);
        break;
      }
    }
  }

  if (mikuState != lastState) {
    mikuAnimeIndex = 0;
    mikuStateChangeTimeMs = nowMs;
  }

  int last_anime_index = mikuAnimeIndex;
  mikuAnimeIndex = ((nowMs - mikuStateChangeTimeMs) / 100) % num_anime_frames;
  if (mikuState == MikuState::RUN && mikuAnimeIndex != last_anime_index &&
      mikuAnimeIndex % 3 == 1) {
    tones[(int)Sound::STEP].noteOn(32, 1);
  }

  if (mikuLife <= 0 && elapsedFromDamage > DAMAGING_MS) {
    mikuState = MikuState::GAME_OVER;
    tones[(int)Sound::GAME_OVER].noteOn(64, 1000);
  }
}

static void updateEnemies() {
  uint64_t nowMs = getTimeMs();
  uint32_t elapsedMs = nowMs - lastEnemySpawnTimeMs;
  int32_t mod = 200 - level * 10 - elapsedMs / 10;
  if (mod < 10) mod = 10;
  if (randomU32() % mod == 0 && elapsedMs >= 500) {
    // spawn a new enemy
    for (int i = 0; i < NUM_MAX_ENEMIES; i++) {
      if (!enemies[i].active) {
        if (lastEnemyType == EnemyType::CACTUS && level >= 2 &&
            elapsedMs >= 1000) {
          lastEnemyType = (EnemyType)(randomU32() % (int)EnemyType::NUM_TYPES);
        } else {
          lastEnemyType = EnemyType::CACTUS;
        }
        enemies[i].active = true;
        enemies[i].type = lastEnemyType;
        enemies[i].x = display::WIDTH + 32;
        enemies[i].y = groundY;
        lastEnemySpawnTimeMs = nowMs;
        break;
      }
    }
  }

  for (int i = 0; i < NUM_MAX_ENEMIES; i++) {
    if (enemies[i].active) {
      enemies[i].x -= speed;
      if (enemies[i].x < -32) {
        enemies[i].active = false;
      }
    }
  }
}

static void mikuJump() {
  mikuVY = -5.0f;
  mikuVX = 0.5f;
  mikuState = MikuState::JUMP;
  mikuJumpingUp = true;
  mikuAnimeIndex = 0;
  tones[(int)Sound::JUMP].noteOn(76, 1);
}

static void renderScene(Graphics2D &gfx) {
  uint64_t nowMs = getTimeMs();

  // sky gradient
  for (int y = 0; y < groundY; y++) {
    int dither = (y % 2 == 0) ? -8 : 8;
    uint16_t color = blend4444(0x04C, 0x8CF, y * 256 / groundY + dither);
    gfx->fillRect(0, y, display::WIDTH, 1, color);
  }

  // clouds
  renderCloud(gfx);

  // ground gradient
  int groundH = display::HEIGHT - groundY;
  for (int y = 0; y < groundH; y++) {
    int dither = (y % 2 == 0) ? -4 : 4;
    uint16_t color = blend4444(0x8C1, 0x000, y * 256 / groundH + dither);
    gfx->fillRect(0, groundY + y, display::WIDTH, 1, color);
  }
  // weeds (background)
  renderWeeds(gfx, weedArrayBack, 0, 16);

  // enemies
  renderEnemies(gfx);

  // miku
  SpriteFrame *mikuFrame = nullptr;
  switch (mikuState) {
    case MikuState::RUN: mikuFrame = &mikuAnimeRun[mikuAnimeIndex]; break;
    case MikuState::JUMP:
      if (mikuVY < -3.0f) {
        mikuFrame = &mikuAnimeJumpUp[mikuAnimeIndex];
      } else if (mikuVY > 3.0f) {
        mikuFrame = &mikuAnimeJumpDown[mikuAnimeIndex];
      } else {
        mikuFrame = &mikuAnimeJumpTop[0];
      }
      break;
    case MikuState::LAND: mikuFrame = &mikuAnimeLand[0]; break;
    default: mikuFrame = nullptr; break;
  }
  uint64_t elapsedFromDamage = nowMs - mikuLastDamageTimeMs;
  if (elapsedFromDamage < DAMAGING_MS) {
    mikuFrame = &mikuAnimeDamage[0];
  }
  if (mikuFrame && (elapsedFromDamage >= INVICIBILITY_MS || nowMs % 128 < 64)) {
    renderSpriteFrame(gfx, bmpMiku, *mikuFrame, mikuX, mikuY);
  }

  // weeds (foreground)
  renderWeeds(gfx, weedArrayFront, 0, 0);

  // status
  renderStatus(gfx);
}

static void renderEnemies(Graphics2D &gfx) {
  for (int i = 0; i < NUM_MAX_ENEMIES; i++) {
    if (!enemies[i].active) continue;
    switch (enemies[i].type) {
      default:
      case EnemyType::CACTUS:
        gfx->drawImage(bmpChara, (int)enemies[i].x - 24, (int)enemies[i].y - 48,
                       48, 48, 0, 0);
        break;
      case EnemyType::BIG_CACTUS:
        gfx->drawImage(bmpChara, (int)enemies[i].x - 24, (int)enemies[i].y - 64,
                       48, 64, 48, 0);
        break;
    }
  }
}

static void renderCloud(Graphics2D &gfx) {
  {
    int i0 = (int)(-cloudOffset / CLOUD_ARRAY_INTERVAL) - 1;
    int i1 = i0 + display::WIDTH / CLOUD_ARRAY_INTERVAL + 2;
    int x0 = (int)cloudOffset + i0 * CLOUD_ARRAY_INTERVAL;
    for (int i = i0; i <= i1; i++) {
      int i_mod = (i + CLOUD_ARRAY_LENGTH) % CLOUD_ARRAY_LENGTH;
      int x = x0 + (i - i0) * CLOUD_ARRAY_INTERVAL;
      int y = cloudY[i_mod];
      int h = groundY - y;
      if (h > 0) {
        gfx->fillRect(x, y, CLOUD_ARRAY_INTERVAL, h, 0xBEF);
      }
    }
  }
  if (numCloudSprites > 0) {
    int i0 = 0;
    int x = 0;
    for (int i = 0; i < numCloudSprites; i++) {
      CloudSprite &sprite = cloudSprites[i];
      if (sprite.x > -16) {
        i0 = i;
        x = sprite.x + (int)cloudOffset;
        break;
      }
    }
    while (x < display::WIDTH + 16) {
      int i1 = (i0 + 1) % numCloudSprites;
      CloudSprite &sprite = cloudSprites[i1];
      int dx = sprite.x - cloudSprites[i0].x;
      if (dx < 0) {
        dx += CLOUD_ARRAY_LENGTH * CLOUD_ARRAY_INTERVAL;
      }
      x += dx;
      int sx = sprite.patternIndex * 32;
      int sy = 32;
      gfx->drawImage(bmpBack, x - 16, sprite.y - 16, 32, 32, sx, sy);
      i0 = i1;
    }
  }
}

static void renderWeeds(Graphics2D &gfx, const WeedArray &weedArray, int sx,
                        int sy) {
  for (int i = 0; i < WEED_ARRAY_LENGTH; i++) {
    int sxOffset = weedArray.patterns[i] * WEED_PATTERN_SIZE;
    int x = weedArray.offset + i * WEED_PATTERN_SIZE;
    if (x < -WEED_PATTERN_SIZE) {
      x += WEED_PATTERN_SIZE * WEED_ARRAY_LENGTH;
    }
    gfx->drawImage(bmpBack, x, groundY - WEED_PATTERN_SIZE, WEED_PATTERN_SIZE,
                   WEED_PATTERN_SIZE, sx + sxOffset, sy);
  }
}

static void renderSpriteFrame(Graphics2D &gfx, const Sprite &bmp,
                              const SpriteFrame &f, int x, int y) {
  gfx->drawImage(bmp, x + f.offsetX, y + f.offsetY, f.width, f.height, f.x,
                 f.y);
}

static void renderStatus(Graphics2D &gfx) {
  for (int i = 0; i < mikuLife; i++) {
    gfx->drawImage(bmpChara, 4 + i * 16, 12, 16, 16, 0, 48);
  }
  if (mikuState == MikuState::GAME_OVER) {
    int w = 128;
    int h = 16;
    gfx->drawImage(bmpBack, (display::WIDTH - w) / 2, (display::HEIGHT - h) / 2,
                   w, h, 0, 64);
  }
}

static void waitVsync() {
  uint64_t nowUs = getTimeUs();
  if (nowUs < nextVsyncTimeUs) {
    sleepUs(nextVsyncTimeUs - nowUs);
  }
  nextVsyncTimeUs += 1000000 / 60;
  if (nextVsyncTimeUs < nowUs) {
    nextVsyncTimeUs = nowUs + 1000000 / 60;
  }
}
