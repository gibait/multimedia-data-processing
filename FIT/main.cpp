#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

void FitCRC_Get16(uint16_t& crc, uint8_t byte) {
        static const uint16_t crc_table[16] =
                {
                        0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
                        0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400};
        uint16_t tmp;
        // compute checksum of lower four bits of byte
        tmp = crc_table[crc & 0xF];
        crc = (crc >> 4) & 0x0FFF;
        crc = crc ^ tmp ^ crc_table[byte & 0xF];
        // now compute checksum of upper four bits of byte
        tmp = crc_table[crc & 0xF];
        crc = (crc >> 4) & 0x0FFF;
        crc = crc ^ tmp ^ crc_table[(byte >> 4) & 0xF];
}

struct FIT_header_t {
        uint8_t hsize;
        uint8_t protocol_version;
        uint16_t profile_version;
        uint32_t data_size;
        uint32_t data_type;
        uint16_t crc;
} header;

struct FIT_field_t {
        uint32_t number{};
        uint8_t size{};
        uint8_t base_type{};
};

struct FIT_definition_record_t {

        uint8_t reserved{};
        uint8_t architecture{};
        uint16_t global_message_number{};
        uint8_t num_fields{};
        vector<FIT_field_t> fields{};
};


template<typename T>
void read_from_file(std::ifstream& is, T& read, int size, uint16_t& crc) {
        is.read(reinterpret_cast<char*>(&read), size);
        header.data_size -= size;

        for (int i = 0; i < size; ++i) {
                FitCRC_Get16(crc, reinterpret_cast<uint8_t*>(&read)[i]);
        }
}

bool parse_header(std::ifstream& is) {
        try {
                is.read(reinterpret_cast<char*>(&header.hsize), 1);
                is.read(reinterpret_cast<char*>(&header.protocol_version), 1);
                is.read(reinterpret_cast<char*>(&header.profile_version), 2);
                is.read(reinterpret_cast<char*>(&header.data_size), 4);
                is.read(reinterpret_cast<char*>(&header.data_type), 4);
                is.read(reinterpret_cast<char*>(&header.crc), 2);
        } catch (std::exception& e) {
                cout << "Error reading FIT header: " << e.what() << endl;
                return false;
        }
        return true;
}

void parse_definition_record(std::ifstream& is, uint8_t local, std::map<uint8_t, FIT_definition_record_t>& definitions, std::map<uint8_t, uint16_t>& associations, uint16_t& crc) {
        FIT_definition_record_t record;

        read_from_file(is, record.reserved, 1, crc);
        read_from_file(is, record.architecture, 1, crc);
        read_from_file(is, record.global_message_number, 2, crc);
        read_from_file(is, record.num_fields, 1, crc);

        FIT_field_t field;

        for (int i = 0; i < record.num_fields; ++i) {
                read_from_file(is, field.number, 1, crc);
                read_from_file(is, field.size, 1, crc);
                read_from_file(is, field.base_type, 1, crc);

                record.fields.push_back(field);
        }

        // Save record
        definitions[local & 0x0f] = record;

        // Save local to global association
        associations[local & 0x0f] = record.global_message_number;
}

void parse_data_record(std::ifstream& is, uint8_t local, map<uint8_t, FIT_definition_record_t> definitions, map<uint8_t, uint16_t>& associations, uint16_t& crc) {
        uint8_t size;
        uint32_t value;

        bool time_field = false;
        bool avg_speed_field = false;


        if (associations.at(local & 0x0f) == 0) {
                time_field = true;
        }

        if (associations.at(local & 0x0f) == 19) {
                avg_speed_field = true;
        }

        if (definitions.find(local & 0x0f) == definitions.end()) {
                cout << "No definition for local " << (local & 0x0f) << endl;
                return;
        }

        for (auto& field: definitions.at(local & 0x0f).fields) {
                size = field.size;
                read_from_file(is, value, size, crc);

                if (time_field && field.number == 4) {
                        cout << "time_created = " << value << "\n";
                }

                if (avg_speed_field && field.number == 13) {
                        // cout << "avg_speed = " << format("{:.3f}\n", float(value) * 60 * 60 / (1000 * 1000));
                        cout.precision(4);
                        cout << "avg_speed = " << setw(3) << float(value) * 60 * 60 / (1000 * 1000) << "\n";
                }
        }
}

int fitdump(std::ifstream& is) {
        map<uint8_t, FIT_definition_record_t> definitions;
        map<uint8_t, uint16_t> message_association;

        if (!parse_header(is)) {
                return 1;
        }

        uint16_t crc = 0;
        for (size_t i = 0; i < 12; ++i) {
                FitCRC_Get16(crc, reinterpret_cast<uint8_t*>(&header)[i]);
        }

        if (header.crc != crc) {
                cout << "FIT header CRC mismatch" << endl;
                return 1;
        } else {
                cout << "Header CRC ok" << endl;
        }

        crc = 0;
        // First record
        uint8_t record_header;
        read_from_file(is, record_header, 1, crc);

        if ((record_header >> 4) != 4) {
                return 1;
        }
        uint8_t local = record_header & 0x0f;
        if (local != 0) {
                return 1;
        }
        parse_definition_record(is, record_header, definitions, message_association, crc);

        do {
                read_from_file(is, record_header, 1, crc);
                local = record_header & 0x0f;

                if ((record_header >> 4) == 4) {
                        parse_definition_record(is, local, definitions, message_association, crc);
                } else if ((record_header >> 4) == 0) {
                        parse_data_record(is, local, definitions, message_association, crc);
                }

        } while (header.data_size > 2);

        uint16_t crc_file;
        is.read(reinterpret_cast<char*>(&crc_file), 2);

        if (crc_file != crc) {
                cout << "FIT file CRC mismatch" << endl;
                return 1;
        } else {
                cout << "File CRC ok" << endl;
        }

        return 0;
}

int main(int argc, char** argv) {
        if (argc != 2) {
                cout << "Usage: " << argv[0] << " <input file>" << endl;
                return 1;
        }

        ifstream is(argv[1], ios::binary);
        if (!is) {
                cout << "Cannot open input file " << argv[1] << endl;
                return 1;
        }

        return fitdump(is);
}