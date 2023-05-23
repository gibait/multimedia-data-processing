//
// Created by giba on 07/05/23.
//

#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

namespace bencode {

        struct elem {
                virtual void print(size_t tab = 0) const = 0;
        };

        elem *elem_factory(std::istream &is);

        void print_pieces(const elem* e, size_t tab);

        class string : public elem {
        public:

                bool operator< (const string &other) const
                {
                        return s_ < other.s_;
                }

                std::string s_;
                string(std::istream &is)
                {
                        auto error = std::runtime_error("Error while parsing string");
                        char colon;
                        int len;
                        is >> len >> colon;
                        if (!is || colon != ':') {
                                throw error;
                        }
                        s_.resize(len);
                        is.read(&s_[0], len);
                        if (!is) {
                                throw error;
                        }
                }

                void print(size_t tab = 0) const override
                {
                        std::cout <<  "\"";
                        for (const auto &i : s_) {
                                std::cout << i;
                        }
                        std::cout << "\"";
                }


        };

        class integer : public elem {
                int64_t i_;

                public:
                        integer(std::istream &is)
                        {
                                char start, end;
                                is >> start >> i_ >> end;
                                if (start != 'i' || end != 'e') {
                                        throw std::runtime_error("Error while parsing integer");
                                }
                        }

                        void print(size_t tab = 0) const override
                        {
                                std::cout << i_;
                        }

        };

        class list : public elem {
                        std::vector<bencode::elem*> list_;

                public:
                        list(std::istream &is)
                        {
                                char start;
                                is >> start;
                                if (!is || start != 'l') {
                                        throw std::runtime_error("Error while parsing list");
                                }
                                while (is.peek() != 'e') {
                                        list_.push_back(elem_factory(is));
                                }
                                is.get(); // consume 'e'
                        }

                        void print(size_t tab = 1) const override
                        {
                                std::cout << "[\n";
                                for (const auto &x: list_) {
                                        std::cout << std::string(tab + 1, '\t');
                                        x->print(tab + 1);
                                        std::cout << "\n";
                                }
                                std::cout << std::string(tab, '\t') << "]";
                        }
        };

        class dict : public elem {
                        std::map<string, elem*> dict_;

                public:

                        dict(std::istream &is)
                        {
                                char start;
                                is >> start;
                                if (!is || start != 'd') {
                                        throw std::runtime_error("Error while parsing list");
                                }
                                while (is.peek() != 'e') {
                                        string s(is);
                                        dict_[s] = elem_factory(is);
                                }
                                is.get();
                        }

                        void print(size_t tab = 0) const override
                        {
                                std::cout << "{\n";
                                for (auto const &x: dict_) {
                                        std::cout << std::string(tab + 1, '\t');
                                        x.first.print(tab + 1);
                                        std::cout << " => ";
                                        if (x.first.s_ == "pieces") {
                                                print_pieces(x.second, tab);
                                        } else {
                                                x.second->print(tab + 1);
                                        }
                                        std::cout << "\n";
                                }
                                std::cout << std::string(tab, '\t') << "}";
                        }
        };



        elem *elem_factory(std::istream& is) {
                        switch(is.peek()) {
                                case 'i':
                                        return new integer(is);
                                case 'l':
                                        return new list(is);
                                case 'd':
                                        return new dict(is);
                                default:
                                        return new string(is);
                        }
        }


        void print_pieces(const elem* e, size_t tab)
        {
                        auto pieces = dynamic_cast<const string*>(e);
                        if (pieces->s_.size() % 20 != 0) {
                                throw std::runtime_error("Error while parsing pieces");
                        }
                        for (size_t i = 0; i < pieces->s_.size(); i += 20) {
                                std::cout << "\n" << std::string(tab + 1, '\t');
                                for (size_t j = 0; j < 20; ++j) {
                                        std::cout << std::format("{:02x}", pieces->s_[i + j]);
                                }
                        }
        }
}



int main(int argc, char **argv)
{
        if (argc != 2) {
                std::cout << "Numero di parametri errato" << std::endl;
        }

        std::ifstream is(argv[1], std::ios::binary);
        if (!is) {
                std::cout << "File non trovato" << std::endl;
        }

        using namespace bencode;
        try {
                bencode::elem *root = elem_factory(is);
                root->print();
        } catch (std::exception &e) {
                throw e;
        }

        return 0;
}
