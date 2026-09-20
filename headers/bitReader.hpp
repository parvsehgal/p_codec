#include <vector>
class bitReader {
  std::vector<unsigned char> buffer;
  int currentByte; // the current byte in the buffer
  int currentPos;  // the current Position in the currentByte can be from 0 to 7
  int bufferSize;

public:
  void init(std::vector<unsigned char> &encodedFile);
  int getBit();
  bool canRead();
};
