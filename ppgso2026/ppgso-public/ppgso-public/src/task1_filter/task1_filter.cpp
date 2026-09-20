// Task 1 - Load a 512x512 image lena.raw
//        - Apply specified per-pixel transformation to each pixel
//        - Save as result.raw
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>

// Size of the framebuffer
const unsigned int SIZE = 512;

// A simple RGB struct will represent a pixel in the framebuffer
struct Pixel {
  // TODO: Define correct data type for r, g, b channel
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

// Funkcia na clamping hodnôt do rozsahu <0,255>
unsigned char clamp(int value) {
  if (value < 0) return 0;
  if (value > 255) return 255;
  return static_cast<unsigned char>(value);
}

int main()
{
  // Initialize a framebuffer
  auto framebuffer = new Pixel[SIZE][SIZE];

  // TODO: Open file lena.raw (this is 512x512 RAW GRB format)
  std::ifstream file("../data/lena.raw", std::ios::binary);
  if (!file) {
    std::cerr << "Nepodarilo sa otvorit lena.raw\n";
    return 1;
  }

  // TODO: Read data to framebuffer and close the file
  file.read(reinterpret_cast<char*>(framebuffer), SIZE * SIZE * sizeof(Pixel));
  file.close();

  srand(static_cast<unsigned int>(time(0)));
  // Traverse the framebuffer
  for (unsigned int y = 0; y < SIZE; y++) {
    for (unsigned int x = 0; x < SIZE; x++) {
      // TODO: Apply pixel operation
      if (y < SIZE / 2) {
        unsigned char gray =
           static_cast<unsigned char>(
               0.3  * framebuffer[y][x].r +
               0.59 * framebuffer[y][x].g +
               0.11 * framebuffer[y][x].b);

        framebuffer[y][x].r = gray;
        framebuffer[y][x].g = gray;
        framebuffer[y][x].b = gray;
      }
      else
      {
        int noise_r = rand() % 101 - 50;
        int noise_g = rand() % 101 - 50;
        int noise_b = rand() % 101 - 50;

        framebuffer[y][x].r = clamp(framebuffer[y][x].r + noise_r);
        framebuffer[y][x].g = clamp(framebuffer[y][x].g + noise_g);
        framebuffer[y][x].b = clamp(framebuffer[y][x].b + noise_b);
      }
    }
  }

  // TODO: Open file result.raw
  std::cout << "Generating result.raw file ..." << std::endl;
  std::ofstream outfile("result.raw", std::ios::binary | std::ios::trunc);

  // TODO: Write the framebuffer to the file and close it
  outfile.write(reinterpret_cast<char*>(framebuffer), SIZE * SIZE * sizeof(Pixel));
  outfile.close();

  std::cout << "Done." << std::endl;
  delete[] framebuffer;
  return EXIT_SUCCESS;
}
