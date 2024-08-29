#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <random>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

void hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b) {
  float p, q, t, ff;
  int i;
  float f;

  if (s <= 0.0) {
    *r = *g = *b = (int)(v * 255.0);
    return;
  }

  h /= 60.0;
  i = (int)floor(h);
  f = h - i; // Factorial part of h
  p = v * (1.0 - s);
  q = v * (1.0 - (s * f));
  t = v * (1.0 - (s * (1.0 - f)));

  switch (i) {
  case 0:
    *r = (int)(v * 255.0);
    *g = (int)(t * 255.0);
    *b = (int)(p * 255.0);
    break;
  case 1:
    *r = (int)(q * 255.0);
    *g = (int)(v * 255.0);
    *b = (int)(p * 255.0);
    break;
  case 2:
    *r = (int)(p * 255.0);
    *g = (int)(v * 255.0);
    *b = (int)(t * 255.0);
    break;
  case 3:
    *r = (int)(p * 255.0);
    *g = (int)(q * 255.0);
    *b = (int)(v * 255.0);
    break;
  case 4:
    *r = (int)(t * 255.0);
    *g = (int)(p * 255.0);
    *b = (int)(v * 255.0);
    break;
  case 5:
  default:
    *r = (int)(v * 255.0);
    *g = (int)(p * 255.0);
    *b = (int)(q * 255.0);
    break;
  }
}

int in_square(int w, int sw, int x, int y) {
  return x > sw && x < w - sw && y > sw && y < w - sw;
}

#define CHAN 3

char *generate_image(int w, const char *message) {
  static std::random_device random_device;
  static std::mt19937_64 engine(random_device());
  static std::uniform_int_distribution<uint8_t> uniform_distribution;
  srand(time(NULL));
  size_t len = strlen(message);
#define INNEREST 63
  char *ptr = new char[w * w * CHAN];
  for (int y = 0; y < w; y++)
    for (int x = 0; x < w; x++) {
      if (in_square(w, INNEREST, x, y)) {
        int ww = (w - INNEREST * 2 - 1);
        int i = ((x - INNEREST) - 1);
        int j = ((y - INNEREST) - 1);
        int c = j * ww + i;
        if (c < len) {
          ptr[(x + y * w) * CHAN + 0] = message[c];
          ptr[(x + y * w) * CHAN + 1] = message[c];
          ptr[(x + y * w) * CHAN + 2] = message[c];
        } else {
          int v = uniform_distribution(engine);
          ptr[(x + y * w) * CHAN + 0] = (v / 10) + 90;
          ptr[(x + y * w) * CHAN + 2] = (v / 10) + 90;
          ptr[(x + y * w) * CHAN + 1] = (v / 10) + 90;
        }
      } else if (in_square(w, INNEREST - 1, x, y)) {
        int r, g, b;
        hsv_to_rgb(int((y + x - 140)), 1, 1, &r, &g, &b);
        ptr[(x + y * w) * CHAN + 0] = r;
        ptr[(x + y * w) * CHAN + 1] = g;
        ptr[(x + y * w) * CHAN + 2] = b;
      } else if (in_square(w, INNEREST - 10, x, y)) {
        ptr[(x + y * w) * CHAN + 0] = 0;
        ptr[(x + y * w) * CHAN + 1] = 0;
        ptr[(x + y * w) * CHAN + 2] = 0;
      } else if (in_square(w, (INNEREST - 10) / 2, x, y)) {
        ptr[(x + y * w) * CHAN + 0] =
            uniform_distribution(engine) / int(y / (w / 256.0)) * 255;
        ptr[(x + y * w) * CHAN + 1] =
            uniform_distribution(engine) / int(y / (w / 256.0)) * 255;
        ptr[(x + y * w) * CHAN + 2] =
            uniform_distribution(engine) / int(y / (w / 256.0)) * 255;
      } else {
        ptr[(x + y * w) * CHAN + 0] = uniform_distribution(engine);
        ptr[(x + y * w) * CHAN + 1] = uniform_distribution(engine);
        ptr[(x + y * w) * CHAN + 2] = uniform_distribution(engine);
      }
    }
  return ptr;
}

int main() {
  std::filesystem::current_path(WORKING_DIRECTORY);

  FILE *file = fopen("text.txt", "r");
  assert(file);
  fseek(file, 0, SEEK_END);
  size_t fileSize = ftell(file);
  assert(fileSize != -1L);
  fseek(file, 0, SEEK_SET);
  std::string text(fileSize, '\0');
  size_t bytesRead = fread(text.data(), 1, fileSize, file);
  assert(bytesRead == fileSize);
  fclose(file);

  int l = 300;
  int res = stbi_write_png("output.png", l, l, CHAN,
                           generate_image(l, text.c_str()), 0);
  if (!res)
    return 1;
  printf("stbi_write_png returned %d\n", res);
  return 0;
}