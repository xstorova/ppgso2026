// Task 3 - Implement Bresenham drawing alg.
//        - Draw a star using lines
//        - Make use of std::vector to define the shape
#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>

#include <ppgso/ppgso.h>

// Size of the framebuffer
const unsigned int SIZE = 512;

struct Point {
  int x,y;
};

float lerp(float v0, float v1, float t) {
  return v0 * (1 - t) + v1 * t;
}

// Bresenham drawing algorithm
void drawLine(ppgso::Image& framebuffer, Point& from, Point& to)
{
  int x1 = from.x;
  int y1 = from.y;
  int x2 = to.x;
  int y2 = to.y;

  int dx = std::abs(x2 - x1);
  int dy = std::abs(y2 - y1);
  int sx = (x1 < x2) ? 1 : -1;
  int sy = (y1 < y2) ? 1 : -1;
  int err = dx - dy;

  while (true) {
    if (x1 >= 0 && x1 < SIZE && y1 >= 0 && y1 < SIZE)
      framebuffer.setPixel(x1, SIZE - 1 - y1, {255, 255, 255});

    if (x1 == x2 && y1 == y2) break;

    int e2 = 2 * err;
    if (e2 > -dy)
    {
      err -= dy;
      x1 += sx;
    }
    if (e2 < dx)
    {
      err += dx;
      y1 += sy;
    }
  }
}

void fillTriangle(ppgso::Image& framebuffer, Point& a, Point& b, Point& c)
{
  float step = 0.01f;

  for (float t1 = 0; t1 <= 1.0f; t1 += step) {
    Point xa, xb;

    xa.x = lerp(a.x, c.x, t1);
    xa.y = lerp(a.y, c.y, t1);

    xb.x = lerp(b.x, c.x, t1);
    xb.y = lerp(b.y, c.y, t1);

    for (float t2 = 0; t2 <= 1.0f; t2 += step) {
      Point xc;
      xc.x = lerp(xa.x, xb.x, t2);
      xc.y = lerp(xa.y, xb.y, t2);

      int px = static_cast<int>(round(xc.x));
      int py = static_cast<int>(round(xc.y));

      if (px >= 0 && px < SIZE && py >= 0 && py < SIZE) {
        framebuffer.setPixel(px, SIZE - 1 - py, {0, 0, 255});
      }
    }
  }
}

int main()
{
  // Use ppgso::Image as our framebuffer
  ppgso::Image framebuffer(SIZE, SIZE);

  // TODO: Generate star points
  std::vector<Point> points;

  points.push_back({50, 50});
  points.push_back({50, 200});
  points.push_back({450, 50});
  points.push_back({450, 200});

  points.push_back({250, 350});
  points.push_back({50, 200});
  points.push_back({450, 200});
  points.push_back({50, 50});
  points.push_back({450, 50});


  // Draw lines
  for(unsigned int i = 0; i < points.size() - 1; i++)
    drawLine(framebuffer, points[i], points[i+1]);

  fillTriangle(framebuffer, points[1], points[3], points[4]);
  // Save the result
  std::cout << "Generating task2_bresenham.bmp file ..." << std::endl;
  ppgso::image::saveBMP(framebuffer, "task2_bresenham.bmp");

  std::cout << "Done." << std::endl;
  return EXIT_SUCCESS;
}
