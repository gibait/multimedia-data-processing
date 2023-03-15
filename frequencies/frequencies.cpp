// Libreria del C inclusa in C++
#include <cstdint>

#include <fstream>
#include <iostream>
#include <ostream>
#include <iomanip>
#include <unordered_map>

void usage()
{
	std::cout << "Usage: frequencies"
		  << " <input_file> <output_file>" << std::endl;
	exit(EXIT_FAILURE);
}


template <typename T, typename CountT = uint32_t>
class freq {
        std::unordered_map<T, CountT> count_;
public:
        void operator() (const T &val) {
                ++count_[val];
        }

        auto begin() { return count_.begin(); }
        auto end() { return count_.end(); }

        auto begin() const { return count_.cbegin(); }
        auto end() const { return count_.cend(); }

        CountT operator[] (const T &val) {
                return count_[val];
        }
};


int main(int argc, char **argv)
{
	using namespace std;

	if (argc != 3) {
		usage();
	}

	ifstream is(argv[1], ios::binary);
	if (!is) {
		cout << "Error opening input file" << endl;
		usage();
	}

	ofstream os(argv[2]);
	if (!os) {
		cout << "Error opening output file" << endl;
		usage();
	}

	freq<int> count;
	while (1) {
		char num;
		num = is.get();
		if (num == EOF) {
			break;
		}
		count(num);
	}

	for (int i = 0; i < 256; i++) {
		if (count[i] > 0) {
                        os << setbase(16) << setw(2) << setfill('0') << uppercase << i;
                        os << '\t' << setbase(10) << count[i] << '\n';
		}
	}

        return 0;
}