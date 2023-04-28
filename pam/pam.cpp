//
// Created by giba on 17/04/23.
//

#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <array>

#include "matrix.h"


using namespace std;
using rgb = array<uint8_t, 3>;


// GRAYSCALE pam save
bool save_pam(mat<uint8_t> &mat_, const string &filename)
{
        ofstream os(filename, ios::binary);
        if (!os) {
                return false;
        }

        os << "P7\n";
        os << "WIDTH " << mat_.columns() << "\n";
        os << "HEIGHT " << mat_.rows() << "\n";
        os << "DEPTH 1" << "\n";
        os << "MAXVAL 255" << "\n";
        os << "TUPLTYPE GRAYSCALE" << "\n";
        os << "ENDHDR" << endl;

        /*for (int r = 0; r < mat_.rows(); ++r) {
                for (int c = 0; c < mat_.columns(); ++c) {
                        os.put(mat_(r, c));
                }
        }*/

        os.write(mat_.rawdata(), mat_.rawsize());

        return true;
}

// GRAYSCALE load pam
bool load_pam(const string &filename, mat<uint8_t> &mat_)
{
        ifstream is(filename, ios::binary);
        if (!is) {
                return false;
        }

        string magic_number;
        getline(is, magic_number);
        if (magic_number != "P7") {
                return false;
        }

        int w, h;
        while (1) {
                string line;
                getline(is, line);
                if (line == "ENDHDR") {
                        break;
                }

                stringstream ss(line);
                string token;
                ss >> token;

                if (token == "WIDTH") {
                        ss >> w;
                } else if (token == "HEIGHT") {
                        ss >> h;
                } else if (token == "DEPTH") {
                        int depth;
                        ss >> depth;
                        if (depth != 1) {
                                return false;
                        }
                } else if (token == "MAXVAL") {
                        int maxval;
                        ss >> maxval;
                        if (maxval != 255) {
                                return false;
                        }
                } else if (token == "TUPLTYPE") {
                        string tupltype;
                        ss >> tupltype;
                        if (tupltype != "GRAYSCALE") {
                                return false;
                        }
                } else {
                        return false;
                }
        }

        mat_.resize(h, w);

        for (int r = 0; r < mat_.rows(); ++r) {
                for (int c = 0; c < mat_.columns(); ++c) {
                        mat_(r, c) = is.get();
                }
        }

        return true;
}

// RGB pam save
bool save_pam(mat<rgb> &mat_, const string &filename)
{
        ofstream os(filename, ios::binary);
        if (!os) {
                return false;
        }

        os << "P7\n";
        os << "WIDTH " << mat_.columns() << "\n";
        os << "HEIGHT " << mat_.rows() << "\n";
        os << "DEPTH 3" << "\n";
        os << "MAXVAL 255" << "\n";
        os << "TUPLTYPE RGB" << "\n";
        os << "ENDHDR" << endl;

        for (int r = 0; r < mat_.rows(); ++r) {
                for (int c = 0; c < mat_.columns(); ++c) {
                        os.put(mat_(r, c)[0]);
                        os.put(mat_(r, c)[1]);
                        os.put(mat_(r, c)[2]);
                }
        }

        // os.write(mat_.rawdata(), mat_.rawsize());

        return true;
}

// RGB pam load
bool load_pam(const string &filename, mat<rgb> &mat_)
{
        ifstream is(filename, ios::binary);
        if (!is) {
                return false;
        }

        string magic_number;
        getline(is, magic_number);
        if (magic_number != "P7") {
                return false;
        }

        int w, h;
        while (1) {
                string line;
                getline(is, line);
                if (line == "ENDHDR") {
                        break;
                }

                stringstream ss(line);
                string token;
                ss >> token;

                if (token == "WIDTH") {
                        ss >> w;
                } else if (token == "HEIGHT") {
                        ss >> h;
                } else if (token == "DEPTH") {
                        int depth;
                        ss >> depth;
                        if (depth != 3) {
                                return false;
                        }
                } else if (token == "MAXVAL") {
                        int maxval;
                        ss >> maxval;
                        if (maxval != 255) {
                                return false;
                        }
                } else if (token == "TUPLTYPE") {
                        string tupltype;
                        ss >> tupltype;
                        if (tupltype != "RGB") {
                                return false;
                        }
                } else {
                        return false;
                }
        }

        mat_.resize(h, w);

        for (int r = 0; r < mat_.rows(); ++r) {
                for (int c = 0; c < mat_.columns(); ++c) {
                        mat_(r, c)[0] = is.get();
                        mat_(r, c)[1] = is.get();
                        mat_(r, c)[2] = is.get();
                }
        }

        return true;
}

template<typename T>
// In place flip
void flip_image(mat<T> &to_flip)
{
        auto rows = to_flip.rows();

        for (int r = 0; r < rows / 2; ++r) {
                for (int c = 0; c < to_flip.columns(); ++c) {
                        // uses array swap in case of rgb
                        swap(to_flip(r, c), to_flip(rows - r - 1, c));
                }
        }
}

template<typename T>
// In place mirror
void mirror_image(mat<T> &to_mirror)
{
        auto cols = to_mirror.columns();

        for (int r = 0; r < to_mirror.rows(); ++r) {
                for (int c = 0; c < cols / 2; ++c) {
                        // uses array swap in case of rgb
                        swap(to_mirror(r, c), to_mirror(r, cols - c - 1));
                }
        }
}



int main(int argc, char **argv)
{
        if (argc != 2) {
                cout << "<pam> outfile" << endl;
                return 1;
        }


        ofstream os(argv[1]);
        if (!os) {
                cout << "Error opening output file" << argv[1] << endl;
                return 1;
        }

        cout << "Creating 256x256 gray image...";
        mat<uint8_t> img1(256, 256);

        for (int r = 0; r < img1.rows(); r++) {
                for (int c = 0; c < img1.columns(); c++) {
                        img1(r, c) = r;
                }
        }
        cout << " done.\n";
        cout << "Saving 256x256 gray image...";
        save_pam(img1, argv[1]);
        cout << " done.\n";


        cout << "Loading frog.pam...";
        mat<uint8_t> img2;
        if (!load_pam("frog.pam", img2)) {
                return 1;
        }
        cout << " done.\n";

        cout << "Flipping...";
        flip_image(img2);
        cout << " done.\n";
        cout << "Saving frog.flipped.pam...";
        save_pam(img2, "frog.flipped.pam");
        cout << " done.\n";


        cout << "Loading laptop.pam...";
        mat<rgb> img3;
        if (!load_pam("laptop.pam", img3)) {
                return 1;
        }
        cout << " done.\n";

        cout << "Mirroring...";
        mirror_image(img3);
        cout << " done.\n";
        cout << "Saving laptop.mirrored.pam...";
        save_pam(img3, "laptop.mirrored.pam");
        cout << " done.\n";
}