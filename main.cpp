#include "headers/chroma.hpp"
#include <cstddef>
#include <string>
using namespace std;

int main() {
  chroma chromaObj;
  string filePath = "./raw_images/aimer.raw";
  chromaObj.generateSubsample(1594, 736, filePath);
}
