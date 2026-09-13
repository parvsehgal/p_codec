#include <vector>
class bitReader {
public:
  std::vector<unsigned char> buffer;
  int bytePos;
  int currBit;
  void reader(std::vector<unsigned char> data);
};
