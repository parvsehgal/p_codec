#include "headers/coder.hpp"
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

// --- actual work, no I/O prompts ---
int doEncode(unsigned width, unsigned height, const string &filePath,
             const string &outputFile) {
  coder coderObj;
  vector<unsigned char> compressed = coderObj.encode(height, width, filePath);
  ofstream out(outputFile, std::ios::binary);
  out.write(reinterpret_cast<char *>(compressed.data()), compressed.size());
  return out ? 0 : 1;
}

int doDecode(const string &compressedFile) {
  coder coderObj;
  coderObj.decode(compressedFile);
  return 0;
}

void printUsage(const char *prog) {
  cerr << "Usage:\n"
       << "  " << prog
       << " encode -w <width> -h <height> -i <input> -o <output>\n"
       << "  " << prog << " decode -i <compressed>\n";
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printUsage(argv[0]);
    return 1;
  }

  string mode = argv[1];
  unsigned width = 0, height = 0;
  string input, output;

  // simple flag parser
  for (int i = 2; i < argc; ++i) {
    string arg = argv[i];
    auto next = [&](const char *flag) -> string {
      if (++i >= argc) {
        cerr << "Missing value for " << flag << "\n";
        exit(1);
      }
      return argv[i];
    };
    if (arg == "-w" || arg == "--width")
      width = stoi(next("-w"));
    else if (arg == "-h" || arg == "--height")
      height = stoi(next("-h"));
    else if (arg == "-i" || arg == "--input")
      input = next("-i");
    else if (arg == "-o" || arg == "--output")
      output = next("-o");
    else {
      cerr << "Unknown option: " << arg << "\n";
      printUsage(argv[0]);
      return 1;
    }
  }

  if (mode == "encode") {
    if (!width || !height || input.empty() || output.empty()) {
      printUsage(argv[0]);
      return 1;
    }
    return doEncode(width, height, input, output);
  } else if (mode == "decode") {
    if (input.empty()) {
      printUsage(argv[0]);
      return 1;
    }
    return doDecode(input);
  }

  printUsage(argv[0]);
  return 1;
}
